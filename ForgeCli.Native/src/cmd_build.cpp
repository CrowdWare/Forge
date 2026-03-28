#include "cmd_build.h"

#include "godot_sdk.h"
#include "sandbox_policy.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

using SmlParseFn = int (*)(const char*, std::int64_t*, char*, int);
using SmsSessionCreateFn = int (*)(std::int64_t*, char*, int);
using SmsSessionLoadFn = int (*)(std::int64_t, const char*, char*, int);
using SmsSessionDisposeFn = int (*)(std::int64_t, char*, int);
using SmsCodegenLlvmIrFn = int (*)(const char*, char*, int, char*, int);
using SmsCodegenLlvmIrModeFn = int (*)(const char*, const char*, char*, int, char*, int);
using SmsSandboxPathAllowFn = int (*)(const char*, const char*, char*, int);
using SmsSetSandboxPathCallbackFn = int (*)(SmsSandboxPathAllowFn, char*, int);

namespace {

forgecli::SandboxRoots g_sandbox_roots;

int sandbox_allow_path(const char* owner, const char* uri_path, char* error, int error_capacity) {
    return forgecli::sandbox_allow_path(g_sandbox_roots, owner, uri_path, error, error_capacity);
}

std::string shell_quote(const std::string& s) {
#if defined(_WIN32)
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else out += c;
    }
    out += "\"";
    return out;
#else
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out += c;
    }
    out += "'";
    return out;
#endif
}

int run_command(const std::string& cmd) {
    return std::system(cmd.c_str());
}

bool command_exists(const std::string& name) {
#if defined(_WIN32)
    const std::string cmd = "where " + shell_quote(name) + " >NUL 2>&1";
#else
    const std::string cmd = "command -v " + shell_quote(name) + " >/dev/null 2>&1";
#endif
    return run_command(cmd) == 0;
}

void* load_symbol(void* lib, const char* name) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(lib), name));
#else
    return dlsym(lib, name);
#endif
}

void* load_lib(const fs::path& file) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(LoadLibraryA(file.string().c_str()));
#else
    return dlopen(file.string().c_str(), RTLD_NOW);
#endif
}

void free_lib(void* lib) {
    if (lib == nullptr) return;
#if defined(_WIN32)
    FreeLibrary(reinterpret_cast<HMODULE>(lib));
#else
    dlclose(lib);
#endif
}

std::string lib_ext() {
#if defined(_WIN32)
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

bool parse_sml(SmlParseFn fn, const fs::path& p, std::string& err) {
    std::ifstream in(p, std::ios::binary);
    if (!in.is_open()) {
        err = "File not found.";
        return false;
    }
    std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    char e[2048] = {0};
    std::int64_t nodes = 0;
    if (fn(src.c_str(), &nodes, e, static_cast<int>(sizeof(e))) == 0) {
        return true;
    }
    err = e[0] ? std::string(e) : "SML parse failed";
    return false;
}

bool parse_sms(SmsSessionCreateFn create_fn,
               SmsSessionLoadFn load_fn,
               SmsSessionDisposeFn dispose_fn,
               const fs::path& p,
               std::string& err) {
    std::ifstream in(p, std::ios::binary);
    if (!in.is_open()) {
        err = "File not found.";
        return false;
    }
    std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    char e[2048] = {0};
    std::int64_t session = 0;
    if (create_fn(&session, e, static_cast<int>(sizeof(e))) != 0) {
        err = e[0] ? std::string(e) : "sms session create failed";
        return false;
    }
    int rc = load_fn(session, src.c_str(), e, static_cast<int>(sizeof(e)));
    dispose_fn(session, e, static_cast<int>(sizeof(e)));
    if (rc == 0) {
        return true;
    }
    err = e[0] ? std::string(e) : "SMS parse failed";
    return false;
}

bool codegen_sms_llvm_ir(SmsCodegenLlvmIrFn codegen_fn,
                         SmsCodegenLlvmIrModeFn codegen_mode_fn,
                         const std::string& source,
                         const std::string& mode,
                         std::string& out_ir,
                         std::string& err) {
    int capacity = 64 * 1024;
    while (capacity <= 8 * 1024 * 1024) {
        std::vector<char> buffer(static_cast<std::size_t>(capacity), '\0');
        char e[2048] = {0};
        const int rc = codegen_mode_fn != nullptr
            ? codegen_mode_fn(source.c_str(), mode.c_str(), buffer.data(), capacity, e, static_cast<int>(sizeof(e)))
            : codegen_fn(source.c_str(), buffer.data(), capacity, e, static_cast<int>(sizeof(e)));
        if (rc == 0) {
            out_ir = std::string(buffer.data());
            return true;
        }
        const std::string msg = e[0] ? std::string(e) : std::string("llvm-ir generation failed");
        if (msg.find("output buffer too small") != std::string::npos) {
            capacity *= 2;
            continue;
        }
        err = msg;
        return false;
    }
    err = "llvm-ir output exceeds supported buffer size";
    return false;
}

bool is_expected_llvm_codegen_skip(const std::string& error) {
    if (error.find("no top-level function call and no 'main' function fallback") != std::string::npos)
        return true;
    return false;
}

std::string sanitize_filename_token(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (char c : input) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_') {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    if (out.empty()) out = "script";
    return out;
}

bool bundle_sms_llvm_ir_artifacts(const fs::path& project,
                                  const fs::path& staging,
                                  const bool strict_native,
                                  std::string& err) {
    const char* sms_dir = std::getenv("SMS_NATIVE_LIB_DIR");
    if (!sms_dir || sms_dir[0] == '\0') {
        std::cerr << "[WARN] SMS_NATIVE_LIB_DIR not set. Skipping SMS LLVM IR bundling.\n";
        return true;
    }

    const fs::path sms_lib_path = fs::path(sms_dir) / ("libsms_native" + lib_ext());
    void* sms_lib = load_lib(sms_lib_path);
    if (!sms_lib) {
        std::cerr << "[WARN] Failed to load SMS native library for LLVM bundling: "
                  << sms_lib_path.string() << "\n";
        return true;
    }
    auto cleanup = [&]() {
        free_lib(sms_lib);
    };

    auto codegen_fn = reinterpret_cast<SmsCodegenLlvmIrFn>(load_symbol(sms_lib, "sms_native_codegen_llvm_ir"));
    auto codegen_mode_fn = reinterpret_cast<SmsCodegenLlvmIrModeFn>(load_symbol(sms_lib, "sms_native_codegen_llvm_ir_mode"));
    if (!codegen_fn) {
        cleanup();
        std::cerr << "[WARN] Missing symbol sms_native_codegen_llvm_ir. Skipping SMS LLVM IR bundling.\n";
        return true;
    }
    if (strict_native && codegen_mode_fn == nullptr) {
        cleanup();
        err = "Native-only SMS compile requires symbol sms_native_codegen_llvm_ir_mode (compiler mode switch missing).";
        return false;
    }

    std::vector<fs::path> sms_files;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(project, ec)) {
        if (ec) {
            cleanup();
            err = "Failed to enumerate project SMS files: " + project.string();
            return false;
        }
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (ext == ".sms") {
            sms_files.push_back(entry.path());
        }
    }
    std::sort(sms_files.begin(), sms_files.end());
    if (sms_files.empty()) {
        cleanup();
        return true;
    }

    const fs::path out_dir = staging / "sms_llvm";
    fs::create_directories(out_dir, ec);
    if (ec) {
        cleanup();
        err = "Failed to create SMS LLVM staging directory: " + out_dir.string();
        return false;
    }

    int emitted_count = 0;
    int expected_skipped_count = 0;
    int unexpected_failed_count = 0;
    std::vector<std::string> manifest_lines;
    manifest_lines.reserve(sms_files.size());
    for (const auto& sms_file : sms_files) {
        std::ifstream in(sms_file, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "[WARN] Failed to read SMS for LLVM bundling: " << sms_file.string() << "\n";
            unexpected_failed_count++;
            continue;
        }
        const std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        std::string ir_text;
        std::string codegen_error;
        if (!codegen_sms_llvm_ir(codegen_fn, codegen_mode_fn, source, "lib", ir_text, codegen_error)) {
            const bool expected_skip = is_expected_llvm_codegen_skip(codegen_error);
            if (expected_skip) {
                expected_skipped_count++;
                continue;
            }
            if (strict_native) {
                cleanup();
                err = "Native-only SMS compile failed for '" + sms_file.filename().string() + "': " + codegen_error;
                return false;
            }
            std::cerr << "[WARN] LLVM IR codegen skipped for " << sms_file.filename().string()
                      << ": " << codegen_error << "\n";
            unexpected_failed_count++;
            continue;
        }

        const std::string stem = sanitize_filename_token(sms_file.stem().string());
        const fs::path ir_out = out_dir / (stem + ".ll");
        std::ofstream out(ir_out, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            cleanup();
            err = "Failed to write SMS LLVM artifact: " + ir_out.string();
            return false;
        }
        out << ir_text;
        emitted_count++;
        manifest_lines.push_back(sms_file.filename().string() + " -> " + ir_out.filename().string());
    }

    if (!manifest_lines.empty()) {
        std::ostringstream manifest;
        manifest << "# Generated SMS LLVM IR artifacts\n";
        for (const auto& line : manifest_lines) {
            manifest << line << "\n";
        }
        std::ofstream out(out_dir / "manifest.txt", std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            cleanup();
            err = "Failed to write SMS LLVM artifact manifest.";
            return false;
        }
        out << manifest.str();
    }

    if (strict_native && (emitted_count + expected_skipped_count) != static_cast<int>(sms_files.size())) {
        cleanup();
        err = "Native-only SMS compile failed: not all SMS scripts produced LLVM IR.";
        return false;
    }

    cleanup();
    if (emitted_count > 0 || unexpected_failed_count > 0) {
        std::cout << "[INFO] SMS LLVM artifacts bundled: " << emitted_count
                  << " file(s), skipped: " << (expected_skipped_count + unexpected_failed_count) << "\n";
    }
    return true;
}

enum class BuildTarget {
    Mac,
    Android,
};

struct BuildOptions {
    BuildTarget target = BuildTarget::Mac;
    fs::path project;
    fs::path output;
    std::string godot_version = "4.6-stable";
    fs::path host_project_dir;
    fs::path native_lib_dir;
    std::string android_package_id;
};

bool parse_target(const std::string& value, BuildTarget& out) {
    if (value == "mac") {
        out = BuildTarget::Mac;
        return true;
    }
    if (value == "android") {
        out = BuildTarget::Android;
        return true;
    }
    return false;
}

std::string to_lower_ascii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

std::string package_safe_suffix(const std::string& project_name) {
    std::string out;
    for (char c : project_name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }
    if (out.empty()) out = "app";
    return out;
}

std::string env_or_empty(const char* key) {
    const char* v = std::getenv(key);
    if (v == nullptr) return {};
    return std::string(v);
}

std::string ini_escape(std::string value) {
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        if (c == '"') out += "\\\"";
        else out += c;
    }
    return out;
}

std::string usage_line() {
    return "Usage: forgecli-native build <mac|android> --project <dir> [--output <path>] [--godot-version <ver>] [--host-project-dir <dir>] [--native-lib-dir <dir>] [--android-package-id <id>]";
}

bool is_valid_android_package_id(const std::string& id) {
    if (id.empty()) return false;
    if (id.front() == '.' || id.back() == '.') return false;

    bool has_dot = false;
    bool segment_started = false;
    for (std::size_t i = 0; i < id.size(); ++i) {
        const char c = id[i];
        if (c == '.') {
            if (!segment_started) return false;
            segment_started = false;
            has_dot = true;
            continue;
        }
        const bool is_alpha = (c >= 'a' && c <= 'z');
        const bool is_digit = (c >= '0' && c <= '9');
        const bool is_underscore = c == '_';
        if (!segment_started) {
            if (!is_alpha) return false;
            segment_started = true;
            continue;
        }
        if (!is_alpha && !is_digit && !is_underscore) return false;
    }
    if (!segment_started) return false;
    return has_dot;
}

bool parse_build_options(const std::vector<std::string>& args, BuildOptions& out, std::string& err) {
    if (args.empty()) {
        err = usage_line();
        return false;
    }

    if (!parse_target(args[0], out.target)) {
        err = "Unknown build target: " + args[0] + "\n" + usage_line();
        return false;
    }

    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "--project" && i + 1 < args.size()) {
            out.project = args[++i];
            continue;
        }
        if (arg == "--output" && i + 1 < args.size()) {
            out.output = args[++i];
            continue;
        }
        if (arg == "--godot-version" && i + 1 < args.size()) {
            out.godot_version = args[++i];
            continue;
        }
        if (arg == "--host-project-dir" && i + 1 < args.size()) {
            out.host_project_dir = args[++i];
            continue;
        }
        if (arg == "--native-lib-dir" && i + 1 < args.size()) {
            out.native_lib_dir = args[++i];
            continue;
        }
        if (arg == "--android-package-id" && i + 1 < args.size()) {
            out.android_package_id = to_lower_ascii(args[++i]);
            continue;
        }
        err = "Unknown or incomplete option: " + arg;
        return false;
    }

    if (out.project.empty()) {
        err = "Missing required option --project";
        return false;
    }
    out.project = fs::absolute(out.project);
    if (!fs::exists(out.project) || !fs::is_directory(out.project)) {
        err = "Project directory does not exist: " + out.project.string();
        return false;
    }

    const std::string project_name = out.project.filename().string().empty()
        ? std::string("forge-app")
        : out.project.filename().string();
    if (out.output.empty()) {
        out.output = fs::current_path() /
            (project_name + (out.target == BuildTarget::Mac ? ".zip" : ".apk"));
    } else if (out.output.is_relative()) {
        out.output = fs::absolute(out.output);
    }
    out.output = fs::absolute(out.output);

    if (out.host_project_dir.empty()) {
        const char* env = std::getenv("FORGE_HOST_PROJECT_DIR");
        if (env != nullptr && env[0] != '\0') out.host_project_dir = env;
    }
    if (out.native_lib_dir.empty()) {
        const char* env = std::getenv("FORGE_NATIVE_LIB_DIR");
        if (env != nullptr && env[0] != '\0') out.native_lib_dir = env;
    }
    if (out.android_package_id.empty()) {
        const char* env = std::getenv("FORGE_ANDROID_PACKAGE_ID");
        if (env != nullptr && env[0] != '\0') out.android_package_id = to_lower_ascii(env);
    }

    if (out.host_project_dir.empty()) {
        err = "Missing FORGE_HOST_PROJECT_DIR (or --host-project-dir).";
        return false;
    }
    if (out.native_lib_dir.empty()) {
        err = "Missing FORGE_NATIVE_LIB_DIR (or --native-lib-dir).";
        return false;
    }

    out.host_project_dir = fs::absolute(out.host_project_dir);
    out.native_lib_dir = fs::absolute(out.native_lib_dir);
    if (!fs::exists(out.host_project_dir) || !fs::is_directory(out.host_project_dir)) {
        err = "Host project directory not found: " + out.host_project_dir.string();
        return false;
    }
    if (!fs::exists(out.native_lib_dir) || !fs::is_directory(out.native_lib_dir)) {
        err = "Native library directory not found: " + out.native_lib_dir.string();
        return false;
    }
    if (out.target == BuildTarget::Android && !out.android_package_id.empty() &&
        !is_valid_android_package_id(out.android_package_id)) {
        err = "Invalid Android package id: " + out.android_package_id;
        return false;
    }

    return true;
}

bool copy_recursive(const fs::path& from, const fs::path& to, std::string& err) {
    std::error_code ec;
    fs::create_directories(to, ec);
    if (ec) {
        err = "Failed to create directory: " + to.string();
        return false;
    }
    for (const auto& entry : fs::recursive_directory_iterator(from, ec)) {
        if (ec) break;
        const fs::path rel = fs::relative(entry.path(), from, ec);
        if (ec) break;
        const fs::path dst = to / rel;
        if (entry.is_directory()) {
            fs::create_directories(dst, ec);
            if (ec) break;
            continue;
        }
        if (!entry.is_regular_file()) {
            continue;
        }
        fs::create_directories(dst.parent_path(), ec);
        if (ec) break;
        fs::copy_file(entry.path(), dst, fs::copy_options::overwrite_existing, ec);
        if (ec) break;
    }
    if (ec) {
        err = "Failed to copy directory: " + from.string() + " -> " + to.string();
        return false;
    }
    return true;
}

bool patch_project_name(const fs::path& project_godot_path,
                        const std::string& name,
                        BuildTarget target,
                        std::string& err) {
    std::ifstream in(project_godot_path, std::ios::binary);
    if (!in.is_open()) {
        err = "Failed to open project.godot for patching: " + project_godot_path.string();
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    bool patched = false;
    bool has_boot_splash_show_image = false;
    bool has_boot_splash_image = false;
    bool has_boot_splash_min_display = false;
    bool has_etc2_astc = false;
    bool has_rendering_method = false;
    bool has_always_on_top = false;
    bool has_transparent = false;
    bool has_extend_to_title = false;
    bool has_viewport_width = false;
    bool has_viewport_height = false;
    bool has_handheld_orientation = false;
    while (std::getline(in, line)) {
        if (line.rfind("config/name=", 0) == 0) {
            line = "config/name=\"" + name + "\"";
            patched = true;
        }
        if (line.rfind("boot_splash/show_image=", 0) == 0 ||
            line.rfind("application/boot_splash/show_image=", 0) == 0) {
            line = "boot_splash/show_image=false";
            has_boot_splash_show_image = true;
        }
        if (line.rfind("boot_splash/image=", 0) == 0 ||
            line.rfind("application/boot_splash/image=", 0) == 0) {
            line = "boot_splash/image=\"\"";
            has_boot_splash_image = true;
        }
        if (line.rfind("boot_splash/minimum_display_time=", 0) == 0 ||
            line.rfind("application/boot_splash/minimum_display_time=", 0) == 0) {
            line = "boot_splash/minimum_display_time=0";
            has_boot_splash_min_display = true;
        }
        if (line.rfind("textures/vram_compression/import_etc2_astc=", 0) == 0) {
            line = "textures/vram_compression/import_etc2_astc=true";
            has_etc2_astc = true;
        }
        if (line.rfind("renderer/rendering_method=", 0) == 0) {
            if (target == BuildTarget::Android) {
                line = "renderer/rendering_method=\"mobile\"";
            }
            has_rendering_method = true;
        }
        if (line.rfind("window/size/always_on_top=", 0) == 0) {
            if (target == BuildTarget::Android) {
                line = "window/size/always_on_top=false";
            }
            has_always_on_top = true;
        }
        if (line.rfind("window/size/viewport_width=", 0) == 0) {
            if (target == BuildTarget::Android) {
                line = "window/size/viewport_width=360";
            }
            has_viewport_width = true;
        }
        if (line.rfind("window/size/viewport_height=", 0) == 0) {
            if (target == BuildTarget::Android) {
                line = "window/size/viewport_height=640";
            }
            has_viewport_height = true;
        }
        if (line.rfind("window/size/transparent=", 0) == 0) {
            if (target == BuildTarget::Android) {
                line = "window/size/transparent=false";
            }
            has_transparent = true;
        }
        if (line.rfind("window/size/extend_to_title=", 0) == 0) {
            if (target == BuildTarget::Android) {
                line = "window/size/extend_to_title=false";
            }
            has_extend_to_title = true;
        }
        if (line.rfind("window/handheld/orientation=", 0) == 0) {
            if (target == BuildTarget::Android) {
                line = "window/handheld/orientation=6";
            }
            has_handheld_orientation = true;
        }
        lines.push_back(line);
    }
    in.close();

    if (!patched) {
        lines.push_back("config/name=\"" + name + "\"");
    }
    if (!has_boot_splash_show_image) {
        auto application_it = std::find(lines.begin(), lines.end(), "[application]");
        if (application_it != lines.end()) {
            lines.insert(application_it + 1, "boot_splash/show_image=false");
        } else {
            lines.push_back("");
            lines.push_back("[application]");
            lines.push_back("boot_splash/show_image=false");
        }
    }
    if (!has_boot_splash_image) {
        auto application_it = std::find(lines.begin(), lines.end(), "[application]");
        if (application_it != lines.end()) {
            lines.insert(application_it + 1, "boot_splash/image=\"\"");
        } else {
            lines.push_back("");
            lines.push_back("[application]");
            lines.push_back("boot_splash/image=\"\"");
        }
    }
    if (!has_boot_splash_min_display) {
        auto application_it = std::find(lines.begin(), lines.end(), "[application]");
        if (application_it != lines.end()) {
            lines.insert(application_it + 1, "boot_splash/minimum_display_time=0");
        } else {
            lines.push_back("");
            lines.push_back("[application]");
            lines.push_back("boot_splash/minimum_display_time=0");
        }
    }
    if (!has_etc2_astc) {
        auto rendering_it = std::find(lines.begin(), lines.end(), "[rendering]");
        if (rendering_it != lines.end()) {
            lines.insert(rendering_it + 1, "textures/vram_compression/import_etc2_astc=true");
        } else {
            lines.push_back("");
            lines.push_back("[rendering]");
            lines.push_back("textures/vram_compression/import_etc2_astc=true");
        }
    }
    if (target == BuildTarget::Android) {
        auto display_it = std::find(lines.begin(), lines.end(), "[display]");
        if (display_it != lines.end()) {
            auto insert_pos = display_it + 1;
            if (!has_always_on_top) lines.insert(insert_pos++, "window/size/always_on_top=false");
            if (!has_viewport_width) lines.insert(insert_pos++, "window/size/viewport_width=360");
            if (!has_viewport_height) lines.insert(insert_pos++, "window/size/viewport_height=640");
            if (!has_transparent) lines.insert(insert_pos++, "window/size/transparent=false");
            if (!has_extend_to_title) lines.insert(insert_pos++, "window/size/extend_to_title=false");
            if (!has_handheld_orientation) lines.insert(insert_pos++, "window/handheld/orientation=6");
        } else {
            lines.push_back("");
            lines.push_back("[display]");
            lines.push_back("window/size/always_on_top=false");
            lines.push_back("window/size/viewport_width=360");
            lines.push_back("window/size/viewport_height=640");
            lines.push_back("window/size/transparent=false");
            lines.push_back("window/size/extend_to_title=false");
            lines.push_back("window/handheld/orientation=6");
        }

        if (!has_rendering_method) {
            auto rendering_it = std::find(lines.begin(), lines.end(), "[rendering]");
            if (rendering_it != lines.end()) {
                lines.insert(rendering_it + 1, "renderer/rendering_method=\"mobile\"");
            } else {
                lines.push_back("");
                lines.push_back("[rendering]");
                lines.push_back("renderer/rendering_method=\"mobile\"");
            }
        }
    }

    std::ofstream out(project_godot_path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        err = "Failed to write patched project.godot: " + project_godot_path.string();
        return false;
    }
    for (const auto& l : lines) {
        out << l << "\n";
    }
    return true;
}

std::string mac_export_preset(const fs::path& export_path) {
    std::ostringstream os;
    os
        << "[preset.0]\n\n"
        << "name=\"macOS\"\n"
        << "platform=\"macOS\"\n"
        << "runnable=true\n"
        << "advanced_options=false\n"
        << "dedicated_server=false\n"
        << "custom_features=\"\"\n"
        << "export_filter=\"all_resources\"\n"
        << "include_filter=\"*.sml,*.sms,*.ll\"\n"
        << "exclude_filter=\"\"\n"
        << "export_path=\"" << export_path.string() << "\"\n"
        << "encryption_include_filters=\"\"\n"
        << "encryption_exclude_filters=\"\"\n"
        << "encrypt_pck=false\n"
        << "encrypt_directory=false\n\n"
        << "[preset.0.options]\n\n"
        << "binary_format/architecture=\"universal\"\n"
        << "custom_template/debug=\"\"\n"
        << "custom_template/release=\"\"\n"
        << "debug/export_console_wrapper=1\n"
        << "codesign/enable=false\n"
        << "codesign/identity=\"\"\n"
        << "codesign/entitlements/custom_file=\"\"\n"
        << "notarization/enable=false\n"
        << "privacy/camera_usage_description=\"\"\n"
        << "privacy/microphone_usage_description=\"\"\n"
        << "application/bundle_identifier=\"io.crowdware.forgerunner\"\n"
        << "application/short_version=\"1.0\"\n"
        << "application/version=\"1.0\"\n"
        << "application/copyright=\"CrowdWare\"\n";
    return os.str();
}

std::string android_export_preset(const fs::path& export_path,
                                  const std::string& project_name,
                                  const std::string& package_id_override,
                                  bool use_gradle_build,
                                  const std::vector<std::string>& plugin_names) {
    std::string package_name = package_id_override;
    if (package_name.empty()) {
        package_name = env_or_empty("FORGE_ANDROID_PACKAGE_ID");
    }
    if (package_name.empty()) {
        const std::string suffix = package_safe_suffix(project_name);
        package_name = "io.crowdware." + suffix;
    }
    std::string version_code = env_or_empty("FORGE_ANDROID_VERSION_CODE");
    if (version_code.empty()) {
        version_code = "1";
    }
    std::string version_name = env_or_empty("FORGE_ANDROID_VERSION_NAME");
    if (version_name.empty()) {
        version_name = "1.0";
    }
    const std::string release_keystore = env_or_empty("FORGE_ANDROID_RELEASE_KEYSTORE");
    const std::string release_user = env_or_empty("FORGE_ANDROID_RELEASE_USER");
    const std::string release_password = env_or_empty("FORGE_ANDROID_RELEASE_PASSWORD");
    std::ostringstream os;
    os
        << "[preset.0]\n\n"
        << "name=\"Android\"\n"
        << "platform=\"Android\"\n"
        << "runnable=true\n"
        << "advanced_options=false\n"
        << "dedicated_server=false\n"
        << "custom_features=\"\"\n"
        << "export_filter=\"all_resources\"\n"
        << "include_filter=\"*.sml,*.sms,*.ll\"\n"
        << "exclude_filter=\"\"\n"
        << "export_path=\"" << export_path.string() << "\"\n"
        << "encryption_include_filters=\"\"\n"
        << "encryption_exclude_filters=\"\"\n"
        << "encrypt_pck=false\n"
        << "encrypt_directory=false\n\n"
        << "[preset.0.options]\n\n"
        << "custom_template/debug=\"\"\n"
        << "custom_template/release=\"\"\n"
        << "gradle_build/use_gradle_build=" << (use_gradle_build ? "true" : "false") << "\n"
        << "gradle_build/export_format=0\n"
        << "architectures/armeabi-v7a=true\n"
        << "architectures/arm64-v8a=true\n"
        << "architectures/x86_64=false\n"
        << "package/unique_name=\"" << package_name << "\"\n"
        << "package/name=\"" << project_name << "\"\n"
        << "package/signed=true\n"
        << "launcher_icons/main_192x192=\"res://icon.svg\"\n"
        << "launcher_icons/adaptive_foreground_432x432=\"res://icon.svg\"\n"
        << "launcher_icons/adaptive_background_432x432=\"res://icon.svg\"\n"
        << "launcher_icons/adaptive_monochrome_432x432=\"res://icon.svg\"\n"
        << "version/code=" << version_code << "\n"
        << "version/name=\"" << ini_escape(version_name) << "\"\n"
        << "user_data_folder=\"" << package_name << "\"\n"
        << "permissions/internet=true\n"
        << "permissions/record_audio=true\n"
        << "plugins/ForgeSpeechBridge=true\n"
        << "plugins/ForgeSpeechBridge.gdap=true\n";
    for (const auto& plugin_name : plugin_names) {
        os << "plugins/" << ini_escape(plugin_name) << "=true\n";
    }
    if (!release_keystore.empty()) {
        os << "keystore/release=\"" << ini_escape(release_keystore) << "\"\n";
    }
    if (!release_user.empty()) {
        os << "keystore/release_user=\"" << ini_escape(release_user) << "\"\n";
    }
    if (!release_password.empty()) {
        os << "keystore/release_password=\"" << ini_escape(release_password) << "\"\n";
    }
    return os.str();
}

std::vector<std::string> detect_android_plugins(const fs::path& android_dir) {
    std::vector<std::string> out;
    const fs::path plugins_dir = android_dir / "plugins";
    if (!fs::exists(plugins_dir) || !fs::is_directory(plugins_dir)) return out;

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(plugins_dir, ec)) {
        if (ec) break;
        if (!entry.is_directory()) continue;

        const fs::path plugin_dir = entry.path();
        bool has_plugin_artifact = false;
        for (const auto& nested : fs::directory_iterator(plugin_dir, ec)) {
            if (ec) break;
            if (!nested.is_regular_file()) continue;
            const std::string ext = to_lower_ascii(nested.path().extension().string());
            if (ext == ".gdap" || ext == ".aar") {
                has_plugin_artifact = true;
                break;
            }
        }
        if (!ec && has_plugin_artifact) {
            out.push_back(plugin_dir.filename().string());
        }
    }

    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}
bool write_text_file(const fs::path& path, const std::string& text, std::string& err) {
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        err = "Failed to open file for writing: " + path.string();
        return false;
    }
    out << text;
    return true;
}

std::string read_text_file_best_effort(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return {};
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::string filter_noisy_godot_log_lines(const std::string& log_text) {
    std::istringstream in(log_text);
    std::ostringstream out;
    std::string line;
    while (std::getline(in, line)) {
        if (line.find("Orphan StringName:") != std::string::npos) continue;
        if (line.find("unclaimed string names at exit") != std::string::npos) continue;
        out << line << "\n";
    }
    return out.str();
}

bool find_native_library(const fs::path& native_lib_dir, fs::path& out_path) {
    const std::vector<std::string> names = {
        "libforge_runner_native.dylib",
        "libforge_runner_native.so",
        "forge_runner_native.dll",
        "libforge_runner_native.dll"
    };
    for (const auto& name : names) {
        const fs::path candidate = native_lib_dir / name;
        if (fs::exists(candidate)) {
            out_path = candidate;
            return true;
        }
    }
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(native_lib_dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        const std::string filename = entry.path().filename().string();
        if (filename.find("forge_runner_native") != std::string::npos) {
            out_path = entry.path();
            return true;
        }
    }
    return false;
}

bool string_contains_ci(const std::string& haystack, const std::string& needle) {
    const std::string h = to_lower_ascii(haystack);
    const std::string n = to_lower_ascii(needle);
    return h.find(n) != std::string::npos;
}

std::string detect_android_abi(const fs::path& path) {
    const std::string s = to_lower_ascii(path.string());
    if (s.find("arm64") != std::string::npos || s.find("aarch64") != std::string::npos) {
        return "arm64";
    }
    if (s.find("armeabi-v7a") != std::string::npos || s.find("armv7") != std::string::npos || s.find("arm32") != std::string::npos) {
        return "arm32";
    }
    if (s.find("x86_64") != std::string::npos) {
        return "x86_64";
    }
    return {};
}

bool patch_gdextension_android(const fs::path& gdextension_path,
                               const std::unordered_map<std::string, std::string>& android_entries,
                               std::string& err) {
    std::ifstream in(gdextension_path, std::ios::binary);
    if (!in.is_open()) {
        err = "Failed to open gdextension file: " + gdextension_path.string();
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        const std::string lower = to_lower_ascii(line);
        if (lower.rfind("android.debug.", 0) == 0 || lower.rfind("android.release.", 0) == 0) {
            continue;
        }
        lines.push_back(line);
    }
    in.close();

    std::size_t insert_at = lines.size();
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (lines[i] == "[libraries]") {
            insert_at = i + 1;
            while (insert_at < lines.size() && !lines[insert_at].empty() && lines[insert_at][0] != '[') {
                insert_at++;
            }
            break;
        }
    }

    std::vector<std::string> new_lines;
    new_lines.reserve(lines.size() + android_entries.size());
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i == insert_at) {
            auto emit = [&](const std::string& key) {
                auto it = android_entries.find(key);
                if (it != android_entries.end()) {
                    new_lines.push_back(key + " = \"" + it->second + "\"");
                }
            };
            emit("android.debug.arm64");
            emit("android.release.arm64");
            emit("android.debug.arm32");
            emit("android.release.arm32");
            emit("android.debug.x86_64");
            emit("android.release.x86_64");
        }
        new_lines.push_back(lines[i]);
    }
    if (insert_at == lines.size()) {
        auto emit = [&](const std::string& key) {
            auto it = android_entries.find(key);
            if (it != android_entries.end()) {
                new_lines.push_back(key + " = \"" + it->second + "\"");
            }
        };
        emit("android.debug.arm64");
        emit("android.release.arm64");
        emit("android.debug.arm32");
        emit("android.release.arm32");
        emit("android.debug.x86_64");
        emit("android.release.x86_64");
    }

    std::ofstream out(gdextension_path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        err = "Failed to write gdextension file: " + gdextension_path.string();
        return false;
    }
    for (const auto& l : new_lines) {
        out << l << "\n";
    }
    return true;
}

bool prepare_android_native_libs(const fs::path& native_lib_dir,
                                 const fs::path& staging,
                                 std::string& err) {
    struct Candidate {
        fs::path path;
        int score = 0;
    };
    std::map<std::string, Candidate> best;
    std::map<std::string, Candidate> best_sms;

    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(native_lib_dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".so") continue;
        const std::string filename = to_lower_ascii(entry.path().filename().string());
        const bool is_forge_runner = filename.find("forge_runner_native") != std::string::npos;
        const bool is_sms_native = filename.find("sms_native") != std::string::npos;
        if (!is_forge_runner && !is_sms_native) continue;

        const std::string abi = detect_android_abi(entry.path());
        if (abi.empty()) continue;

        int score = 0;
        const std::string p = to_lower_ascii(entry.path().string());
        if (p.find(".android.") != std::string::npos) score += 10;
        if (p.find("template_debug") != std::string::npos) score += 8;
        if (p.find("debug") != std::string::npos) score += 4;
        if (p.find("template_release") != std::string::npos) score += 2;

        auto* target_map = is_sms_native ? &best_sms : &best;
        auto it = target_map->find(abi);
        if (it == target_map->end() || score > it->second.score) {
            (*target_map)[abi] = Candidate{ entry.path(), score };
        }
    }
    if (ec) {
        err = "Failed to enumerate native library directory: " + native_lib_dir.string();
        return false;
    }

    if (best.find("arm64") == best.end() && best.find("arm32") == best.end()) {
        err =
            "No Android ForgeRunner native libraries found in '" + native_lib_dir.string() + "'. "
            "Expected .so files containing 'forge_runner_native' and ABI hints (arm64/arm32).";
        return false;
    }

    const fs::path bin_dir = staging / "bin";
    fs::create_directories(bin_dir, ec);
    if (ec) {
        err = "Failed to create staging bin dir: " + bin_dir.string();
        return false;
    }

    std::unordered_map<std::string, std::string> gdext_entries;
    auto copy_abi = [&](const std::string& abi_key, const std::string& suffix) -> bool {
        auto it = best.find(abi_key);
        if (it == best.end()) return true;
        const fs::path src = it->second.path;
        const fs::path dst = bin_dir / ("libforge_runner_native.android.template_debug." + suffix + ".so");
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            err = "Failed to copy Android native lib: " + src.string() + " -> " + dst.string();
            return false;
        }
        const std::string res_path = "res://bin/" + dst.filename().string();
        gdext_entries["android.debug." + suffix] = res_path;
        gdext_entries["android.release." + suffix] = res_path;
        return true;
    };

    if (!copy_abi("arm64", "arm64")) return false;
    if (!copy_abi("arm32", "arm32")) return false;
    if (!copy_abi("x86_64", "x86_64")) return false;

    auto copy_sms_abi = [&](const std::string& abi_key, const std::string& suffix) -> bool {
        auto it = best_sms.find(abi_key);
        if (it == best_sms.end()) return true;
        const fs::path src = it->second.path;
        const fs::path dst = bin_dir / ("libsms_native.android.template_debug." + suffix + ".so");
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            err = "Failed to copy Android SMS native lib: " + src.string() + " -> " + dst.string();
            return false;
        }
        return true;
    };
    // Optional: SMS runtime may be linked into forge_runner_native on Android.
    if (!copy_sms_abi("arm64", "arm64")) return false;
    if (!copy_sms_abi("arm32", "arm32")) return false;
    if (!copy_sms_abi("x86_64", "x86_64")) return false;

    if (!patch_gdextension_android(staging / "forge_runner_native.gdextension", gdext_entries, err)) {
        return false;
    }
    return true;
}

std::string make_temp_id() {
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
#if defined(_WIN32)
    const unsigned long pid = static_cast<unsigned long>(GetCurrentProcessId());
#else
    const long pid = static_cast<long>(getpid());
#endif
    return std::to_string(now) + "-" + std::to_string(pid);
}

bool validate_project(const fs::path& project, std::string& err) {
    const char* sml_dir = std::getenv("SML_NATIVE_LIB_DIR");
    const char* sms_dir = std::getenv("SMS_NATIVE_LIB_DIR");
    if (!sml_dir || !sms_dir) {
        err = "Set SML_NATIVE_LIB_DIR and SMS_NATIVE_LIB_DIR before running forgecli build.";
        return false;
    }

    const fs::path sml_lib_path = fs::path(sml_dir) / ("libsmlcore_native" + lib_ext());
    const fs::path sms_lib_path = fs::path(sms_dir) / ("libsms_native" + lib_ext());
    void* sml_lib = load_lib(sml_lib_path);
    void* sms_lib = load_lib(sms_lib_path);
    if (!sml_lib || !sms_lib) {
        if (sml_lib) free_lib(sml_lib);
        if (sms_lib) free_lib(sms_lib);
        err = "Failed to load native parser libraries.";
        return false;
    }

    auto cleanup = [&]() {
        free_lib(sml_lib);
        free_lib(sms_lib);
    };

    auto sml_parse = reinterpret_cast<SmlParseFn>(load_symbol(sml_lib, "smlcore_native_parse"));
    auto sms_create = reinterpret_cast<SmsSessionCreateFn>(load_symbol(sms_lib, "sms_native_session_create"));
    auto sms_load = reinterpret_cast<SmsSessionLoadFn>(load_symbol(sms_lib, "sms_native_session_load"));
    auto sms_dispose = reinterpret_cast<SmsSessionDisposeFn>(load_symbol(sms_lib, "sms_native_session_dispose"));
    auto sms_set_sandbox = reinterpret_cast<SmsSetSandboxPathCallbackFn>(load_symbol(sms_lib, "sms_native_set_sandbox_path_callback"));
    if (!sml_parse || !sms_create || !sms_load || !sms_dispose || !sms_set_sandbox) {
        cleanup();
        err = "Missing native parser symbol(s).";
        return false;
    }

    std::string sandbox_init_error;
    if (!forgecli::initialize_sandbox_roots(project, g_sandbox_roots, sandbox_init_error)) {
        cleanup();
        err = sandbox_init_error;
        return false;
    }
    char sandbox_error[2048] = {0};
    if (sms_set_sandbox(&sandbox_allow_path, sandbox_error, static_cast<int>(sizeof(sandbox_error))) != 0) {
        cleanup();
        err = sandbox_error[0] ? sandbox_error : "Failed to register SMS sandbox callback.";
        return false;
    }

    std::vector<fs::path> scripts;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(project, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        const auto ext = to_lower_ascii(entry.path().extension().string());
        if (ext == ".sml" || ext == ".sms") scripts.push_back(entry.path());
    }
    std::sort(scripts.begin(), scripts.end());
    if (scripts.empty()) {
        cleanup();
        err = "No .sml/.sms files found in project root.";
        return false;
    }

    bool ok = true;
    std::string per_file_error;
    for (const auto& script : scripts) {
        per_file_error.clear();
        const auto ext = to_lower_ascii(script.extension().string());
        const bool file_ok = ext == ".sml"
            ? parse_sml(sml_parse, script, per_file_error)
            : parse_sms(sms_create, sms_load, sms_dispose, script, per_file_error);
        if (file_ok) {
            std::cout << "[OK]   " << script.string() << "\n";
            continue;
        }
        std::cout << "[FAIL] " << script.string() << "\n  error: " << per_file_error << "\n";
        ok = false;
    }

    cleanup();
    if (!ok) {
        err = "Project validation failed.";
    }
    return ok;
}

} // namespace

int cmd_build(const std::vector<std::string>& args) {
    BuildOptions opts;
    std::string err;
    if (!parse_build_options(args, opts, err)) {
        std::cerr << err << "\n";
        return 1;
    }

    if (!validate_project(opts.project, err)) {
        std::cerr << err << "\n";
        return 2;
    }

    if (opts.target == BuildTarget::Android) {
        if (!command_exists("adb")) {
            std::cerr << "[WARN] adb not found in PATH. Android export may fail.\n";
        }
        if (!command_exists("java")) {
            std::cerr << "[WARN] java not found in PATH. Android export may fail.\n";
        }
    }

    const char* home = std::getenv("HOME");
    if (!home || home[0] == '\0') {
        std::cerr << "HOME is not set. Cannot resolve Godot cache path.\n";
        return 1;
    }
    godot_sdk::Config sdk_cfg{
        opts.godot_version,
        fs::path(home) / ".cache" / "forge-runner" / "godot" / opts.godot_version
    };

    const std::string godot_path = godot_sdk::ensure_binary(sdk_cfg, err);
    if (godot_path.empty()) {
        std::cerr << err << "\n";
        return 1;
    }
    if (!godot_sdk::ensure_export_templates(sdk_cfg, err)) {
        std::cerr << err << "\n";
        return 1;
    }

    const std::string project_name = opts.project.filename().string().empty()
        ? std::string("forge-app")
        : opts.project.filename().string();
    const fs::path staging = fs::temp_directory_path() / ("forge-build-" + make_temp_id());
    struct StagingCleanup {
        fs::path path;
        bool enabled = true;
        ~StagingCleanup() {
            if (!enabled || path.empty()) return;
            std::error_code ec;
            fs::remove_all(path, ec);
        }
    } cleanup{ staging };

    if (!copy_recursive(opts.host_project_dir, staging, err)) {
        std::cerr << err << "\n";
        return 1;
    }

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(opts.project, ec)) {
        if (ec) {
            std::cerr << "Failed to enumerate project files: " << opts.project.string() << "\n";
            return 1;
        }
        if (!entry.is_regular_file()) continue;
        const std::string ext = to_lower_ascii(entry.path().extension().string());
        if (ext != ".sml" && ext != ".sms") continue;
        fs::copy_file(entry.path(), staging / entry.path().filename(), fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cerr << "Failed to copy script file: " << entry.path().string() << "\n";
            return 1;
        }
    }

    const fs::path assets_dir = opts.project / "assets";
    if (fs::exists(assets_dir) && fs::is_directory(assets_dir)) {
        if (!copy_recursive(assets_dir, staging / "assets", err)) {
            std::cerr << err << "\n";
            return 1;
        }
    }

    const fs::path addons_dir = opts.project / "addons";
    if (fs::exists(addons_dir) && fs::is_directory(addons_dir)) {
        if (!copy_recursive(addons_dir, staging / "addons", err)) {
            std::cerr << err << "\n";
            return 1;
        }
    }

    bool use_gradle_build = false;
    std::vector<std::string> android_plugins;
    if (opts.target == BuildTarget::Android) {
        const fs::path android_dir = opts.project / "android";
        if (fs::exists(android_dir) && fs::is_directory(android_dir)) {
            if (!copy_recursive(android_dir, staging / "android", err)) {
                std::cerr << err << "\n";
                return 1;
            }
            use_gradle_build = true;
            android_plugins = detect_android_plugins(android_dir);
        }
    }

    if (opts.target == BuildTarget::Android) {
        if (!bundle_sms_llvm_ir_artifacts(opts.project, staging, true, err)) {
            std::cerr << err << "\n";
            return 1;
        }
    }

    if (opts.target == BuildTarget::Android) {
        if (!prepare_android_native_libs(opts.native_lib_dir, staging, err)) {
            std::cerr << err << "\n";
            return 1;
        }
    } else {
        fs::path native_lib_file;
        if (!find_native_library(opts.native_lib_dir, native_lib_file)) {
            std::cerr << "Could not find ForgeRunner native library in: " << opts.native_lib_dir.string() << "\n";
            return 1;
        }
        fs::copy_file(native_lib_file, staging / native_lib_file.filename(), fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cerr << "Failed to copy native library: " << native_lib_file.string() << "\n";
            return 1;
        }
    }

    if (!patch_project_name(staging / "project.godot", project_name, opts.target, err)) {
        std::cerr << err << "\n";
        return 1;
    }

    fs::create_directories(opts.output.parent_path(), ec);

    const std::string preset_name = opts.target == BuildTarget::Mac ? "macOS" : "Android";
    const fs::path export_path = opts.output;

    if (opts.target == BuildTarget::Android) {
        const std::string release_keystore = env_or_empty("FORGE_ANDROID_RELEASE_KEYSTORE");
        const std::string release_user = env_or_empty("FORGE_ANDROID_RELEASE_USER");
        const std::string release_password = env_or_empty("FORGE_ANDROID_RELEASE_PASSWORD");
        if (release_keystore.empty() || release_user.empty() || release_password.empty()) {
            std::cerr << "Missing Android release signing config.\n"
                      << "Set FORGE_ANDROID_RELEASE_KEYSTORE, FORGE_ANDROID_RELEASE_USER, FORGE_ANDROID_RELEASE_PASSWORD.\n";
            return 1;
        }
        const fs::path release_keystore_path = fs::absolute(release_keystore);
        if (!fs::exists(release_keystore_path)) {
            std::cerr << "Android release keystore not found: " << release_keystore_path.string() << "\n";
            return 1;
        }
    }

    const std::string preset_text = opts.target == BuildTarget::Mac
        ? mac_export_preset(export_path)
        : android_export_preset(export_path, project_name, opts.android_package_id, use_gradle_build, android_plugins);
    if (!write_text_file(staging / "export_presets.cfg", preset_text, err)) {
        std::cerr << err << "\n";
        return 1;
    }

    const fs::path export_log_path = staging / "godot_export.log";
    const std::string export_mode = "--export-release";
    const bool should_install_android_template =
        opts.target == BuildTarget::Android && use_gradle_build;
    const std::string install_android_template_flag =
        should_install_android_template ? " --install-android-build-template" : "";
    const std::string export_cmd_base = shell_quote(godot_path)
        + " --headless --path " + shell_quote(staging.string())
        + install_android_template_flag
        + " " + export_mode + " " + shell_quote(preset_name)
        + " " + shell_quote(export_path.string());
    const std::string export_cmd = export_cmd_base + " > " + shell_quote(export_log_path.string()) + " 2>&1";
    const int export_rc = run_command(export_cmd);
    if (export_rc != 0) {
        cleanup.enabled = false;
        std::cerr << "Godot export failed (exit " << export_rc << ").\n";
        std::cerr << "Command: " << export_cmd_base << "\n";
        const std::string log_text = read_text_file_best_effort(export_log_path);
        const std::string filtered_log_text = filter_noisy_godot_log_lines(log_text);
        if (!filtered_log_text.empty()) {
            std::cerr << "---- godot_export.log ----\n";
            std::cerr << filtered_log_text;
            if (filtered_log_text.back() != '\n') std::cerr << "\n";
            std::cerr << "---- end godot_export.log ----\n";
        } else {
            std::cerr << "No export log written at: " << export_log_path.string() << "\n";
        }
        std::cerr << "Staging kept at: " << staging.string() << "\n";
        return 1;
    }

    std::cout << "[OK] " << opts.output.string() << "\n";
    return 0;
}

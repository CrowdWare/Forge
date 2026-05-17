/*
#############################################################################
# Copyright (C) 2026 CrowdWare
#
# This file is part of Forge.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-CrowdWare-Commercial
#
# Forge is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Forge is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Forge. If not, see <https://www.gnu.org/licenses/>.
#
# Commercial licensing is available from CrowdWare for proprietary use.
#############################################################################
*/

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>

#include "cmd_build.h"
#include "sandbox_policy.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

using SmlParseFn = int (*)(const char*, std::int64_t*, char*, int);
using SmsExecuteFn = int (*)(const char*, std::int64_t*, char*, int);
using SmsSessionCreateFn = int (*)(std::int64_t*, char*, int);
using SmsSessionLoadFn = int (*)(std::int64_t, const char*, char*, int);
using SmsSessionInvokeFn = int (*)(std::int64_t, const char*, const char*, const char*, std::int64_t*, char*, int);
using SmsSessionDisposeFn = int (*)(std::int64_t, char*, int);
using SmsCodegenLlvmIrFn = int (*)(const char*, char*, int, char*, int);
using SmsCodegenLlvmIrModeFn = int (*)(const char*, const char*, char*, int, char*, int);
using SmsSandboxPathAllowFn = int (*)(const char*, const char*, char*, int);
using SmsSetSandboxPathCallbackFn = int (*)(SmsSandboxPathAllowFn, char*, int);
using SmsUiGetPropFn = int (*)(const char*, const char*, char*, int, char*, int);
using SmsUiSetPropFn = int (*)(const char*, const char*, const char*, char*, int);
using SmsUiInvokeFn = int (*)(const char*, const char*, const char*, char*, int, char*, int);
using SmsSetUiCallbacksFn = int (*)(SmsUiGetPropFn, SmsUiSetPropFn, SmsUiInvokeFn, char*, int);

static forgecli::SandboxRoots g_sandbox_roots;

static int sandbox_allow_path(const char* owner, const char* uri_path, char* error, int error_capacity) {
    return forgecli::sandbox_allow_path(g_sandbox_roots, owner, uri_path, error, error_capacity);
}

static void* load_symbol(void* lib, const char* name) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(lib), name));
#else
    return dlsym(lib, name);
#endif
}

static void* load_lib(const std::string& file) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(LoadLibraryA(file.c_str()));
#else
    return dlopen(file.c_str(), RTLD_NOW);
#endif
}

static std::string ext() {
#if defined(_WIN32)
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

static bool command_exists_in_path(const std::string& command) {
    const char* path_env = std::getenv("PATH");
    if (path_env == nullptr || path_env[0] == '\0') {
        return false;
    }

    std::string path(path_env);
#if defined(_WIN32)
    const char sep = ';';
#else
    const char sep = ':';
#endif
    std::size_t start = 0;
    while (start <= path.size()) {
        std::size_t end = path.find(sep, start);
        if (end == std::string::npos) end = path.size();
        const auto dir = path.substr(start, end - start);
        if (!dir.empty()) {
            fs::path candidate = fs::path(dir) / command;
            if (fs::exists(candidate)) return true;
#if defined(_WIN32)
            candidate = fs::path(dir) / (command + ".exe");
            if (fs::exists(candidate)) return true;
#endif
        }
        if (end == path.size()) break;
        start = end + 1;
    }
    return false;
}

static bool parse_sml(SmlParseFn fn, const fs::path& p, std::string& err) {
    if (!fs::exists(p)) {
        err = "File not found.";
        return false;
    }
    std::ifstream in(p, std::ios::binary);
    std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    char e[2048] = {0};
    std::int64_t nodes = 0;
    if (fn(src.c_str(), &nodes, e, static_cast<int>(sizeof(e))) == 0) {
        return true;
    }
    err = e[0] ? std::string(e) : "SML parse failed";
    return false;
}

static bool parse_sms(SmsSessionCreateFn create_fn, SmsSessionLoadFn load_fn, SmsSessionDisposeFn dispose_fn, const fs::path& p, std::string& err) {
    if (!fs::exists(p)) {
        err = "File not found.";
        return false;
    }
    std::ifstream in(p, std::ios::binary);
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

static std::string read_file_text(const fs::path& p, std::string& err) {
    if (!fs::exists(p)) {
        err = "File not found.";
        return {};
    }
    std::ifstream in(p, std::ios::binary);
    if (!in.is_open()) {
        err = "Failed to open file.";
        return {};
    }
    std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return src;
}

static int cli_ui_get_stub(const char*, const char*, char* out_json, int out_json_capacity, char*, int) {
    if (out_json != nullptr && out_json_capacity > 0) {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
    }
    return 0;
}

static int cli_ui_set_stub(const char*, const char*, const char*, char*, int) {
    return 0;
}

static int cli_ui_invoke_stub(
    const char* object_id,
    const char* method,
    const char* args_json,
    char* out_json,
    int out_json_capacity,
    char* error,
    int error_capacity) {
    const std::string object = object_id ? object_id : "";
    const std::string method_name = method ? method : "";
    const std::string args = args_json ? args_json : "[]";

    if (object == "__log__") {
        std::cout << "[sms." << method_name << "] " << args << "\n";
        if (out_json != nullptr && out_json_capacity > 0) {
            std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
        }
        return 0;
    }

    if (error != nullptr && error_capacity > 0) {
        std::snprintf(error, static_cast<std::size_t>(error_capacity), "Unknown invoke target: %s.%s", object.c_str(), method_name.c_str());
    }
    return 1;
}

static bool init_sms_sandbox_for_file(
    SmsSetSandboxPathCallbackFn set_sandbox_cb,
    const fs::path& sms_file,
    std::string& err) {
    if (set_sandbox_cb == nullptr) {
        return true;
    }

    const fs::path project_root = sms_file.parent_path();
    if (!forgecli::initialize_sandbox_roots(project_root, g_sandbox_roots, err)) {
        return false;
    }

    char callback_error[2048] = {0};
    if (set_sandbox_cb(&sandbox_allow_path, callback_error, static_cast<int>(sizeof(callback_error))) != 0) {
        err = callback_error[0] ? std::string(callback_error) : "Failed to register SMS sandbox callback.";
        return false;
    }
    return true;
}

static bool split_handler_key(const std::string& full, std::string& out_target, std::string& out_event) {
    const auto dot = full.find('.');
    if (dot == std::string::npos || dot == 0 || dot + 1 >= full.size()) {
        return false;
    }
    out_target = full.substr(0, dot);
    out_event = full.substr(dot + 1);
    return true;
}

static std::string quote_shell_arg(const fs::path& p) {
    std::string s = p.string();
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

static int cmd_sms(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Missing sms subcommand. Use: sms run <file.sms> | sms llvm-ir <file.sms> [--out <file.ll>] [--mode <exe|lib>] | sms build <file.sms> [--out <binary>] | sms demo [--build] [--out <binary>] [--force]\n";
        return 1;
    }

    const std::string sub = args[0];
    if (sub == "demo") {
        bool build_binary = false;
        bool force = false;
        fs::path out_binary = "main";
        fs::path demo_file = "main.sms";

        for (std::size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--build") {
                build_binary = true;
            } else if (args[i] == "--force") {
                force = true;
            } else if (args[i] == "--out" && i + 1 < args.size()) {
                out_binary = args[++i];
            } else if (args[i] == "--file" && i + 1 < args.size()) {
                demo_file = args[++i];
            } else {
                std::cerr << "Unknown option: " << args[i] << "\n";
                return 1;
            }
        }

        if (fs::exists(demo_file) && !force) {
            std::cerr << "Demo file already exists: " << demo_file << " (use --force to overwrite)\n";
            return 1;
        }

        std::ofstream out(demo_file, std::ios::binary);
        if (!out.is_open()) {
            std::cerr << "Failed to write demo file: " << demo_file << "\n";
            return 1;
        }
        out << "fun getAnswer() {\n"
               "    return 42\n"
               "}\n"
               "\n"
               "log.success(\"The answer of all questions is: ${getAnswer()}\")\n"
               "getAnswer()\n";
        out.close();

        std::cout << "Wrote demo: " << demo_file << "\n";
        const int run_rc = cmd_sms({"run", demo_file.string()});
        if (run_rc != 0) {
            return run_rc;
        }
        if (build_binary) {
            return cmd_sms({"build", demo_file.string(), "--out", out_binary.string()});
        }
        return 0;
    }

    if (sub != "run" && sub != "llvm-ir" && sub != "build") {
        std::cerr << "Unknown sms subcommand: " << sub << "\n";
        return 1;
    }

    if (args.size() < 2) {
        std::cerr << "Missing SMS file path.\n";
        return 1;
    }

    const fs::path sms_file = args[1];
    std::string read_err;
    const std::string source = read_file_text(sms_file, read_err);
    if (source.empty() && !read_err.empty()) {
        std::cerr << read_err << "\n";
        return 1;
    }

    const char* sms_dir = std::getenv("SMS_NATIVE_LIB_DIR");
    if (!sms_dir) {
        std::cerr << "Set SMS_NATIVE_LIB_DIR.\n";
        return 1;
    }

    const std::string sms_lib_path = (fs::path(sms_dir) / ("libsms_native" + ext())).string();
    void* sms_lib = load_lib(sms_lib_path);
    if (!sms_lib) {
        std::cerr << "Failed to load sms native library: " << sms_lib_path << "\n";
        return 1;
    }

    auto sms_set_sandbox = reinterpret_cast<SmsSetSandboxPathCallbackFn>(load_symbol(sms_lib, "sms_native_set_sandbox_path_callback"));
    std::string sandbox_err;
    if (!init_sms_sandbox_for_file(sms_set_sandbox, sms_file, sandbox_err)) {
        std::cerr << sandbox_err << "\n";
        return 1;
    }

    if (sub == "llvm-ir" || sub == "build") {
        fs::path out_file;
        std::string llvm_mode = "exe";
        bool keep_ir = false;
        for (std::size_t i = 2; i < args.size(); i++) {
            if (args[i] == "--out" && i + 1 < args.size()) {
                out_file = args[++i];
            } else if (args[i] == "--mode" && i + 1 < args.size()) {
                llvm_mode = args[++i];
            } else if (args[i] == "--keep-ir") {
                keep_ir = true;
            } else {
                std::cerr << "Unknown option: " << args[i] << "\n";
                return 1;
            }
        }
        if (llvm_mode != "exe" && llvm_mode != "lib") {
            std::cerr << "--mode expects 'exe' or 'lib', got: " << llvm_mode << "\n";
            return 1;
        }

        auto sms_codegen_ir = reinterpret_cast<SmsCodegenLlvmIrFn>(load_symbol(sms_lib, "sms_native_codegen_llvm_ir"));
        auto sms_codegen_ir_mode = reinterpret_cast<SmsCodegenLlvmIrModeFn>(load_symbol(sms_lib, "sms_native_codegen_llvm_ir_mode"));
        if (!sms_codegen_ir) {
            std::cerr << "Missing symbol: sms_native_codegen_llvm_ir\n";
            return 1;
        }

        int capacity = 64 * 1024;
        std::string ir_text;
        while (capacity <= 8 * 1024 * 1024) {
            std::vector<char> buffer(static_cast<std::size_t>(capacity), '\0');
            char error[2048] = {0};
            const int rc = sms_codegen_ir_mode != nullptr
                ? sms_codegen_ir_mode(source.c_str(), llvm_mode.c_str(), buffer.data(), capacity, error, static_cast<int>(sizeof(error)))
                : sms_codegen_ir(source.c_str(), buffer.data(), capacity, error, static_cast<int>(sizeof(error)));
            if (rc == 0) {
                ir_text = std::string(buffer.data());
                break;
            }
            const std::string err = error;
            if (err.find("output buffer too small") != std::string::npos) {
                capacity *= 2;
                continue;
            }
            std::cerr << (err.empty() ? "llvm-ir generation failed" : err) << "\n";
            return 2;
        }
        if (ir_text.empty()) {
            std::cerr << "llvm-ir output exceeds supported buffer size\n";
            return 2;
        }

        if (sub == "llvm-ir") {
            if (!out_file.empty()) {
                std::ofstream out(out_file, std::ios::binary);
                if (!out.is_open()) {
                    std::cerr << "Failed to open output file: " << out_file << "\n";
                    return 1;
                }
                out << ir_text;
                std::cout << "Wrote LLVM IR: " << out_file << "\n";
            } else {
                std::cout << ir_text << "\n";
            }
            return 0;
        }

        if (out_file.empty()) {
#if defined(_WIN32)
            out_file = sms_file.stem().string() + ".exe";
#else
            out_file = sms_file.stem();
#endif
        }

        const fs::path ir_file = out_file.string() + ".ll";
        {
            std::error_code ec;
            fs::remove(out_file, ec);
        }
        {
            std::ofstream out(ir_file, std::ios::binary);
            if (!out.is_open()) {
                std::cerr << "Failed to open IR output file: " << ir_file << "\n";
                return 1;
            }
            out << ir_text;
        }

        if (llvm_mode == "lib") {
            std::cerr << "sms build currently supports only --mode exe.\n";
            return 1;
        }
        const std::string cmd = "clang -O2 -o " + quote_shell_arg(out_file) + " " + quote_shell_arg(ir_file);
        const int rc = std::system(cmd.c_str());
        if (rc != 0) {
            std::cerr << "clang build failed (" << rc << "). Command: " << cmd << "\n";
            return 2;
        }
        if (!keep_ir) {
            std::error_code ec;
            fs::remove(ir_file, ec);
        }

        std::cout << "Built binary: " << out_file << "\n";
        return 0;
    }

    auto sms_execute = reinterpret_cast<SmsExecuteFn>(load_symbol(sms_lib, "sms_native_execute"));
    auto sms_create = reinterpret_cast<SmsSessionCreateFn>(load_symbol(sms_lib, "sms_native_session_create"));
    auto sms_load = reinterpret_cast<SmsSessionLoadFn>(load_symbol(sms_lib, "sms_native_session_load"));
    auto sms_invoke = reinterpret_cast<SmsSessionInvokeFn>(load_symbol(sms_lib, "sms_native_session_invoke"));
    auto sms_dispose = reinterpret_cast<SmsSessionDisposeFn>(load_symbol(sms_lib, "sms_native_session_dispose"));
    auto sms_set_ui = reinterpret_cast<SmsSetUiCallbacksFn>(load_symbol(sms_lib, "sms_native_set_ui_callbacks"));
    if (!sms_execute || !sms_create || !sms_load || !sms_invoke || !sms_dispose || !sms_set_ui) {
        std::cerr << "Missing required SMS symbol(s).\n";
        return 1;
    }

    char ui_error[2048] = {0};
    if (sms_set_ui(&cli_ui_get_stub, &cli_ui_set_stub, &cli_ui_invoke_stub, ui_error, static_cast<int>(sizeof(ui_error))) != 0) {
        std::cerr << (ui_error[0] ? ui_error : "Failed to register UI callbacks.") << "\n";
        return 1;
    }

    std::string invoke_target_event;
    std::string invoke_args = "[]";
    for (std::size_t i = 2; i < args.size(); i++) {
        if (args[i] == "--invoke" && i + 1 < args.size()) {
            invoke_target_event = args[++i];
        } else if (args[i] == "--args" && i + 1 < args.size()) {
            invoke_args = args[++i];
        } else {
            std::cerr << "Unknown option: " << args[i] << "\n";
            return 1;
        }
    }

    if (invoke_target_event.empty()) {
        char error[2048] = {0};
        std::int64_t result = 0;
        const int rc = sms_execute(source.c_str(), &result, error, static_cast<int>(sizeof(error)));
        if (rc != 0) {
            std::cerr << (error[0] ? error : "sms execution failed") << "\n";
            return 2;
        }
        std::cout << "result: " << result << "\n";
        return 0;
    }

    std::string target_id;
    std::string event_name;
    if (!split_handler_key(invoke_target_event, target_id, event_name)) {
        std::cerr << "--invoke expects <target.event>, got: " << invoke_target_event << "\n";
        return 1;
    }

    std::int64_t session = -1;
    char error[2048] = {0};
    if (sms_create(&session, error, static_cast<int>(sizeof(error))) != 0 || session < 0) {
        std::cerr << (error[0] ? error : "sms session create failed") << "\n";
        return 2;
    }
    if (sms_load(session, source.c_str(), error, static_cast<int>(sizeof(error))) != 0) {
        sms_dispose(session, nullptr, 0);
        std::cerr << (error[0] ? error : "sms session load failed") << "\n";
        return 2;
    }

    std::int64_t result = 0;
    const int rc = sms_invoke(
        session,
        target_id.c_str(),
        event_name.c_str(),
        invoke_args.c_str(),
        &result,
        error,
        static_cast<int>(sizeof(error)));
    sms_dispose(session, nullptr, 0);
    if (rc != 0) {
        std::cerr << (error[0] ? error : "sms invoke failed") << "\n";
        return 2;
    }

    std::cout << "result: " << result << "\n";
    return 0;
}

static int cmd_toolchain(const std::vector<std::string>& args) {
    if (args.empty() || args[0] != "doctor") {
        std::cerr << "Usage: forgecli-native toolchain doctor\n";
        return 1;
    }

    bool ok = true;
    auto report = [&](const std::string& name, bool pass, const std::string& detail) {
        std::cout << (pass ? "[OK]   " : "[FAIL] ") << name;
        if (!detail.empty()) {
            std::cout << " - " << detail;
        }
        std::cout << "\n";
        if (!pass) ok = false;
    };

    report("cmake", command_exists_in_path("cmake"), "required for native builds");
    report("clang", command_exists_in_path("clang"), "required for sms build");

    const char* sms_lib_dir = std::getenv("SMS_NATIVE_LIB_DIR");
    if (!sms_lib_dir || sms_lib_dir[0] == '\0') {
        report("SMS_NATIVE_LIB_DIR", false, "not set");
    } else {
        const fs::path sms_lib = fs::path(sms_lib_dir) / ("libsms_native" + ext());
        report("SMS_NATIVE_LIB_DIR", fs::exists(sms_lib), sms_lib.string());
    }

    const char* sml_lib_dir = std::getenv("SML_NATIVE_LIB_DIR");
    if (!sml_lib_dir || sml_lib_dir[0] == '\0') {
        report("SML_NATIVE_LIB_DIR", false, "not set (needed for validate/build)");
    } else {
        const fs::path sml_lib = fs::path(sml_lib_dir) / ("libsmlcore_native" + ext());
        report("SML_NATIVE_LIB_DIR", fs::exists(sml_lib), sml_lib.string());
    }

    std::cout << "\n" << (ok ? "Toolchain doctor passed." : "Toolchain doctor found issues.") << "\n";
    return ok ? 0 : 2;
}

static int cmd_new(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Missing project name.\n";
        return 1;
    }
    fs::path out = fs::current_path() / args[0];
    bool force = false;
    for (std::size_t i = 1; i < args.size(); i++) {
        if (args[i] == "--force") {
            force = true;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            out = args[++i];
        } else {
            std::cerr << "Unknown option: " << args[i] << "\n";
            return 1;
        }
    }
    std::error_code ec;
    if (fs::exists(out) && !fs::is_empty(out, ec) && !force) {
        std::cerr << "Directory not empty. Use --force.\n";
        return 1;
    }
    fs::create_directories(out / "assets", ec);

    std::ofstream(out / "app.sml")
        << "SplashScreen {\n"
           "    id: splashScreen\n"
           "    size: 640, 480\n"
           "    duration: 500\n"
           "    loadOnReady: \"main.sml\"\n"
           "\n"
           "    VBoxContainer {\n"
           "        anchors: left | top | right | bottom\n"
           "        padding: 20, 20, 20, 20\n"
           "\n"
           "        Control { sizeFlagsVertical: expandFill }\n"
           "\n"
           "        Label {\n"
           "            text: \"Loading...\"\n"
           "            sizeFlagsHorizontal: shrinkCenter\n"
           "        }\n"
           "\n"
           "        Control { sizeFlagsVertical: expandFill }\n"
           "    }\n"
           "}\n";

    std::ofstream(out / "main.sml")
        << "Window {\n"
           "    id: mainWindow\n"
           "    title: @Strings.windowTitle, \"" << args[0] << "\"\n"
           "    minSize: 900, 600\n"
           "    size: 1200, 800\n"
           "\n"
           "    DockingHost {\n"
           "        id: mainDockHost\n"
           "        anchors: left | top | right | bottom\n"
           "\n"
           "        DockingContainer {\n"
           "            id: centerDock\n"
           "            dockSide: center\n"
           "            flex: true\n"
           "            closeable: false\n"
           "\n"
           "            Viewport3D {\n"
           "                id: viewport\n"
           "                centerDock.title: \"Viewport\"\n"
           "                anchors: left | top | right | bottom\n"
           "            }\n"
           "        }\n"
           "    }\n"
           "}\n";

    std::ofstream(out / "main.sms")
        << "fun ready() {\n"
           "    log.info(\"Forge app ready\")\n"
           "}\n";

    std::ofstream(out / "theme.sml")
        << "Colors {\n"
           "    accent: \"#28A9E0\"\n"
           "}\n";

    std::ofstream(out / "strings.sml")
        << "Strings {\n"
           "    windowTitle: \"" << args[0] << "\"\n"
           "}\n";

    std::ofstream(out / "README.md")
        << "# " << args[0] << "\n"
           "\n"
           "Generated by ForgeCli.\n"
           "\n"
           "## Files\n"
           "\n"
           "- app.sml (startup splash)\n"
           "- main.sml (main UI)\n"
           "- main.sms (event logic)\n"
           "- theme.sml (theme overrides)\n"
           "- strings.sml (localized strings)\n"
           "\n"
           "## Validate\n"
           "\n"
           "```bash\n"
           "forgecli validate --project .\n"
           "```\n"
           "\n"
           "## Generate from AI prompt\n"
           "\n"
           "```bash\n"
           "forgecli generate --project . --provider mock --prompt \"create a window with docking and centered viewport3d\"\n"
           "```\n";

    std::cout << "Created scaffold at '" << fs::absolute(out).string() << "'.\n";
    return 0;
}

static int cmd_validate(const std::vector<std::string>& args) {
    fs::path project = fs::current_path();
    for (std::size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--project" && i + 1 < args.size()) {
            project = args[++i];
        } else if (args[i] != "--verbose") {
            std::cerr << "Unknown option: " << args[i] << "\n";
            return 1;
        }
    }

    const char* sml_dir = std::getenv("SML_NATIVE_LIB_DIR");
    const char* sms_dir = std::getenv("SMS_NATIVE_LIB_DIR");
    if (!sml_dir || !sms_dir) {
        std::cerr << "Set SML_NATIVE_LIB_DIR and SMS_NATIVE_LIB_DIR.\n";
        return 1;
    }

    const std::string sml_lib_path = (fs::path(sml_dir) / ("libsmlcore_native" + ext())).string();
    const std::string sms_lib_path = (fs::path(sms_dir) / ("libsms_native" + ext())).string();
    void* sml_lib = load_lib(sml_lib_path);
    void* sms_lib = load_lib(sms_lib_path);
    if (!sml_lib || !sms_lib) {
        std::cerr << "Failed to load native libraries.\n";
        return 1;
    }

    auto sml_parse = reinterpret_cast<SmlParseFn>(load_symbol(sml_lib, "smlcore_native_parse"));
    auto sms_create = reinterpret_cast<SmsSessionCreateFn>(load_symbol(sms_lib, "sms_native_session_create"));
    auto sms_load = reinterpret_cast<SmsSessionLoadFn>(load_symbol(sms_lib, "sms_native_session_load"));
    auto sms_dispose = reinterpret_cast<SmsSessionDisposeFn>(load_symbol(sms_lib, "sms_native_session_dispose"));
    auto sms_set_sandbox = reinterpret_cast<SmsSetSandboxPathCallbackFn>(load_symbol(sms_lib, "sms_native_set_sandbox_path_callback"));
    if (!sml_parse || !sms_create || !sms_load || !sms_dispose || !sms_set_sandbox) {
        std::cerr << "Missing native symbol(s).\n";
        return 1;
    }

    std::string sandbox_init_error;
    if (!forgecli::initialize_sandbox_roots(project, g_sandbox_roots, sandbox_init_error)) {
        std::cerr << sandbox_init_error << "\n";
        return 1;
    }

    char sandbox_error[2048] = {0};
    if (sms_set_sandbox(&sandbox_allow_path, sandbox_error, static_cast<int>(sizeof(sandbox_error))) != 0) {
        std::cerr << (sandbox_error[0] ? sandbox_error : "Failed to register SMS sandbox callback.") << "\n";
        return 1;
    }

    bool ok = true;
    std::string err;
    const fs::path app = project / "app.sml";
    const fs::path main_sml = project / "main.sml";
    const fs::path main_sms = project / "main.sms";

    if (parse_sml(sml_parse, app, err)) std::cout << "[OK]   " << app.string() << "\n";
    else { std::cout << "[FAIL] " << app.string() << "\n  error: " << err << "\n"; ok = false; }
    err.clear();
    if (parse_sml(sml_parse, main_sml, err)) std::cout << "[OK]   " << main_sml.string() << "\n";
    else { std::cout << "[FAIL] " << main_sml.string() << "\n  error: " << err << "\n"; ok = false; }
    err.clear();
    if (parse_sms(sms_create, sms_load, sms_dispose, main_sms, err)) std::cout << "[OK]   " << main_sms.string() << "\n";
    else { std::cout << "[FAIL] " << main_sms.string() << "\n  error: " << err << "\n"; ok = false; }

    std::cout << "\n" << (ok ? "Validation passed." : "Validation failed.") << "\n";
    return ok ? 0 : 2;
}

static void help() {
    std::cout << "forgecli-native\n\n";
    std::cout << "Usage:\n";
    std::cout << "  forgecli-native new <name> [--output <dir>] [--force]\n";
    std::cout << "  forgecli-native validate [--project <dir>]\n";
    std::cout << "  forgecli-native build mac     --project <dir> [--output <path>] [--godot-version <ver>]\n";
    std::cout << "  forgecli-native build android --project <dir> [--output <path>] [--godot-version <ver>] [--android-package-id <id>]\n";
    std::cout << "  forgecli-native run mac       --project <dir> [--godot-version <ver>]\n";
    std::cout << "  forgecli-native toolchain doctor\n";
    std::cout << "  forgecli-native sms run <file.sms> [--invoke <target.event>] [--args <json-array>]\n";
    std::cout << "  forgecli-native sms llvm-ir <file.sms> [--out <file.ll>] [--mode <exe|lib>]\n";
    std::cout << "  forgecli-native sms build <file.sms> [--out <binary>] [--keep-ir]\n";
    std::cout << "  forgecli-native sms demo [--build] [--out <binary>] [--force]\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        help();
        return 0;
    }
    std::string cmd = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++) args.emplace_back(argv[i]);

    if (cmd == "new") return cmd_new(args);
    if (cmd == "validate") return cmd_validate(args);
    if (cmd == "build") return cmd_build(args);
    if (cmd == "run") return cmd_run(args);
    if (cmd == "toolchain") return cmd_toolchain(args);
    if (cmd == "sms") return cmd_sms(args);
    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        help();
        return 0;
    }
    std::cerr << "Unknown command: " << cmd << "\n";
    return 1;
}

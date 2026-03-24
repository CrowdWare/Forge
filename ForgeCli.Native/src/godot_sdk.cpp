#include "godot_sdk.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace godot_sdk {
namespace {

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

bool run_command(const std::string& cmd, std::string& err) {
    const int rc = std::system(cmd.c_str());
    if (rc == 0) return true;
    err = "Command failed (exit " + std::to_string(rc) + "): " + cmd;
    return false;
}

bool download_file(const std::string& url, const fs::path& dst, std::string& err) {
    std::error_code ec;
    fs::create_directories(dst.parent_path(), ec);
    std::cout << "[DL] " << url << "\n";
    const std::string cmd =
        "curl -L -# -o " + shell_quote(dst.string()) + " " + shell_quote(url);
    return run_command(cmd, err);
}

bool unzip_file(const fs::path& src, const fs::path& dst, std::string& err) {
    std::error_code ec;
    fs::remove_all(dst, ec);
    fs::create_directories(dst, ec);
    const std::string cmd =
        "unzip -q " + shell_quote(src.string()) + " -d " + shell_quote(dst.string());
    return run_command(cmd, err);
}

bool copy_directory_contents(const fs::path& from, const fs::path& to, std::string& err) {
    std::error_code ec;
    fs::create_directories(to, ec);
    for (const auto& entry : fs::recursive_directory_iterator(from, ec)) {
        if (ec) break;
        const fs::path rel = fs::relative(entry.path(), from, ec);
        if (ec) {
            err = "Failed to compute relative path while copying templates.";
            return false;
        }
        const fs::path dst = to / rel;
        if (entry.is_directory()) {
            fs::create_directories(dst, ec);
            if (ec) {
                err = "Failed to create directory: " + dst.string();
                return false;
            }
            continue;
        }
        if (!entry.is_regular_file()) {
            continue;
        }
        fs::create_directories(dst.parent_path(), ec);
        if (ec) {
            err = "Failed to create directory: " + dst.parent_path().string();
            return false;
        }
        fs::copy_file(entry.path(), dst, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            err = "Failed to copy file: " + entry.path().string() + " -> " + dst.string();
            return false;
        }
    }
    if (ec) {
        err = "Failed to iterate directory: " + from.string();
        return false;
    }
    return true;
}

fs::path find_first_named_file(const fs::path& root, const std::string& exact_name) {
    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(root, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        if (entry.path().filename().string() == exact_name) {
            return entry.path();
        }
    }
    return {};
}

fs::path find_templates_dir(const fs::path& root) {
    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(root, ec)) {
        if (ec) break;
        if (entry.is_directory() && entry.path().filename() == "templates") {
            return entry.path();
        }
    }
    return {};
}

std::string host_binary_archive_name(const std::string& version) {
#if defined(_WIN32)
    return "Godot_v" + version + "_win64.exe.zip";
#elif defined(__APPLE__)
    return "Godot_v" + version + "_macos.universal.zip";
#else
    return "Godot_v" + version + "_linux.x86_64.zip";
#endif
}

std::string host_binary_filename() {
#if defined(_WIN32)
    return "godot.exe";
#else
    return "godot";
#endif
}

} // namespace

std::string normalize_version(const std::string& version) {
    std::string out = version;
    std::replace(out.begin(), out.end(), '-', '.');
    return out;
}

fs::path template_install_dir(const std::string& version_normalized) {
#if defined(_WIN32)
    const char* appdata = std::getenv("APPDATA");
    if (!appdata || appdata[0] == '\0') {
        return {};
    }
    return fs::path(appdata) / "Godot" / "export_templates" / version_normalized;
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (!home || home[0] == '\0') {
        return {};
    }
    return fs::path(home) / "Library" / "Application Support" / "Godot" / "export_templates" / version_normalized;
#else
    const char* home = std::getenv("HOME");
    if (!home || home[0] == '\0') {
        return {};
    }
    return fs::path(home) / ".local" / "share" / "godot" / "export_templates" / version_normalized;
#endif
}

std::string ensure_binary(const Config& cfg, std::string& err) {
    fs::path binary_path = cfg.cache / host_binary_filename();
#if defined(__APPLE__)
    binary_path = cfg.cache / "Godot.app" / "Contents" / "MacOS" / "Godot";
#endif
    std::error_code ec;
    if (fs::exists(binary_path, ec)) {
        return binary_path.string();
    }

    fs::create_directories(cfg.cache, ec);
    if (ec) {
        err = "Failed to create Godot cache directory: " + cfg.cache.string();
        return "";
    }

    const std::string archive_name = host_binary_archive_name(cfg.version);
    const std::string base_url =
        "https://github.com/godotengine/godot/releases/download/" + cfg.version + "/";
    const std::string archive_url = base_url + archive_name;
    const fs::path archive_path = cfg.cache / archive_name;
    if (!download_file(archive_url, archive_path, err)) {
        return "";
    }

    const fs::path extract_dir = cfg.cache / "_bin_extract";
    if (!unzip_file(archive_path, extract_dir, err)) {
        return "";
    }

#if defined(__APPLE__)
    fs::path extracted_binary = extract_dir / "Godot.app" / "Contents" / "MacOS" / "Godot";
    if (!fs::exists(extracted_binary)) {
        extracted_binary = find_first_named_file(extract_dir, "Godot");
    }
#elif defined(_WIN32)
    fs::path extracted_binary = find_first_named_file(extract_dir, "Godot_v" + cfg.version + "_win64.exe");
    if (extracted_binary.empty()) {
        extracted_binary = find_first_named_file(extract_dir, "Godot.exe");
    }
#else
    fs::path extracted_binary = find_first_named_file(extract_dir, "Godot_v" + cfg.version + "_linux.x86_64");
    if (extracted_binary.empty()) {
        extracted_binary = find_first_named_file(extract_dir, "Godot");
    }
#endif

    if (extracted_binary.empty() || !fs::exists(extracted_binary)) {
        err = "Could not locate extracted Godot binary in: " + extract_dir.string();
        return "";
    }

#if defined(__APPLE__)
    const fs::path extracted_app = extract_dir / "Godot.app";
    if (!fs::exists(extracted_app) || !fs::is_directory(extracted_app)) {
        err = "Could not locate extracted Godot.app in: " + extract_dir.string();
        return "";
    }

    const fs::path cached_app = cfg.cache / "Godot.app";
    fs::remove_all(cached_app, ec);
    ec.clear();
    fs::create_directories(cfg.cache, ec);
    if (ec) {
        err = "Failed to create cache directory for Godot.app: " + cfg.cache.string();
        return "";
    }
    fs::copy(extracted_app, cached_app, fs::copy_options::recursive, ec);
    if (ec) {
        err = "Failed to copy Godot.app to cache: " + cached_app.string();
        return "";
    }
    return (cached_app / "Contents" / "MacOS" / "Godot").string();
#else
    fs::copy_file(extracted_binary, binary_path, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        err = "Failed to copy Godot binary to cache: " + binary_path.string();
        return "";
    }

#if !defined(_WIN32)
    fs::permissions(binary_path,
                    fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                    fs::perm_options::add,
                    ec);
    if (ec) {
        err = "Failed to set executable bit on: " + binary_path.string();
        return "";
    }
#endif

    return binary_path.string();
#endif
}

bool ensure_export_templates(const Config& cfg, std::string& err) {
    const std::string version_normalized = normalize_version(cfg.version);
    const fs::path install_dir = template_install_dir(version_normalized);
    if (install_dir.empty()) {
        err = "Cannot resolve Godot template install dir (HOME/APPDATA missing).";
        return false;
    }

    const fs::path ready_sentinel = cfg.cache / ".ready";
    std::error_code ec;
    if (fs::exists(ready_sentinel, ec) && fs::exists(install_dir, ec)) {
        return true;
    }

    fs::create_directories(cfg.cache, ec);
    if (ec) {
        err = "Failed to create Godot cache directory: " + cfg.cache.string();
        return false;
    }

    const std::string archive_name =
        "Godot_v" + cfg.version + "_export_templates.tpz";
    const std::string archive_url =
        "https://github.com/godotengine/godot/releases/download/" + cfg.version + "/" + archive_name;
    const fs::path archive_path = cfg.cache / archive_name;
    if (!download_file(archive_url, archive_path, err)) {
        return false;
    }

    const fs::path extract_dir = cfg.cache / "_templates_extract";
    if (!unzip_file(archive_path, extract_dir, err)) {
        return false;
    }

    const fs::path templates_dir = find_templates_dir(extract_dir);
    if (templates_dir.empty()) {
        err = "Could not locate 'templates/' in export template archive.";
        return false;
    }

    if (!copy_directory_contents(templates_dir, install_dir, err)) {
        return false;
    }

    fs::create_directories(ready_sentinel.parent_path(), ec);
    std::ofstream ready(ready_sentinel, std::ios::binary);
    if (!ready.is_open()) {
        err = "Failed to write Godot cache sentinel: " + ready_sentinel.string();
        return false;
    }
    ready << "ready\n";
    return true;
}

} // namespace godot_sdk

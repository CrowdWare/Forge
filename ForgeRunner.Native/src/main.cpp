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

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

using NativeSessionCreateFn = int (*)(std::int64_t*, char*, int);
using NativeSessionLoadFn = int (*)(std::int64_t, const char*, char*, int);
using NativeSessionInvokeFn = int (*)(std::int64_t, const char*, const char*, const char*, std::int64_t*, char*, int);
using NativeSessionDisposeFn = int (*)(std::int64_t, char*, int);
using NativeSetUiCallbacksFn = int (*) (
    int (*)(const char*, const char*, char*, int, char*, int),
    int (*)(const char*, const char*, const char*, char*, int),
    int (*)(const char*, const char*, const char*, char*, int, char*, int),
    char*,
    int);

namespace {

struct RootInfo {
    std::string name;
    std::optional<std::string> id;
    std::optional<int> duration_ms;
    std::optional<std::string> load_on_ready;
};

struct ManifestFileEntry {
    fs::path path;
    std::int64_t size_bytes = -1;
    std::string remote_url;
};

struct ManifestInfo {
    fs::path manifest_path;
    fs::path entry_path;
    fs::path entry_relative;
    std::vector<ManifestFileEntry> files;
};

struct SyncStats {
    int total_files = 0;
    int completed_files = 0;
    std::int64_t planned_bytes = 0;
    std::int64_t downloaded_bytes = 0;
};

std::string current_timestamp_utc() {
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const std::time_t tt = clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif

    char buffer[32]{};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buffer);
}

std::string trim(std::string text) {
    auto is_ws = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!text.empty() && is_ws(static_cast<unsigned char>(text.front()))) {
        text.erase(text.begin());
    }
    while (!text.empty() && is_ws(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
    return text;
}

std::string to_lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

bool iequals(std::string a, std::string b) {
    return to_lower(std::move(a)) == to_lower(std::move(b));
}

void print_usage() {
    std::cout << "ForgeRunner.Native\n"
              << "Usage:\n"
              << "  forge-runner-native [--url <value>] [--verbose] [--help]\n";
}

std::optional<std::string> read_text_file(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}

bool write_text_file(const fs::path& path, const std::string& value) {
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }
    output << value;
    return static_cast<bool>(output);
}

std::string shared_library_extension() {
#if defined(_WIN32)
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

fs::path last_url_state_file() {
    const char* home = std::getenv("HOME");
#if defined(_WIN32)
    if (home == nullptr) {
        home = std::getenv("USERPROFILE");
    }
#endif
    if (home == nullptr || std::string(home).empty()) {
        return fs::current_path() / ".forge_runner_native_last_url";
    }
    return fs::path(home) / ".forge_runner_native_last_url";
}

std::optional<std::string> restore_last_url() {
    const auto path = last_url_state_file();
    const auto content = read_text_file(path);
    if (!content.has_value()) {
        return std::nullopt;
    }
    const auto value = trim(*content);
    if (value.empty()) {
        return std::nullopt;
    }
    return value;
}

void persist_last_url(const std::string& url) {
    if (url.empty()) {
        return;
    }
    (void)write_text_file(last_url_state_file(), url + "\n");
}

std::optional<fs::path> resolve_file_url(const std::string& url) {
    constexpr const char* kPrefix = "file://";
    if (url.rfind(kPrefix, 0) != 0) {
        return std::nullopt;
    }
    std::string raw = url.substr(std::char_traits<char>::length(kPrefix));
    if (raw.empty()) {
        return std::nullopt;
    }
#if defined(_WIN32)
    if (!raw.empty() && raw[0] == '/') {
        raw.erase(raw.begin());
    }
#endif
    return fs::path(raw);
}

bool starts_with(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

bool is_http_url(const std::string& url) {
    return starts_with(url, "http://") || starts_with(url, "https://");
}

bool is_manifest_url(const std::string& url) {
    return url.find("manifest.sml") != std::string::npos;
}

std::string dirname_url(const std::string& url) {
    const auto pos = url.find_last_of('/');
    if (pos == std::string::npos) {
        return url;
    }
    return url.substr(0, pos + 1);
}

std::string trim_leading_slashes(std::string value) {
    while (!value.empty() && (value.front() == '/' || value.front() == '.')) {
        if (value.front() == '.') {
            value.erase(value.begin());
            continue;
        }
        value.erase(value.begin());
    }
    return value;
}

std::string url_join(const std::string& base_dir_url, const fs::path& rel_path) {
    const auto rel = trim_leading_slashes(rel_path.generic_string());
    return base_dir_url + rel;
}

std::string shell_single_quote(const std::string& raw) {
    std::string out = "'";
    for (char ch : raw) {
        if (ch == '\'') {
            out += "'\"'\"'";
            continue;
        }
        out.push_back(ch);
    }
    out.push_back('\'');
    return out;
}

bool exec_capture_stdout(const std::string& cmd, std::string& out) {
#if defined(_WIN32)
    (void)cmd;
    (void)out;
    return false;
#else
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe == nullptr) {
        return false;
    }
    char buffer[4096];
    while (fgets(buffer, static_cast<int>(sizeof(buffer)), pipe) != nullptr) {
        out.append(buffer);
    }
    const int rc = pclose(pipe);
    return rc == 0;
#endif
}

bool fetch_url_text(const std::string& url, std::string& out) {
#if defined(_WIN32)
    (void)url;
    (void)out;
    return false;
#else
    const std::string cmd = "curl -fsSL --retry 1 --connect-timeout 10 --max-time 60 "
        + shell_single_quote(url);
    return exec_capture_stdout(cmd, out);
#endif
}

bool fetch_url_to_file(const std::string& url, const fs::path& destination) {
#if defined(_WIN32)
    (void)url;
    (void)destination;
    return false;
#else
    std::error_code ec;
    fs::create_directories(destination.parent_path(), ec);
    const std::string cmd = "curl -fsSL --retry 1 --connect-timeout 10 --max-time 120 -o "
        + shell_single_quote(destination.string()) + " "
        + shell_single_quote(url);
    const int rc = std::system(cmd.c_str());
    return rc == 0;
#endif
}

fs::path remote_cache_root_for_url(const std::string& manifest_url) {
    const char* home = std::getenv("HOME");
    fs::path base = (home == nullptr || std::string(home).empty())
        ? (fs::current_path() / ".forge_runner_native_cache")
        : (fs::path(home) / ".forge_runner_native_cache");
    const auto h = static_cast<unsigned long long>(std::hash<std::string>{}(manifest_url));
    std::ostringstream suffix;
    suffix << std::hex << h;
    return base / suffix.str();
}

std::string first_string_argument(const std::string& args_json) {
    const auto first_quote = args_json.find('"');
    if (first_quote == std::string::npos) {
        return args_json;
    }
    const auto second_quote = args_json.find('"', first_quote + 1);
    if (second_quote == std::string::npos || second_quote <= first_quote + 1) {
        return args_json;
    }
    return args_json.substr(first_quote + 1, second_quote - first_quote - 1);
}

std::string json_escape(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);
    for (char ch : value) {
        switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
        }
    }
    return out;
}

std::vector<std::string> parse_string_args_json(const std::string& args_json) {
    std::vector<std::string> out;
    bool in_string = false;
    bool escaping = false;
    std::string current;

    for (std::size_t i = 0; i < args_json.size(); ++i) {
        const char ch = args_json[i];
        if (!in_string) {
            if (ch == '"') {
                in_string = true;
                current.clear();
            }
            continue;
        }

        if (escaping) {
            switch (ch) {
                case 'n': current.push_back('\n'); break;
                case 'r': current.push_back('\r'); break;
                case 't': current.push_back('\t'); break;
                default: current.push_back(ch); break;
            }
            escaping = false;
            continue;
        }
        if (ch == '\\') {
            escaping = true;
            continue;
        }
        if (ch == '"') {
            out.push_back(current);
            in_string = false;
            continue;
        }
        current.push_back(ch);
    }

    return out;
}

struct UiState {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> props_by_object;
    std::int64_t next_tree_handle = 1;
    std::string last_sml_error;
    std::string last_project_path;
};

UiState& ui_state() {
    static UiState state;
    return state;
}

std::optional<std::size_t> find_matching_brace(const std::string& source, std::size_t open_index) {
    if (open_index >= source.size() || source[open_index] != '{') {
        return std::nullopt;
    }

    int depth = 0;
    bool in_string = false;
    bool escaping = false;
    for (std::size_t i = open_index; i < source.size(); ++i) {
        const char ch = source[i];

        if (in_string) {
            if (escaping) {
                escaping = false;
                continue;
            }
            if (ch == '\\') {
                escaping = true;
                continue;
            }
            if (ch == '"') {
                in_string = false;
            }
            continue;
        }

        if (ch == '"') {
            in_string = true;
            continue;
        }
        if (ch == '{') {
            depth++;
            continue;
        }
        if (ch == '}') {
            depth--;
            if (depth == 0) {
                return i;
            }
        }
    }

    return std::nullopt;
}

std::optional<std::size_t> find_root_open_brace(const std::string& source) {
    bool in_string = false;
    bool escaping = false;
    for (std::size_t i = 0; i < source.size(); ++i) {
        const char ch = source[i];
        if (in_string) {
            if (escaping) {
                escaping = false;
                continue;
            }
            if (ch == '\\') {
                escaping = true;
                continue;
            }
            if (ch == '"') {
                in_string = false;
            }
            continue;
        }
        if (ch == '"') {
            in_string = true;
            continue;
        }
        if (ch == '{') {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<std::string> find_root_element_name(const std::string& sml_source) {
    std::string token;
    bool seen_non_space = false;
    for (char ch : sml_source) {
        if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
            if (!token.empty()) {
                return token;
            }
            continue;
        }
        seen_non_space = true;
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
            token.push_back(ch);
            continue;
        }
        if (!token.empty()) {
            return token;
        }
        if (seen_non_space) {
            break;
        }
    }
    if (!token.empty()) {
        return token;
    }
    return std::nullopt;
}

std::optional<std::string> find_top_level_string_property(const std::string& body, const std::string& key) {
    int depth = 0;
    bool in_string = false;
    bool escaping = false;

    for (std::size_t i = 0; i < body.size(); ++i) {
        const char ch = body[i];

        if (in_string) {
            if (escaping) {
                escaping = false;
                continue;
            }
            if (ch == '\\') {
                escaping = true;
                continue;
            }
            if (ch == '"') {
                in_string = false;
            }
            continue;
        }

        if (ch == '"') {
            in_string = true;
            continue;
        }
        if (ch == '{') {
            depth++;
            continue;
        }
        if (ch == '}') {
            depth = std::max(0, depth - 1);
            continue;
        }

        if (depth != 0) {
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(ch)) == 0 && ch != '_') {
            continue;
        }

        std::size_t j = i;
        std::string ident;
        while (j < body.size()) {
            const char cj = body[j];
            if (std::isalnum(static_cast<unsigned char>(cj)) != 0 || cj == '_') {
                ident.push_back(cj);
                j++;
                continue;
            }
            break;
        }

        if (ident != key) {
            i = (j > i) ? (j - 1) : i;
            continue;
        }

        while (j < body.size() && std::isspace(static_cast<unsigned char>(body[j])) != 0) {
            j++;
        }
        if (j >= body.size() || body[j] != ':') {
            i = (j > i) ? (j - 1) : i;
            continue;
        }
        j++;
        while (j < body.size() && std::isspace(static_cast<unsigned char>(body[j])) != 0) {
            j++;
        }
        if (j >= body.size() || body[j] != '"') {
            return std::nullopt;
        }
        j++;

        std::string value;
        bool local_escape = false;
        while (j < body.size()) {
            const char cj = body[j];
            if (local_escape) {
                value.push_back(cj);
                local_escape = false;
                j++;
                continue;
            }
            if (cj == '\\') {
                local_escape = true;
                j++;
                continue;
            }
            if (cj == '"') {
                return value;
            }
            value.push_back(cj);
            j++;
        }
        return std::nullopt;
    }

    return std::nullopt;
}

std::optional<int> find_top_level_int_property(const std::string& body, const std::string& key) {
    int depth = 0;
    bool in_string = false;
    bool escaping = false;

    for (std::size_t i = 0; i < body.size(); ++i) {
        const char ch = body[i];

        if (in_string) {
            if (escaping) {
                escaping = false;
                continue;
            }
            if (ch == '\\') {
                escaping = true;
                continue;
            }
            if (ch == '"') {
                in_string = false;
            }
            continue;
        }

        if (ch == '"') {
            in_string = true;
            continue;
        }
        if (ch == '{') {
            depth++;
            continue;
        }
        if (ch == '}') {
            depth = std::max(0, depth - 1);
            continue;
        }

        if (depth != 0) {
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(ch)) == 0 && ch != '_') {
            continue;
        }

        std::size_t j = i;
        std::string ident;
        while (j < body.size()) {
            const char cj = body[j];
            if (std::isalnum(static_cast<unsigned char>(cj)) != 0 || cj == '_') {
                ident.push_back(cj);
                j++;
                continue;
            }
            break;
        }

        if (ident != key) {
            i = (j > i) ? (j - 1) : i;
            continue;
        }

        while (j < body.size() && std::isspace(static_cast<unsigned char>(body[j])) != 0) {
            j++;
        }
        if (j >= body.size() || body[j] != ':') {
            i = (j > i) ? (j - 1) : i;
            continue;
        }
        j++;
        while (j < body.size() && std::isspace(static_cast<unsigned char>(body[j])) != 0) {
            j++;
        }

        std::string raw;
        if (j < body.size() && (body[j] == '+' || body[j] == '-')) {
            raw.push_back(body[j]);
            j++;
        }
        while (j < body.size() && std::isdigit(static_cast<unsigned char>(body[j])) != 0) {
            raw.push_back(body[j]);
            j++;
        }
        if (raw.empty() || raw == "+" || raw == "-") {
            return std::nullopt;
        }

        try {
            return std::stoi(raw);
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

std::optional<std::string> find_top_level_identifier_property(const std::string& body, const std::string& key) {
    int depth = 0;
    bool in_string = false;
    bool escaping = false;

    for (std::size_t i = 0; i < body.size(); ++i) {
        const char ch = body[i];

        if (in_string) {
            if (escaping) {
                escaping = false;
                continue;
            }
            if (ch == '\\') {
                escaping = true;
                continue;
            }
            if (ch == '"') {
                in_string = false;
            }
            continue;
        }

        if (ch == '"') {
            in_string = true;
            continue;
        }
        if (ch == '{') {
            depth++;
            continue;
        }
        if (ch == '}') {
            depth = std::max(0, depth - 1);
            continue;
        }

        if (depth != 0) {
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(ch)) == 0 && ch != '_') {
            continue;
        }

        std::size_t j = i;
        std::string ident;
        while (j < body.size()) {
            const char cj = body[j];
            if (std::isalnum(static_cast<unsigned char>(cj)) != 0 || cj == '_') {
                ident.push_back(cj);
                j++;
                continue;
            }
            break;
        }

        if (ident != key) {
            i = (j > i) ? (j - 1) : i;
            continue;
        }

        while (j < body.size() && std::isspace(static_cast<unsigned char>(body[j])) != 0) {
            j++;
        }
        if (j >= body.size() || body[j] != ':') {
            i = (j > i) ? (j - 1) : i;
            continue;
        }
        j++;
        while (j < body.size() && std::isspace(static_cast<unsigned char>(body[j])) != 0) {
            j++;
        }

        std::string value;
        while (j < body.size()) {
            const char cj = body[j];
            if (std::isalnum(static_cast<unsigned char>(cj)) != 0 || cj == '_' || cj == '-') {
                value.push_back(cj);
                j++;
                continue;
            }
            break;
        }
        if (!value.empty()) {
            return value;
        }
        return std::nullopt;
    }

    return std::nullopt;
}

fs::path normalize_relative_path(const std::string& value) {
    fs::path rel = fs::path(trim(value)).lexically_normal();
    if (rel.empty()) {
        return fs::path("app.sml");
    }
    while (!rel.empty() && rel.native().rfind("./", 0) == 0) {
        rel = rel.lexically_relative(".");
    }
    return rel;
}

RootInfo parse_root_info(const std::string& source) {
    RootInfo result;
    const auto root_name = find_root_element_name(source);
    if (!root_name.has_value()) {
        return result;
    }
    result.name = *root_name;

    const auto open_brace = find_root_open_brace(source);
    if (!open_brace.has_value()) {
        return result;
    }

    const auto close_brace = find_matching_brace(source, *open_brace);
    if (!close_brace.has_value() || *close_brace <= *open_brace) {
        return result;
    }

    const std::string body = source.substr(*open_brace + 1, *close_brace - *open_brace - 1);
    result.id = find_top_level_string_property(body, "id");
    if (!result.id.has_value()) {
        result.id = find_top_level_identifier_property(body, "id");
    }
    result.duration_ms = find_top_level_int_property(body, "duration");
    result.load_on_ready = find_top_level_string_property(body, "loadOnReady");
    return result;
}

std::optional<ManifestInfo> parse_manifest(const fs::path& manifest_path, const std::string& source) {
    const auto root = parse_root_info(source);
    if (!iequals(root.name, "Manifest")) {
        return std::nullopt;
    }

    const auto open_brace = find_root_open_brace(source);
    if (!open_brace.has_value()) {
        return std::nullopt;
    }
    const auto close_brace = find_matching_brace(source, *open_brace);
    if (!close_brace.has_value() || *close_brace <= *open_brace) {
        return std::nullopt;
    }

    const std::string body = source.substr(*open_brace + 1, *close_brace - *open_brace - 1);
    const auto entry = find_top_level_string_property(body, "entry");

    ManifestInfo info;
    info.manifest_path = manifest_path;
    info.entry_relative = normalize_relative_path(entry.value_or("app.sml"));
    info.entry_path = (manifest_path.parent_path() / info.entry_relative).lexically_normal();

    std::size_t pos = 0;
    while (true) {
        pos = source.find("File", pos);
        if (pos == std::string::npos) {
            break;
        }

        const char prev = pos == 0 ? '\0' : source[pos - 1];
        const char next = (pos + 4) < source.size() ? source[pos + 4] : '\0';
        const bool prev_ok = prev == '\0' || !(std::isalnum(static_cast<unsigned char>(prev)) != 0 || prev == '_');
        const bool next_ok = next == '\0' || !(std::isalnum(static_cast<unsigned char>(next)) != 0 || next == '_');
        if (!prev_ok || !next_ok) {
            pos += 4;
            continue;
        }

        const auto file_open = source.find('{', pos + 4);
        if (file_open == std::string::npos) {
            break;
        }
        const auto file_close = find_matching_brace(source, file_open);
        if (!file_close.has_value() || *file_close <= file_open) {
            break;
        }

        const std::string file_body = source.substr(file_open + 1, *file_close - file_open - 1);
        const auto path_value = find_top_level_string_property(file_body, "path");
        if (path_value.has_value() && !trim(*path_value).empty()) {
            ManifestFileEntry file;
            file.path = normalize_relative_path(*path_value);
            const auto size_value = find_top_level_int_property(file_body, "size");
            if (size_value.has_value()) {
                file.size_bytes = static_cast<std::int64_t>(*size_value);
            }
            info.files.push_back(file);
        }

        pos = *file_close + 1;
    }

    bool has_entry = false;
    for (const auto& f : info.files) {
        if (f.path == info.entry_relative) {
            has_entry = true;
            break;
        }
    }
    if (!has_entry) {
        ManifestFileEntry entry_file;
        entry_file.path = info.entry_relative;
        info.files.push_back(entry_file);
    }

    return info;
}

fs::path resolve_reference_path(const fs::path& current_doc, const std::string& ref) {
    fs::path candidate = fs::path(trim(ref));
    if (candidate.empty()) {
        return current_doc;
    }
    if (candidate.is_absolute()) {
        return candidate.lexically_normal();
    }
    return (current_doc.parent_path() / candidate).lexically_normal();
}

int ui_get_stub(const char* object_id, const char* property, char* out_json, int out_json_capacity, char*, int) {
    const std::string object = object_id != nullptr ? object_id : "";
    const std::string prop = property != nullptr ? property : "";
    if (out_json == nullptr || out_json_capacity <= 0) {
        return 0;
    }

    if (prop == "__exists") {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "1");
        return 0;
    }

    auto& state = ui_state();
    const auto object_it = state.props_by_object.find(object);
    if (object_it == state.props_by_object.end()) {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
        return 0;
    }
    const auto prop_it = object_it->second.find(prop);
    if (prop_it == object_it->second.end()) {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
        return 0;
    }

    std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", prop_it->second.c_str());
    return 0;
}

int ui_set_stub(const char* object_id, const char* property, const char* value_json, char*, int) {
    const std::string object = object_id != nullptr ? object_id : "";
    const std::string prop = property != nullptr ? property : "";
    const std::string value = value_json != nullptr ? value_json : "null";
    ui_state().props_by_object[object][prop] = value;
    return 0;
}

int ui_invoke_bridge(
    const char* object_id,
    const char* method,
    const char* args_json,
    char* out_json,
    int out_json_capacity,
    char*,
    int) {
    const std::string object = object_id != nullptr ? object_id : "";
    const std::string method_name = method != nullptr ? method : "";
    const std::string args = args_json != nullptr ? args_json : "[]";
    const std::string first_arg = first_string_argument(args);
    const auto string_args = parse_string_args_json(args);

    if (object == "__log__") {
        const char* color = "";
        const char* reset = "\033[0m";
        if (method_name == "success") {
            color = "\033[32m";
        } else if (method_name == "warning" || method_name == "warn") {
            color = "\033[33m";
        } else if (method_name == "error") {
            color = "\033[31m";
        } else if (method_name == "debug") {
            color = "\033[90m";
        }

        if (color[0] != '\0') {
            std::cout << color << first_arg << reset << "\n";
        } else {
            std::cout << first_arg << "\n";
        }
        if (out_json != nullptr && out_json_capacity > 0) {
            std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
        }
        return 0;
    }

    if (object == "__fs__") {
        if (method_name == "exists" && !string_args.empty()) {
            const bool exists = fs::exists(fs::path(string_args[0]));
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%d", exists ? 1 : 0);
            }
            return 0;
        }
        if (method_name == "readText" && !string_args.empty()) {
            const auto text = read_text_file(fs::path(string_args[0])).value_or("");
            if (out_json != nullptr && out_json_capacity > 0) {
                const auto json = "\"" + json_escape(text) + "\"";
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", json.c_str());
            }
            return 0;
        }
        if (method_name == "writeText" && string_args.size() >= 2) {
            const bool ok = write_text_file(fs::path(string_args[0]), string_args[1]);
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%d", ok ? 1 : 0);
            }
            return 0;
        }
        if (method_name == "list" && !string_args.empty()) {
            std::ostringstream json;
            json << "[";
            std::error_code ec;
            bool first = true;
            for (const auto& entry : fs::directory_iterator(fs::path(string_args[0]), ec)) {
                if (ec) {
                    break;
                }
                if (!first) {
                    json << ",";
                }
                first = false;
                const auto p = entry.path();
                json << "{"
                     << "\"Name\":\"" << json_escape(p.filename().string()) << "\","
                     << "\"Path\":\"" << json_escape(p.string()) << "\","
                     << "\"IsDirectory\":" << (entry.is_directory(ec) ? "1" : "0")
                     << "}";
            }
            json << "]";
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", json.str().c_str());
            }
            return 0;
        }
        if (out_json != nullptr && out_json_capacity > 0) {
            std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
        }
        return 0;
    }

    if (object == "__os__") {
        if (method_name == "fileExists" && !string_args.empty()) {
            const bool exists = fs::exists(fs::path(string_args[0]));
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%d", exists ? 1 : 0);
            }
            return 0;
        }
        if (method_name == "readFile" && !string_args.empty()) {
            const auto text = read_text_file(fs::path(string_args[0])).value_or("");
            if (out_json != nullptr && out_json_capacity > 0) {
                const auto json = "\"" + json_escape(text) + "\"";
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", json.c_str());
            }
            return 0;
        }
        if (method_name == "writeFile" && string_args.size() >= 2) {
            const bool ok = write_text_file(fs::path(string_args[0]), string_args[1]);
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%d", ok ? 1 : 0);
            }
            return 0;
        }
        if (method_name == "resolvePath" && !string_args.empty()) {
            const auto resolved = fs::absolute(fs::path(string_args[0])).lexically_normal().string();
            if (out_json != nullptr && out_json_capacity > 0) {
                const auto json = "\"" + json_escape(resolved) + "\"";
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", json.c_str());
            }
            return 0;
        }
        if (method_name == "toResPath" && !string_args.empty()) {
            if (out_json != nullptr && out_json_capacity > 0) {
                const auto json = "\"" + json_escape(string_args[0]) + "\"";
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", json.c_str());
            }
            return 0;
        }
        if (method_name == "callStatic") {
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
            }
            return 0;
        }
        if (method_name == "loadPromptConfig") {
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "{}");
            }
            return 0;
        }
        if (method_name == "savePromptConfig") {
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "1");
            }
            return 0;
        }
        if (out_json != nullptr && out_json_capacity > 0) {
            std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
        }
        return 0;
    }

    if (object == "__ui__") {
        auto& state = ui_state();
        if (method_name == "setLastProjectPath" && !string_args.empty()) {
            state.last_project_path = string_args[0];
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
            }
            return 0;
        }
        if (method_name == "getLastProjectPath") {
            const auto json = "\"" + json_escape(state.last_project_path) + "\"";
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", json.c_str());
            }
            return 0;
        }
        if (method_name == "hasLastProject") {
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%d", state.last_project_path.empty() ? 0 : 1);
            }
            return 0;
        }
        if (method_name == "validateSmlSyntax") {
            state.last_sml_error.clear();
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "1");
            }
            return 0;
        }
        if (method_name == "getLastSmlSyntaxError") {
            const auto json = "\"" + json_escape(state.last_sml_error) + "\"";
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", json.c_str());
            }
            return 0;
        }
        if (method_name == "renderSmlPreview" || method_name == "copyTemplateFilesToProject" || method_name == "showMessage"
            || method_name == "openSaveFileDialog" || method_name == "openFileDialog" || method_name == "setTimeout") {
            if (out_json != nullptr && out_json_capacity > 0) {
                std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "1");
            }
            return 0;
        }
        if (out_json != nullptr && out_json_capacity > 0) {
            std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
        }
        return 0;
    }

    if ((object == "ui" || object == "__ui__") && method_name == "getObject") {
        if (out_json != nullptr && out_json_capacity > 0) {
            std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "\"%s\"", first_arg.c_str());
        }
        return 0;
    }

    {
        auto& object_props = ui_state().props_by_object[object];
        if (method_name == "SetPath" && !string_args.empty()) {
            object_props["path"] = "\"" + json_escape(string_args[0]) + "\"";
            if (out_json != nullptr && out_json_capacity > 0) std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
            return 0;
        }
        if (method_name == "GetPath") {
            const auto it = object_props.find("path");
            const std::string value = it != object_props.end() ? it->second : "\"\"";
            if (out_json != nullptr && out_json_capacity > 0) std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", value.c_str());
            return 0;
        }
        if (method_name == "SetText" && !string_args.empty()) {
            object_props["text"] = "\"" + json_escape(string_args[0]) + "\"";
            if (out_json != nullptr && out_json_capacity > 0) std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
            return 0;
        }
        if (method_name == "GetText") {
            const auto it = object_props.find("text");
            const std::string value = it != object_props.end() ? it->second : "\"\"";
            if (out_json != nullptr && out_json_capacity > 0) std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", value.c_str());
            return 0;
        }
        if (method_name == "Clear") {
            if (out_json != nullptr && out_json_capacity > 0) std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
            return 0;
        }
        if (method_name == "CreateRoot" || method_name == "CreateChild") {
            const auto handle = ui_state().next_tree_handle++;
            if (out_json != nullptr && out_json_capacity > 0) std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%lld", static_cast<long long>(handle));
            return 0;
        }
        if (method_name == "GetSelectedPath") {
            const auto it = object_props.find("selectedPath");
            const std::string value = it != object_props.end() ? it->second : "\"\"";
            if (out_json != nullptr && out_json_capacity > 0) std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", value.c_str());
            return 0;
        }
    }

    if (out_json != nullptr && out_json_capacity > 0) {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
    }
    return 0;
}

void* load_library(const fs::path& file) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(LoadLibraryA(file.string().c_str()));
#else
    return dlopen(file.string().c_str(), RTLD_NOW);
#endif
}

void* load_symbol(void* lib, const char* name) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(lib), name));
#else
    return dlsym(lib, name);
#endif
}

bool invoke_event_or_ignore_missing(
    NativeSessionInvokeFn invoke_fn,
    std::int64_t session,
    const std::string& target,
    const std::string& event_name) {
    char error[1024] = {0};
    std::int64_t out = 0;
    const int rc = invoke_fn(session, target.c_str(), event_name.c_str(), "[]", &out, error, static_cast<int>(sizeof(error)));
    if (rc == 0) {
        return true;
    }
    const std::string err = error;
    if (err.find("No SMS event handler found") != std::string::npos) {
        return false;
    }
    std::cerr << "[ForgeRunner.Native] event dispatch failed for '" << target << "." << event_name << "': " << err << "\n";
    return false;
}

class SmsRuntime {
public:
    explicit SmsRuntime(const fs::path& repo_root)
        : repo_root_(repo_root) {}

    bool dispatch_ready(const fs::path& sms_path, const std::string& sms_source, const RootInfo& root_info) {
        if (!ensure_loaded()) {
            return false;
        }

        std::int64_t session = 0;
        char error[1024] = {0};
        if (create_fn_(&session, error, static_cast<int>(sizeof(error))) != 0) {
            std::cerr << "[ForgeRunner.Native] session create failed: " << error << "\n";
            return false;
        }

        if (load_fn_(session, sms_source.c_str(), error, static_cast<int>(sizeof(error))) != 0) {
            std::cerr << "[ForgeRunner.Native] session load failed for '" << sms_path.string() << "': " << error << "\n";
            (void)dispose_fn_(session, nullptr, 0);
            return false;
        }

        invoke_event_or_ignore_missing(invoke_fn_, session, "__runtime__", "ready");
        if (!root_info.name.empty()) {
            invoke_event_or_ignore_missing(invoke_fn_, session, to_lower(root_info.name), "ready");
        }
        if (root_info.id.has_value() && !root_info.id->empty()) {
            invoke_event_or_ignore_missing(invoke_fn_, session, *root_info.id, "ready");
        }

        (void)dispose_fn_(session, nullptr, 0);
        return true;
    }

private:
    bool ensure_loaded() {
        if (loaded_) {
            return true;
        }

        const fs::path sms_lib_dir = std::getenv("SMS_NATIVE_LIB_DIR") != nullptr
            ? fs::path(std::getenv("SMS_NATIVE_LIB_DIR"))
            : (repo_root_ / "ForgeRunner.Native" / "build");
        const fs::path sms_lib_path = sms_lib_dir / ("libsms_native" + shared_library_extension());

        lib_handle_ = load_library(sms_lib_path);
        if (lib_handle_ == nullptr) {
            std::cerr << "[ForgeRunner.Native] failed to load sms native library: " << sms_lib_path.string() << "\n";
            return false;
        }

        create_fn_ = reinterpret_cast<NativeSessionCreateFn>(load_symbol(lib_handle_, "sms_native_session_create"));
        load_fn_ = reinterpret_cast<NativeSessionLoadFn>(load_symbol(lib_handle_, "sms_native_session_load"));
        invoke_fn_ = reinterpret_cast<NativeSessionInvokeFn>(load_symbol(lib_handle_, "sms_native_session_invoke"));
        dispose_fn_ = reinterpret_cast<NativeSessionDisposeFn>(load_symbol(lib_handle_, "sms_native_session_dispose"));
        set_ui_callbacks_fn_ = reinterpret_cast<NativeSetUiCallbacksFn>(load_symbol(lib_handle_, "sms_native_set_ui_callbacks"));

        if (create_fn_ == nullptr || load_fn_ == nullptr || invoke_fn_ == nullptr || dispose_fn_ == nullptr || set_ui_callbacks_fn_ == nullptr) {
            std::cerr << "[ForgeRunner.Native] missing required sms native symbol(s)\n";
            return false;
        }

        char cb_error[1024] = {0};
        (void)set_ui_callbacks_fn_(&ui_get_stub, &ui_set_stub, &ui_invoke_bridge, cb_error, static_cast<int>(sizeof(cb_error)));

        loaded_ = true;
        return true;
    }

    fs::path repo_root_;
    bool loaded_ = false;
    void* lib_handle_ = nullptr;

    NativeSessionCreateFn create_fn_ = nullptr;
    NativeSessionLoadFn load_fn_ = nullptr;
    NativeSessionInvokeFn invoke_fn_ = nullptr;
    NativeSessionDisposeFn dispose_fn_ = nullptr;
    NativeSetUiCallbacksFn set_ui_callbacks_fn_ = nullptr;
};

std::vector<ManifestFileEntry> filter_manifest_files(
    const std::vector<ManifestFileEntry>& files,
    const fs::path& target,
    bool include_target) {
    std::vector<ManifestFileEntry> out;
    for (const auto& file : files) {
        const bool is_target = file.path == target;
        if ((include_target && is_target) || (!include_target && !is_target)) {
            out.push_back(file);
        }
    }
    return out;
}

std::string progress_route_name(const std::string& root_name) {
    const auto lower = to_lower(root_name);
    if (lower == "terminal") {
        return "terminal";
    }
    if (lower == "window") {
        return "window-overlay";
    }
    if (lower == "splashscreen") {
        return "splash-embedded";
    }
    return "default";
}

void log_progress_line(const std::string& route, const SyncStats& stats, const std::string& current_path) {
    std::cout << "[ForgeRunner.Native][Startup][" << route << "] "
              << stats.completed_files << "/" << stats.total_files
              << " bytes=" << stats.downloaded_bytes << "/" << stats.planned_bytes;
    if (!current_path.empty()) {
        std::cout << " current=" << current_path;
    }
    std::cout << "\n";
}

void sync_manifest_files(
    const fs::path& base_dir,
    const std::vector<ManifestFileEntry>& files,
    const std::string& route,
    bool verbose,
    bool remote_mode) {
    if (files.empty()) {
        return;
    }

    SyncStats stats;
    stats.total_files = static_cast<int>(files.size());

    for (const auto& file : files) {
        if (file.size_bytes > 0) {
            stats.planned_bytes += file.size_bytes;
        }
    }

    for (const auto& file : files) {
        const fs::path absolute = (base_dir / file.path).lexically_normal();
        std::int64_t observed_bytes = 0;

        std::error_code ec;
        if (remote_mode && !file.remote_url.empty() && !fs::exists(absolute, ec)) {
            if (!fetch_url_to_file(file.remote_url, absolute)) {
                std::cerr << "[ForgeRunner.Native][Startup] failed downloading: " << file.remote_url << "\n";
            }
        }

        if (fs::exists(absolute, ec)) {
            if (file.size_bytes > 0) {
                observed_bytes = file.size_bytes;
            } else {
                observed_bytes = static_cast<std::int64_t>(fs::file_size(absolute, ec));
            }
        } else {
            std::cerr << "[ForgeRunner.Native][Startup] missing manifest file: " << absolute.string() << "\n";
        }

        if (observed_bytes > 0) {
            stats.downloaded_bytes += observed_bytes;
            if (stats.planned_bytes <= 0) {
                stats.planned_bytes += observed_bytes;
            }
        }

        stats.completed_files++;
        log_progress_line(route, stats, file.path.string());

        if (verbose) {
            std::cout << "[ForgeRunner.Native][Startup] synced '" << file.path.string() << "'\n";
        }
    }
}

bool run_document(
    const fs::path& sml_path,
    const std::string& sml_source,
    SmsRuntime& sms_runtime,
    const RootInfo& root_info,
    bool verbose) {
    const fs::path sms_path = sml_path.parent_path() / (sml_path.stem().string() + ".sms");
    const auto sms_source = read_text_file(sms_path);
    if (!sms_source.has_value()) {
        std::cout << "[ForgeRunner.Native] no companion SMS for '" << sml_path.filename().string() << "' (expected '"
                  << sms_path.filename().string() << "'). Continuing SML-only.\n";
        return true;
    }

    if (verbose) {
        std::cout << "[ForgeRunner.Native] loading SMS: " << sms_path.string() << "\n";
    }

    return sms_runtime.dispatch_ready(sms_path, *sms_source, root_info);
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(argc > 0 ? static_cast<std::size_t>(argc) : 0U);
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i] != nullptr ? argv[i] : "");
    }

    bool verbose = false;
    std::string url;
    bool url_from_arg = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = args[static_cast<std::size_t>(i)];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--verbose") {
            verbose = true;
            continue;
        }
        if (arg == "--url" && i + 1 < argc) {
            url = args[static_cast<std::size_t>(++i)];
            url_from_arg = true;
            continue;
        }
        if (arg.rfind("--url=", 0) == 0) {
            url = arg.substr(6);
            url_from_arg = true;
            continue;
        }
    }

    std::cout << "[ForgeRunner.Native] bootstrap started at " << current_timestamp_utc() << "\n";
    std::cout << "[ForgeRunner.Native] executable main is active.\n";

    if (url.empty()) {
        const auto restored = restore_last_url();
        if (restored.has_value()) {
            url = *restored;
            std::cout << "[ForgeRunner.Native] restored last url=" << url << "\n";
        }
    }

    if (!url.empty()) {
        std::cout << "[ForgeRunner.Native] url=" << url << "\n";
        if (url_from_arg) {
            persist_last_url(url);
        }
    } else {
        std::cout << "[ForgeRunner.Native] url=<none>\n";
    }

    if (verbose) {
        std::cout << "[ForgeRunner.Native] verbose=true\n";
    }

    if (url.empty()) {
        return 0;
    }

    const auto start_path_opt = resolve_file_url(url);
    const bool remote_manifest_mode = is_http_url(url) && is_manifest_url(url);
    fs::path start_path;
    std::optional<std::string> start_source_opt;
    if (start_path_opt.has_value()) {
        start_path = (*start_path_opt).lexically_normal();
        start_source_opt = read_text_file(start_path);
        if (!start_source_opt.has_value()) {
            std::cerr << "[ForgeRunner.Native] failed to read startup file: " << start_path.string() << "\n";
            return 1;
        }
    } else if (remote_manifest_mode) {
        std::string manifest_text;
        if (!fetch_url_text(url, manifest_text)) {
            std::cerr << "[ForgeRunner.Native] failed to download remote manifest: " << url << "\n";
            return 1;
        }
        start_source_opt = manifest_text;
        start_path = remote_cache_root_for_url(url) / "manifest.sml";
        (void)write_text_file(start_path, manifest_text);
    } else {
        std::cerr << "[ForgeRunner.Native] unsupported url scheme: " << url << "\n";
        return 1;
    }

    ManifestInfo manifest_info;
    bool has_manifest = false;

    const RootInfo start_root = parse_root_info(*start_source_opt);
    if (iequals(start_root.name, "Manifest") || iequals(start_path.filename().string(), "manifest.sml")) {
        const auto parsed_manifest = parse_manifest(start_path, *start_source_opt);
        if (!parsed_manifest.has_value()) {
            std::cerr << "[ForgeRunner.Native] invalid manifest: " << start_path.string() << "\n";
            return 1;
        }
        manifest_info = *parsed_manifest;
        has_manifest = true;
        std::cout << "[ForgeRunner.Native][Startup] manifest loaded: " << manifest_info.manifest_path.string() << "\n";
    } else {
        const fs::path sibling_manifest = start_path.parent_path() / "manifest.sml";
        const auto sibling_manifest_source = read_text_file(sibling_manifest);
        if (sibling_manifest_source.has_value()) {
            const auto parsed_manifest = parse_manifest(sibling_manifest, *sibling_manifest_source);
            if (parsed_manifest.has_value()) {
                const auto expected_entry = normalize_relative_path(start_path.filename().string());
                if (parsed_manifest->entry_relative == expected_entry) {
                    manifest_info = *parsed_manifest;
                    has_manifest = true;
                    std::cout << "[ForgeRunner.Native][Startup] sibling manifest loaded: " << sibling_manifest.string() << "\n";
                }
            }
        }
    }

    if (remote_manifest_mode && has_manifest) {
        const std::string base_url = dirname_url(url);
        const fs::path cache_root = remote_cache_root_for_url(url);
        manifest_info.manifest_path = cache_root / "manifest.sml";
        manifest_info.entry_path = cache_root / manifest_info.entry_relative;
        for (auto& f : manifest_info.files) {
            f.remote_url = url_join(base_url, f.path);
        }
        (void)write_text_file(manifest_info.manifest_path, *start_source_opt);
    }

    // Local file:// execution is the pre-deploy path: resolve manifest entry,
    // but do not run startup download/sync behavior.
    const bool predeploy_local_mode = start_path_opt.has_value();
    if (has_manifest && predeploy_local_mode) {
        std::cout << "[ForgeRunner.Native][Startup] local predeploy mode: startup sync skipped.\n";
    }

    fs::path current_sml = has_manifest ? manifest_info.entry_path : start_path;
    fs::path manifest_entry_rel = has_manifest ? manifest_info.entry_relative : fs::path{};

    if (has_manifest && !predeploy_local_mode) {
        auto entry_only = filter_manifest_files(manifest_info.files, manifest_entry_rel, true);
        sync_manifest_files(manifest_info.manifest_path.parent_path(), entry_only, "entry", verbose, remote_manifest_mode);
    }

    const fs::path repo_root = fs::current_path();
    SmsRuntime sms_runtime(repo_root);

    bool synced_remaining = !has_manifest || predeploy_local_mode;
    int splash_hops = 0;
    constexpr int kMaxSplashHops = 4;

    while (true) {
        const auto current_sml_source_opt = read_text_file(current_sml);
        if (!current_sml_source_opt.has_value()) {
            std::cerr << "[ForgeRunner.Native] failed to read SML: " << current_sml.string() << "\n";
            return 1;
        }
        const std::string current_sml_source = *current_sml_source_opt;
        const RootInfo current_root = parse_root_info(current_sml_source);

        std::cout << "[ForgeRunner.Native] loaded SML: " << current_sml.string();
        if (!current_root.name.empty()) {
            std::cout << " root=" << current_root.name;
        }
        std::cout << "\n";

        if (!run_document(current_sml, current_sml_source, sms_runtime, current_root, verbose)) {
            return 1;
        }

        const std::string route = progress_route_name(current_root.name);

        bool splash_timer_applied = false;
        if (!synced_remaining && has_manifest) {
            auto remaining_files = filter_manifest_files(manifest_info.files, manifest_entry_rel, false);

            if (to_lower(current_root.name) == "splashscreen") {
                std::thread sync_thread([&]() {
                    sync_manifest_files(
                        manifest_info.manifest_path.parent_path(),
                        remaining_files,
                        route,
                        verbose,
                        remote_manifest_mode);
                });

                const int duration = std::max(0, current_root.duration_ms.value_or(0));
                if (duration > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
                    splash_timer_applied = true;
                }
                sync_thread.join();
            } else {
                sync_manifest_files(manifest_info.manifest_path.parent_path(), remaining_files, route, verbose, remote_manifest_mode);
            }

            synced_remaining = true;
        }

        if (!iequals(current_root.name, "SplashScreen")) {
            break;
        }

        const auto load_on_ready = current_root.load_on_ready;
        if (!load_on_ready.has_value() || trim(*load_on_ready).empty()) {
            break;
        }

        const int duration = std::max(0, current_root.duration_ms.value_or(0));
        if (duration > 0 && !splash_timer_applied) {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        }

        current_sml = resolve_reference_path(current_sml, *load_on_ready);
        std::cout << "[ForgeRunner.Native][Splash] loadOnReady -> " << current_sml.string() << "\n";

        splash_hops++;
        if (splash_hops > kMaxSplashHops) {
            std::cerr << "[ForgeRunner.Native] splash loadOnReady hop limit exceeded.\n";
            return 1;
        }
    }

    return 0;
}

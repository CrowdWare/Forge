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

#include "forge_asset_cache.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <godot_cpp/classes/hashing_context.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace fs = std::filesystem;
using namespace godot;

namespace forge {

// ---------------------------------------------------------------------------
// forge_cache_dir
// ---------------------------------------------------------------------------

std::string forge_cache_dir() {
#if defined(_WIN32)
    if (const char* v = std::getenv("LOCALAPPDATA"); v && v[0] != '\0')
        return std::string(v) + "\\ForgeRunner\\cache";
    if (const char* v = std::getenv("USERPROFILE"); v && v[0] != '\0')
        return std::string(v) + "\\AppData\\Local\\ForgeRunner\\cache";
    return "C:\\ForgeRunner\\cache";
#else
    if (const char* v = std::getenv("XDG_CACHE_HOME"); v && v[0] != '\0')
        return std::string(v) + "/forge-runner";
    if (const char* v = std::getenv("HOME"); v && v[0] != '\0')
        return std::string(v) + "/.cache/forge-runner";
    return "/tmp/forge-runner-cache";
#endif
}

// ---------------------------------------------------------------------------
// File-local helpers
// ---------------------------------------------------------------------------

static std::string current_utc_ts() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    char buf[32]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buf);
}

static std::string escape_sml(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else                out += static_cast<char>(c);
    }
    return out;
}

static std::string unescape_sml(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            ++i;
            if      (s[i] == '"')  out += '"';
            else if (s[i] == '\\') out += '\\';
            else if (s[i] == 'n')  out += '\n';
            else { out += '\\'; out += s[i]; }
        } else {
            out += s[i];
        }
    }
    return out;
}

// Extract quoted value from `key: "value"` line.
static std::string extract_quoted(const std::string& line, const std::string& key) {
    const std::string prefix = key + ": \"";
    const auto pos = line.find(prefix);
    if (pos == std::string::npos) return {};
    const auto start = pos + prefix.size();
    // Find unescaped closing quote
    size_t end = start;
    while (end < line.size()) {
        if (line[end] == '"' && (end == start || line[end - 1] != '\\')) break;
        ++end;
    }
    if (end >= line.size()) return {};
    return unescape_sml(line.substr(start, end - start));
}

// Extract file extension from a URL, e.g. ".sml" or ".png".
static std::string extract_url_ext(const std::string& url) {
    auto clean = url;
    const auto q = clean.find('?');
    if (q != std::string::npos) clean = clean.substr(0, q);
    const auto dot   = clean.rfind('.');
    const auto slash = clean.rfind('/');
    if (dot == std::string::npos) return {};
    if (slash != std::string::npos && dot < slash) return {};
    auto ext = clean.substr(dot);
    return (ext.size() <= 8) ? ext : std::string{};
}

// ---------------------------------------------------------------------------
// AssetCache - construction
// ---------------------------------------------------------------------------

AssetCache::AssetCache() : root_(forge_cache_dir()) {
    load_index();
}

AssetCache::AssetCache(const std::string& root) : root_(root) {
    load_index();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::string AssetCache::resolve(const std::string& url,
                                const std::string& expected_sha256) const {
    const auto it = index_.find(url);
    if (it == index_.end()) return {};
    const auto& e = it->second;
    if (!fs::exists(e.local_path)) return {};

    if (!expected_sha256.empty()) {
        auto normalize = [](std::string h) {
            const std::string pfx = "sha256:";
            if (h.size() > pfx.size() && h.substr(0, pfx.size()) == pfx)
                h = h.substr(pfx.size());
            std::transform(h.begin(), h.end(), h.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            return h;
        };
        if (normalize(expected_sha256) != normalize(e.sha256)) return {};
    }
    return e.local_path;
}

std::string AssetCache::store(const std::string& url,
                              const std::vector<uint8_t>& data) {
    try { fs::create_directories(root_ + "/files"); }
    catch (...) { return {}; }

    const std::string filename =
        sha256_of(std::vector<uint8_t>(url.begin(), url.end())) +
        extract_url_ext(url);
    const std::string local_path = root_ + "/files/" + filename;

    if (!atomic_write(local_path, data)) return {};

    Entry e;
    e.sha256     = sha256_of(data);
    e.local_path = local_path;
    e.timestamp  = current_utc_ts();
    index_[url]  = e;
    save_index();
    return local_path;
}

std::string AssetCache::sha256_of(const std::vector<uint8_t>& data) {
    Ref<HashingContext> ctx;
    ctx.instantiate();
    ctx->start(HashingContext::HASH_SHA256);

    PackedByteArray pba;
    pba.resize(static_cast<int64_t>(data.size()));
    if (!data.empty())
        std::memcpy(pba.ptrw(), data.data(), data.size());
    ctx->update(pba);

    const PackedByteArray result = ctx->finish();
    std::string hex;
    hex.reserve(result.size() * 2);
    for (int64_t i = 0; i < result.size(); ++i) {
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02x",
                      static_cast<unsigned char>(result[i]));
        hex += buf;
    }
    return hex;
}

// ---------------------------------------------------------------------------
// Index persistence
// ---------------------------------------------------------------------------

std::string AssetCache::index_path() const {
    return root_ + "/index.sml";
}

void AssetCache::load_index() {
    index_.clear();
    std::ifstream f(index_path());
    if (!f.is_open()) return;

    bool        in_entry  = false;
    std::string cur_url, cur_sha256, cur_local, cur_ts;
    std::string line;

    while (std::getline(f, line)) {
        // Trim leading whitespace
        size_t i = 0;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
        const auto tl = line.substr(i);

        if (tl.find("Entry {") != std::string::npos) {
            in_entry = true;
            cur_url.clear(); cur_sha256.clear(); cur_local.clear(); cur_ts.clear();
        } else if (in_entry && tl == "}") {
            if (!cur_url.empty() && !cur_local.empty()) {
                Entry e;
                e.sha256     = cur_sha256;
                e.local_path = cur_local;
                e.timestamp  = cur_ts;
                index_[cur_url] = e;
            }
            in_entry = false;
        } else if (in_entry) {
            if      (tl.rfind("url:",       0) == 0) cur_url    = extract_quoted(tl, "url");
            else if (tl.rfind("sha256:",    0) == 0) cur_sha256 = extract_quoted(tl, "sha256");
            else if (tl.rfind("localPath:", 0) == 0) cur_local  = extract_quoted(tl, "localPath");
            else if (tl.rfind("timestamp:", 0) == 0) cur_ts     = extract_quoted(tl, "timestamp");
        }
    }
}

void AssetCache::save_index() const {
    try { fs::create_directories(root_); }
    catch (...) { return; }

    const std::string tmp = index_path() + ".tmp";
    {
        std::ofstream f(tmp);
        if (!f.is_open()) return;

        f << "CacheIndex {\n";
        for (const auto& [url, e] : index_) {
            f << "    Entry {\n";
            f << "        url: \""       << escape_sml(url)          << "\"\n";
            f << "        sha256: \""    << escape_sml(e.sha256)     << "\"\n";
            f << "        localPath: \"" << escape_sml(e.local_path) << "\"\n";
            f << "        timestamp: \"" << escape_sml(e.timestamp)  << "\"\n";
            f << "    }\n";
        }
        f << "}\n";
        if (!f.good()) return;
    }

    std::error_code ec;
    if (fs::exists(index_path())) fs::remove(index_path(), ec);
    fs::rename(tmp, index_path(), ec);
}

bool AssetCache::atomic_write(const std::string& path,
                              const std::vector<uint8_t>& data) {
    const std::string tmp = path + ".tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f.is_open()) return false;
        if (!data.empty())
            f.write(reinterpret_cast<const char*>(data.data()),
                    static_cast<std::streamsize>(data.size()));
        if (!f.good()) return false;
    }
    std::error_code ec;
    if (fs::exists(path)) fs::remove(path, ec);
    fs::rename(tmp, path, ec);
    return !ec;
}

std::string AssetCache::url_to_local_path(const std::string& url) const {
    return root_ + "/files/" +
           sha256_of(std::vector<uint8_t>(url.begin(), url.end())) +
           extract_url_ext(url);
}

} // namespace forge

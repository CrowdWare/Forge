#pragma once

#include <filesystem>
#include <string>

namespace godot_sdk {

struct Config {
    std::string version;
    std::filesystem::path cache;
};

std::string ensure_binary(const Config& cfg, std::string& err);
bool ensure_export_templates(const Config& cfg, std::string& err);
std::string normalize_version(const std::string& version);
std::filesystem::path template_install_dir(const std::string& version_normalized);

} // namespace godot_sdk

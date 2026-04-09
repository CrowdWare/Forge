/*
#############################################################################
# Copyright (C) 2026 CrowdWare
#
# This file is part of Forge.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-CrowdWare-Commercial
#############################################################################
*/

#include "forge_logger.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

namespace forge {

Logger::Logger() {
    const char* env = std::getenv("FORGE_LOG_LEVEL");
    if (!env || env[0] == '\0') return; // default: CRITICAL

    std::string val(env);
    std::transform(val.begin(), val.end(), val.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if      (val == "debug")    level_ = LogLevel::DEBUG;
    else if (val == "info")     level_ = LogLevel::INFO;
    else if (val == "critical") level_ = LogLevel::CRITICAL;
    else if (val == "error")    level_ = LogLevel::ERROR;
}

Logger& Logger::get() {
    static Logger instance;
    return instance;
}

void Logger::debug(const std::string& msg) const {
    if (level_ > LogLevel::DEBUG) return;
    UtilityFunctions::print(String(msg.c_str()));
}

void Logger::info(const std::string& msg) const {
    if (level_ > LogLevel::INFO) return;
    UtilityFunctions::print(String(msg.c_str()));
}

void Logger::critical(const std::string& msg) const {
    if (level_ > LogLevel::CRITICAL) return;
    UtilityFunctions::push_warning(String(msg.c_str()));
}

void Logger::error(const std::string& msg) const {
    // errors always log
    UtilityFunctions::push_error(String(msg.c_str()));
}

} // namespace forge

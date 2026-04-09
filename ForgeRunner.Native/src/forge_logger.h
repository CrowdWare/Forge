/*
#############################################################################
# Copyright (C) 2026 CrowdWare
#
# This file is part of Forge.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-CrowdWare-Commercial
#############################################################################
*/

#pragma once

#include <string>

namespace forge {

// Log levels (ascending verbosity)
// Default (no env var): CRITICAL - only critical + error
// FORGE_LOG_LEVEL=info  - also info messages
// FORGE_LOG_LEVEL=debug - everything
enum class LogLevel { DEBUG = 0, INFO = 1, CRITICAL = 2, ERROR = 3 };

class Logger {
public:
    static Logger& get();

    void set_level(LogLevel l) { level_ = l; }
    LogLevel level() const { return level_; }

    void debug(const std::string& msg) const;
    void info(const std::string& msg) const;
    void critical(const std::string& msg) const;
    void error(const std::string& msg) const;

private:
    Logger();
    LogLevel level_ = LogLevel::CRITICAL;
};

} // namespace forge

// Convenience macros - prefix is always [ForgeRunner.Native]
#define FLOG_DEBUG(msg)    forge::Logger::get().debug(   std::string("[ForgeRunner.Native] ") + (msg))
#define FLOG_INFO(msg)     forge::Logger::get().info(    std::string("[ForgeRunner.Native] ") + (msg))
#define FLOG_CRITICAL(msg) forge::Logger::get().critical(std::string("[ForgeRunner.Native] ") + (msg))
#define FLOG_ERROR(msg)    forge::Logger::get().error(   std::string("[ForgeRunner.Native] ") + (msg))

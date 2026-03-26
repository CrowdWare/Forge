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

#include "forge_sms_error_policy.h"

namespace forge {

bool sms_error_is_missing_handler(const std::string& message) {
    return message.find("No SMS event handler found") != std::string::npos;
}

bool sms_error_requires_exit(const std::string& message) {
    if (message.find("RuntimeError:") != std::string::npos) return true;
    if (message.find("Stack overflow") != std::string::npos) return true;
    if (message.find("[SMS Guru Meditation]") != std::string::npos) return true;
    return false;
}

} // namespace forge


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

#pragma once

#include <cstdint>

#if defined(_WIN32)
#define SMS_EXPORT __declspec(dllexport)
#else
#define SMS_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
typedef int (*sms_native_ui_get_prop_fn)(
    const char* object_id,
    const char* property,
    char* out_json,
    int out_json_capacity,
    char* error,
    int error_capacity);

typedef int (*sms_native_ui_set_prop_fn)(
    const char* object_id,
    const char* property,
    const char* value_json,
    char* error,
    int error_capacity);

typedef int (*sms_native_ui_invoke_fn)(
    const char* object_id,
    const char* method,
    const char* args_json,
    char* out_json,
    int out_json_capacity,
    char* error,
    int error_capacity);

typedef int (*sms_native_ui_get_string_prop_fn)(
    const char* object_id,
    const char* property,
    char* out_text,
    int out_text_capacity,
    char* error,
    int error_capacity);

typedef int (*sms_native_ui_set_string_prop_fn)(
    const char* object_id,
    const char* property,
    const char* value_text,
    char* error,
    int error_capacity);

typedef int (*sms_native_sandbox_path_allow_fn)(
    const char* owner,
    const char* uri_path,
    char* error,
    int error_capacity);

SMS_EXPORT int sms_native_execute(const char* source, std::int64_t* out_result, char* error, int error_capacity);
SMS_EXPORT int sms_native_sml_parse(const char* source, std::int64_t* out_node_count, char* error, int error_capacity);
SMS_EXPORT int sms_native_session_create(std::int64_t* out_session, char* error, int error_capacity);
SMS_EXPORT int sms_native_session_load(std::int64_t session, const char* source, char* error, int error_capacity);
SMS_EXPORT int sms_native_session_invoke(
    std::int64_t session,
    const char* target_id,
    const char* event_name,
    const char* args_json,
    std::int64_t* out_result,
    char* error,
    int error_capacity);
SMS_EXPORT int sms_native_session_dispose(std::int64_t session, char* error, int error_capacity);
SMS_EXPORT int sms_native_set_ui_callbacks(
    sms_native_ui_get_prop_fn get_prop,
    sms_native_ui_set_prop_fn set_prop,
    sms_native_ui_invoke_fn invoke,
    char* error,
    int error_capacity);
SMS_EXPORT int sms_native_set_ui_string_callbacks(
    sms_native_ui_get_string_prop_fn get_string_prop,
    sms_native_ui_set_string_prop_fn set_string_prop,
    char* error,
    int error_capacity);
SMS_EXPORT int sms_native_set_sandbox_path_callback(
    sms_native_sandbox_path_allow_fn allow_path,
    char* error,
    int error_capacity);
SMS_EXPORT int sms_native_codegen_cpp(
    const char* source,
    char* out_cpp,
    int out_cpp_capacity,
    char* error,
    int error_capacity);
SMS_EXPORT int sms_native_codegen_llvm_ir(
    const char* source,
    char* out_ir,
    int out_ir_capacity,
    char* error,
    int error_capacity);
SMS_EXPORT int sms_native_aot_invoke(
    const char* source,
    const char* target_id,
    const char* event_name,
    const char* args_json,
    std::int64_t* out_result,
    char* error,
    int error_capacity);
}

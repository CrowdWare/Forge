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

#include "sms_native.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct ExecResult {
    int rc = 0;
    std::int64_t value = 0;
    std::string error;
};

struct ScopedSandboxCallback {
    explicit ScopedSandboxCallback(sms_native_sandbox_path_allow_fn fn) {
        sms_native_set_sandbox_path_callback(fn, nullptr, 0);
    }

    ~ScopedSandboxCallback() {
        sms_native_set_sandbox_path_callback(nullptr, nullptr, 0);
    }
};

int strict_scheme_only_sandbox_callback(const char*, const char* uri_path, char* error, int error_capacity) {
    const std::string path = uri_path != nullptr ? uri_path : "";
    const bool allowed_scheme = path.rfind("res:/", 0) == 0
        || path.rfind("appRes:/", 0) == 0
        || path.rfind("user:/", 0) == 0;
    if (allowed_scheme) {
        return 0;
    }
    if (error != nullptr && error_capacity > 0) {
        std::strncpy(error, "path rejected by strict test sandbox policy: unsupported URI scheme", static_cast<std::size_t>(error_capacity - 1));
        error[error_capacity - 1] = '\0';
    }
    return 1;
}

static const std::string kMantra = "Tu keinem Lebewesen Leid an.\n";

ExecResult execute(const std::string& source) {
    const std::string full = kMantra + source;
    char error[1024] = {0};
    std::int64_t value = 0;
    const auto rc = sms_native_execute(full.c_str(), &value, error, static_cast<int>(sizeof(error)));
    return {rc, value, error};
}

void expect_ok_value(const std::string& name, const std::string& source, std::int64_t expected) {
    const auto result = execute(source);
    if (result.rc != 0) {
        throw std::runtime_error(name + " failed unexpectedly: " + result.error);
    }
    if (result.value != expected) {
        throw std::runtime_error(name + " returned " + std::to_string(result.value)
            + " (expected " + std::to_string(expected) + ").");
    }
}

void expect_error_contains(const std::string& name, const std::string& source, const std::string& expected_substring) {
    const auto result = execute(source);
    if (result.rc == 0) {
        throw std::runtime_error(name + " unexpectedly succeeded.");
    }
    if (result.error.find(expected_substring) == std::string::npos) {
        throw std::runtime_error(name + " returned unexpected error: " + result.error);
    }
}

using TestFn = void(*)();

struct TestCase {
    const char* name;
    TestFn fn;
};

void test_import_res_scheme() {
    expect_ok_value("import_res_scheme", "import \"res:/libs/math.sms\"; 1;", 1);
}

void test_import_appres_scheme() {
    expect_ok_value("import_appres_scheme", "import \"appRes:/modules/ui.sms\" as ui; 2;", 2);
}

void test_import_user_scheme() {
    expect_ok_value("import_user_scheme", "import \"user:/local/tools.sms\"; 3;", 3);
}

void test_import_forbid_double_slash() {
    expect_error_contains("import_forbid_double_slash", "import \"res://libs/math.sms\"; 1;", "must not contain '//'");
}

void test_import_forbid_relative() {
    expect_error_contains("import_forbid_relative", "import \"./math.sms\"; 1;", "Relative import paths are not allowed");
}

void test_import_forbid_parent_traversal_after_scheme() {
    expect_error_contains(
        "import_forbid_parent_traversal_after_scheme",
        "import \"res:/../math.sms\"; 1;",
        "must not contain '..' segments");
}

void test_import_forbid_deep_parent_traversal_after_scheme() {
    expect_error_contains(
        "import_forbid_deep_parent_traversal_after_scheme",
        "import \"res:/mein_spezieller/pfad/aus/dem/ich/ausbreche/../../../../../../make_nonsense.sms\"; 1;",
        "must not contain '..' segments");
}

void test_runtime_error_division_by_zero() {
    expect_error_contains(
        "runtime_error_division_by_zero",
        "var x = 1 / 0;",
        "[SMS Runtime Error] division_by_zero");
}

void test_runtime_error_division_by_zero_thread_message() {
    expect_error_contains(
        "runtime_error_division_by_zero_thread_message",
        "var x = 10 / 0;",
        "SMS thread terminated. App continues.");
}

void test_runtime_error_division_by_zero_has_location() {
    expect_error_contains(
        "runtime_error_division_by_zero_has_location",
        "var x = 10 / 0;",
        "line 2, col");
}

void test_runtime_error_modulo_by_zero() {
    expect_error_contains(
        "runtime_error_modulo_by_zero",
        "var x = 5 % 0;",
        "[SMS Runtime Error] modulo_by_zero");
}

void test_runtime_error_index_out_of_bounds() {
    expect_error_contains(
        "runtime_error_index_out_of_bounds",
        "var a = [1, 2, 3]; a[10];",
        "[SMS Runtime Error] index_out_of_bounds");
}

void test_runtime_error_tuple_index_out_of_bounds() {
    expect_error_contains(
        "runtime_error_tuple_index_out_of_bounds",
        "var t = (1, 2); t[5];",
        "[SMS Runtime Error] index_out_of_bounds");
}

void test_runtime_error_null_access() {
    expect_error_contains(
        "runtime_error_null_access",
        "var x = null; x[0];",
        "[SMS Runtime Error] null_access");
}

void test_tuple_return_replaces_try_catch() {
    // Go-style error handling via tuple return — no try/catch needed
    expect_ok_value(
        "tuple_return_replaces_try_catch",
        "fun safeDivide(a: Int32, b: Int32): (Bool, String, Int32) {"
        "    if (b == 0) { return (false, \"division by zero\", 0); }"
        "    return (true, \"ok\", a / b);"
        "}"
        "var result = safeDivide(10, 0);"
        "if (result[0]) { result[2]; } else { 0; }",
        0);
}

void test_export_function_named_args_and_defaults() {
    expect_ok_value(
        "export_function_named_args_and_defaults",
        "export fun add(a: Int32, b: Int32 = 5): Int32 { return a + b; } add(a = 7);",
        12);
}

void test_data_class_named_args_and_defaults() {
    expect_ok_value(
        "data_class_named_args_and_defaults",
        "data class Vec3(x: Int32 = 1, y: Int32 = 2, z: Int32 = 3);"
        "var v = Vec3(y = 10);"
        "v.x + v.y + v.z;",
        14);
}

void test_for_in_typed_variable() {
    expect_ok_value(
        "for_in_typed_variable",
        "var list = [1, 2, 3, 4];"
        "var sum = 0;"
        "for (item: Int32 in list) { sum = sum + item; }"
        "sum;",
        10);
}

void test_named_then_positional_rejected() {
    expect_error_contains(
        "named_then_positional_rejected",
        "fun add(a, b) { return a + b; } add(a = 1, 2);",
        "Positional argument is not allowed after named argument");
}

void test_unknown_named_function_argument() {
    expect_error_contains(
        "unknown_named_function_argument",
        "fun add(a, b) { return a + b; } add(a = 1, c = 2);",
        "Unknown named argument 'c'");
}

void test_duplicate_named_function_argument() {
    expect_error_contains(
        "duplicate_named_function_argument",
        "fun add(a, b) { return a + b; } add(a = 1, a = 2, b = 3);",
        "Duplicate named argument 'a'");
}

void test_missing_required_function_argument() {
    expect_error_contains(
        "missing_required_function_argument",
        "fun add(a, b) { return a + b; } add(a = 1);",
        "Missing required argument 'b'");
}

void test_unknown_named_dataclass_argument() {
    expect_error_contains(
        "unknown_named_dataclass_argument",
        "data class Vec3(x, y, z); Vec3(y = 2, w = 9);",
        "Unknown named argument 'w'");
}

void test_duplicate_named_dataclass_argument() {
    expect_error_contains(
        "duplicate_named_dataclass_argument",
        "data class Vec3(x, y, z); Vec3(x = 1, x = 2, y = 3, z = 4);",
        "Duplicate named argument 'x'");
}

void test_missing_required_dataclass_argument() {
    expect_error_contains(
        "missing_required_dataclass_argument",
        "data class Vec3(x, y, z); Vec3(y = 2, z = 3);",
        "Missing required argument 'x'");
}

void test_invalid_import_scheme() {
    expect_error_contains(
        "invalid_import_scheme",
        "import \"file:/tmp/mod.sms\"; 1;",
        "Import path must start with");
}

void test_os_file_exists_rejects_non_sandbox_path() {
    ScopedSandboxCallback callback(&strict_scheme_only_sandbox_callback);
    expect_error_contains(
        "os_file_exists_rejects_non_sandbox_path",
        "os.fileExists(\"/tmp/file.txt\");",
        "unsupported URI scheme");
}

void test_fs_read_text_rejects_non_sandbox_path() {
    ScopedSandboxCallback callback(&strict_scheme_only_sandbox_callback);
    expect_error_contains(
        "fs_read_text_rejects_non_sandbox_path",
        "fs.readText(\"file://tmp/file.txt\");",
        "unsupported URI scheme");
}

void test_os_file_exists_trusted_fallback_without_sandbox_policy_callback() {
    expect_error_contains(
        "os_file_exists_trusted_fallback_without_sandbox_policy_callback",
        "os.fileExists(\"res:/safe/file.txt\");",
        "ui invoke bridge unavailable");
}

void test_tuple_literal_indexing() {
    expect_ok_value(
        "tuple_literal_indexing",
        "var t = (10, 20, 30); t[1];",
        20);
}

void test_tuple_literal_first_element() {
    expect_ok_value(
        "tuple_literal_first_element",
        "var t = (99, 0, 0); t[0];",
        99);
}

void test_tuple_return_from_function() {
    expect_ok_value(
        "tuple_return_from_function",
        "fun safeDivide(a: Int32, b: Int32): (Bool, Int32) {"
        "    if (b == 0) { return (false, 0); }"
        "    return (true, a / b);"
        "}"
        "var result = safeDivide(10, 2);"
        "result[1];",
        5);
}

void test_tuple_return_error_case() {
    expect_ok_value(
        "tuple_return_error_case",
        "fun safeDivide(a: Int32, b: Int32): (Bool, Int32) {"
        "    if (b == 0) { return (false, 0); }"
        "    return (true, a / b);"
        "}"
        "var result = safeDivide(10, 0);"
        "result[1];",
        0);
}

void test_tuple_return_success_flag() {
    expect_ok_value(
        "tuple_return_success_flag",
        "fun safeDivide(a: Int32, b: Int32): (Bool, Int32) {"
        "    if (b == 0) { return (false, 0); }"
        "    return (true, a / b);"
        "}"
        "var result = safeDivide(10, 2);"
        "if (result[0]) { 1; } else { 0; }",
        1);
}

void test_tuple_return_failure_flag() {
    expect_ok_value(
        "tuple_return_failure_flag",
        "fun safeDivide(a: Int32, b: Int32): (Bool, Int32) {"
        "    if (b == 0) { return (false, 0); }"
        "    return (true, a / b);"
        "}"
        "var result = safeDivide(5, 0);"
        "if (result[0]) { 1; } else { 0; }",
        0);
}

void test_type_mismatch_reassign_int_to_string() {
    expect_error_contains(
        "type_mismatch_reassign_int_to_string",
        "var count = 0; count = \"hallo\";",
        "type_mismatch");
}

void test_type_mismatch_reassign_string_to_int() {
    expect_error_contains(
        "type_mismatch_reassign_string_to_int",
        "var name = \"foo\"; name = 42;",
        "type_mismatch");
}

void test_type_mismatch_error_contains_expected_and_got() {
    expect_error_contains(
        "type_mismatch_error_contains_expected",
        "var x = 1; x = \"bad\";",
        "expected: Int32");
    expect_error_contains(
        "type_mismatch_error_contains_got",
        "var x = 1; x = \"bad\";",
        "got: String");
}

void test_type_mismatch_has_source_location() {
    expect_error_contains(
        "type_mismatch_has_source_location",
        "var x = 1; x = \"bad\";",
        "line 2, col");
}

void test_type_mismatch_has_source_line_context() {
    expect_error_contains(
        "type_mismatch_has_source_line_context",
        "var x = 1; x = \"bad\";",
        "> var x = 1; x = \"bad\";");
}

void test_runtime_error_has_source_line_context() {
    expect_error_contains(
        "runtime_error_has_source_line_context",
        "var x = 5 / 0;",
        "> var x = 5 / 0;");
}

void test_guru_meditation_recursion_limit() {
    expect_error_contains(
        "guru_meditation_recursion_limit",
        "fun inf() { return inf(); } inf();",
        "[SMS Guru Meditation] #0000002A.recursion_limit_exceeded");
}

void test_guru_meditation_contains_42() {
    expect_error_contains(
        "guru_meditation_contains_42",
        "fun inf() { return inf(); } inf();",
        "#0000002A");
}

void test_guru_meditation_point_of_no_return() {
    expect_error_contains(
        "guru_meditation_point_of_no_return",
        "fun inf() { return inf(); } inf();",
        "point of no return");
}

void test_type_consistent_reassign_allowed() {
    expect_ok_value(
        "type_consistent_reassign_allowed",
        "var x = 10; x = 20; x;",
        20);
}

void test_ahimsa_english() {
    const std::string src = "Do not harm to living beings.\n42;";
    char error[1024] = {0};
    std::int64_t value = 0;
    const int rc = sms_native_execute(src.c_str(), &value, error, static_cast<int>(sizeof(error)));
    if (rc != 0) {
        throw std::runtime_error(std::string("ahimsa_english failed: ") + error);
    }
    if (value != 42) {
        throw std::runtime_error("ahimsa_english returned " + std::to_string(value) + " (expected 42)");
    }
}

void test_ahimsa_esperanto() {
    const std::string src = "Ne dama\xc4\x9du iun ajn vivantan esta\xc4\xb5on.\n42;";
    char error[1024] = {0};
    std::int64_t value = 0;
    const int rc = sms_native_execute(src.c_str(), &value, error, static_cast<int>(sizeof(error)));
    if (rc != 0) {
        throw std::runtime_error(std::string("ahimsa_esperanto failed: ") + error);
    }
    if (value != 42) {
        throw std::runtime_error("ahimsa_esperanto returned " + std::to_string(value) + " (expected 42)");
    }
}

void test_ahimsa_missing() {
    const std::string src = "42;";
    char error[1024] = {0};
    std::int64_t value = 0;
    const int rc = sms_native_execute(src.c_str(), &value, error, static_cast<int>(sizeof(error)));
    if (rc == 0) {
        throw std::runtime_error("ahimsa_missing: expected compile error but succeeded");
    }
    if (std::string(error).find("mantra") == std::string::npos) {
        throw std::runtime_error(std::string("ahimsa_missing: unexpected error: ") + error);
    }
}

const std::vector<TestCase>& all_tests() {
    static const std::vector<TestCase> tests = {
        {"import_res_scheme", test_import_res_scheme},
        {"import_appres_scheme", test_import_appres_scheme},
        {"import_user_scheme", test_import_user_scheme},
        {"import_forbid_double_slash", test_import_forbid_double_slash},
        {"import_forbid_relative", test_import_forbid_relative},
        {"import_forbid_parent_traversal_after_scheme", test_import_forbid_parent_traversal_after_scheme},
        {"import_forbid_deep_parent_traversal_after_scheme", test_import_forbid_deep_parent_traversal_after_scheme},
        {"runtime_error_division_by_zero", test_runtime_error_division_by_zero},
        {"runtime_error_division_by_zero_thread_message", test_runtime_error_division_by_zero_thread_message},
        {"runtime_error_division_by_zero_has_location", test_runtime_error_division_by_zero_has_location},
        {"runtime_error_modulo_by_zero", test_runtime_error_modulo_by_zero},
        {"runtime_error_index_out_of_bounds", test_runtime_error_index_out_of_bounds},
        {"runtime_error_tuple_index_out_of_bounds", test_runtime_error_tuple_index_out_of_bounds},
        {"runtime_error_null_access", test_runtime_error_null_access},
        {"tuple_return_replaces_try_catch", test_tuple_return_replaces_try_catch},
        {"export_function_named_args_and_defaults", test_export_function_named_args_and_defaults},
        {"data_class_named_args_and_defaults", test_data_class_named_args_and_defaults},
        {"for_in_typed_variable", test_for_in_typed_variable},
        {"named_then_positional_rejected", test_named_then_positional_rejected},
        {"unknown_named_function_argument", test_unknown_named_function_argument},
        {"duplicate_named_function_argument", test_duplicate_named_function_argument},
        {"missing_required_function_argument", test_missing_required_function_argument},
        {"unknown_named_dataclass_argument", test_unknown_named_dataclass_argument},
        {"duplicate_named_dataclass_argument", test_duplicate_named_dataclass_argument},
        {"missing_required_dataclass_argument", test_missing_required_dataclass_argument},
        {"invalid_import_scheme", test_invalid_import_scheme},
        {"os_file_exists_rejects_non_sandbox_path", test_os_file_exists_rejects_non_sandbox_path},
        {"fs_read_text_rejects_non_sandbox_path", test_fs_read_text_rejects_non_sandbox_path},
        {"os_file_exists_trusted_fallback_without_sandbox_policy_callback", test_os_file_exists_trusted_fallback_without_sandbox_policy_callback},
        {"tuple_literal_indexing", test_tuple_literal_indexing},
        {"tuple_literal_first_element", test_tuple_literal_first_element},
        {"tuple_return_from_function", test_tuple_return_from_function},
        {"tuple_return_error_case", test_tuple_return_error_case},
        {"tuple_return_success_flag", test_tuple_return_success_flag},
        {"tuple_return_failure_flag", test_tuple_return_failure_flag},
        {"type_mismatch_reassign_int_to_string", test_type_mismatch_reassign_int_to_string},
        {"type_mismatch_reassign_string_to_int", test_type_mismatch_reassign_string_to_int},
        {"type_mismatch_error_contains_expected_and_got", test_type_mismatch_error_contains_expected_and_got},
        {"type_mismatch_has_source_location", test_type_mismatch_has_source_location},
        {"type_mismatch_has_source_line_context", test_type_mismatch_has_source_line_context},
        {"runtime_error_has_source_line_context", test_runtime_error_has_source_line_context},
        {"type_consistent_reassign_allowed", test_type_consistent_reassign_allowed},
        {"guru_meditation_recursion_limit", test_guru_meditation_recursion_limit},
        {"guru_meditation_contains_42", test_guru_meditation_contains_42},
        {"guru_meditation_point_of_no_return", test_guru_meditation_point_of_no_return},
        {"ahimsa_english", test_ahimsa_english},
        {"ahimsa_esperanto", test_ahimsa_esperanto},
        {"ahimsa_missing", test_ahimsa_missing},
    };
    return tests;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto& tests = all_tests();
        if (argc == 2) {
            const std::string requested = argv[1];
            for (const auto& test : tests) {
                if (requested == test.name) {
                    test.fn();
                    std::cout << "sms_native_spec_tests: " << test.name << " passed\n";
                    return 0;
                }
            }
            throw std::runtime_error("unknown test case: " + requested);
        }

        for (const auto& test : tests) {
            test.fn();
        }
        std::cout << "sms_native_spec_tests: all tests passed (" << tests.size() << ")\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "sms_native_spec_tests failed: " << ex.what() << "\n";
        return 1;
    }
}

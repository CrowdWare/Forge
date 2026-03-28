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
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

static const std::string kMantra = "Tu keinem Lebewesen Leid an.\n";

struct ExecResult {
    int rc = 0;
    std::int64_t value = 0;
    std::string error;
};

ExecResult execute(const std::string& source) {
    const std::string full = kMantra + source;
    char error[1024] = {0};
    std::int64_t value = 0;
    const auto rc = sms_native_execute(full.c_str(), &value, error, static_cast<int>(sizeof(error)));
    return {rc, value, error};
}

ExecResult session_invoke(const std::string& source, const char* target, const char* event_name, const char* args_json) {
    const std::string full = kMantra + source;
    char error[1024] = {0};
    std::int64_t session = 0;
    std::int64_t result = 0;

    auto rc = sms_native_session_create(&session, error, static_cast<int>(sizeof(error)));
    if (rc != 0) {
        return {rc, 0, error};
    }

    rc = sms_native_session_load(session, full.c_str(), error, static_cast<int>(sizeof(error)));
    if (rc != 0) {
        sms_native_session_dispose(session, nullptr, 0);
        return {rc, 0, error};
    }

    rc = sms_native_session_invoke(
        session,
        target,
        event_name,
        args_json,
        &result,
        error,
        static_cast<int>(sizeof(error)));
    sms_native_session_dispose(session, nullptr, 0);
    return {rc, result, error};
}

void expect_ok_value(const std::string& name, const std::string& source, std::int64_t expected) {
    const auto result = execute(source);
    if (result.rc != 0) {
        throw std::runtime_error(name + " failed unexpectedly: " + result.error);
    }
    if (result.value != expected) {
        throw std::runtime_error(name + " value=" + std::to_string(result.value)
            + " expected=" + std::to_string(expected));
    }
}

void expect_error_contains(const std::string& name, const std::string& source, const std::string& needle) {
    const auto result = execute(source);
    if (result.rc == 0) {
        throw std::runtime_error(name + " unexpectedly succeeded.");
    }
    if (result.error.find(needle) == std::string::npos) {
        throw std::runtime_error(name + " unexpected error: " + result.error);
    }
}

using TestFn = void(*)();
struct TestCase { const char* name; TestFn fn; };

void test_parse_with_event_handlers_parses_top_level_handlers() {
    expect_ok_value(
        "parse_with_event_handlers_parses_top_level_handlers",
        "on open.pressed() { 0 } on mainWindow.sizeChanged(w, h) { w + h } 1",
        1);
}

void test_parse_with_invalid_event_syntax_rejected_empty_param_comma() {
    expect_error_contains(
        "parse_with_invalid_event_syntax_rejected_empty_param_comma",
        "on open.pressed(,) { }",
        "Expected parameter name");
}

void test_parse_with_invalid_event_syntax_rejected_missing_comma() {
    expect_error_contains(
        "parse_with_invalid_event_syntax_rejected_missing_comma",
        "on open.pressed(a b) { }",
        "Expected ')' after event parameters");
}

void test_parse_with_duplicate_event_handlers_throws() {
    expect_error_contains(
        "parse_with_duplicate_event_handlers_throws",
        "on open.pressed() { } on open.pressed() { }",
        "Duplicate event handler");
}

void test_runtime_invoke_event_with_no_handler_returns_error() {
    const auto result = session_invoke("var x = 1", "open", "pressed", "[]");
    if (result.rc == 0) {
        throw std::runtime_error("runtime_invoke_event_with_no_handler_returns_error unexpectedly succeeded.");
    }
    if (result.error.find("No SMS event handler found") == std::string::npos) {
        throw std::runtime_error("unexpected error: " + result.error);
    }
}

void test_runtime_invoke_event_binds_args_and_executes_body() {
    const auto result = session_invoke(
        "on mainWindow.sizeChanged(w, h) { return w + h }",
        "mainWindow",
        "sizeChanged",
        "[10,20]");
    if (result.rc != 0) {
        throw std::runtime_error("runtime_invoke_event_binds_args_and_executes_body failed: " + result.error);
    }
    if (result.value != 30) {
        throw std::runtime_error("expected 30, got " + std::to_string(result.value));
    }
}

void test_runtime_invoke_event_with_arg_count_mismatch_throws() {
    const auto result = session_invoke(
        "on mainWindow.sizeChanged(w, h) { return 1 }",
        "mainWindow",
        "sizeChanged",
        "[10]");
    if (result.rc == 0) {
        throw std::runtime_error("runtime_invoke_event_with_arg_count_mismatch_throws unexpectedly succeeded.");
    }
    if (result.error.find("expects 2 arg(s), got 1") == std::string::npos) {
        throw std::runtime_error("unexpected error: " + result.error);
    }
}

void test_parse_multiline_global_function_call_parses() {
    expect_ok_value(
        "parse_multiline_global_function_call_parses",
        "fun foo(x, y, z) { return x + y + z }"
        "var a = 1 var b = 2 var c = 3 "
        "var r = foo( a, b, c ) r",
        6);
}

void test_parse_multiline_method_call_parses() {
    expect_ok_value(
        "parse_multiline_method_call_parses",
        "var list = [1,2,3] var r = list.removeAt( 1 ) r",
        2);
}

void test_parse_multiline_nested_calls_parses() {
    expect_ok_value(
        "parse_multiline_nested_calls_parses",
        "fun inner(a,b){ return a + b }"
        "fun outer(v,c){ return v + c }"
        "var r = outer( inner(1,2), 3 )"
        "r",
        6);
}

void test_parse_integer_literal_uses_integer_runtime_value() {
    expect_ok_value("parse_integer_literal_uses_integer_runtime_value", "var x = 42 x", 42);
}

void test_parse_double_literals_are_rejected_in_native_subset() {
    expect_error_contains(
        "parse_double_literals_are_rejected_in_native_subset",
        "var x = 1.0",
        "Expected property name after '.'");
}

void test_parse_exponentials_are_rejected() {
    expect_error_contains(
        "parse_exponentials_are_rejected",
        "var x = 1e3",
        "Unknown variable");
}

void test_runtime_recursive_function_hits_recursion_limit() {
    const auto result = execute(
        "fun loop() { return loop() }"
        "loop()");
    if (result.rc == 0) {
        throw std::runtime_error("runtime_recursive_function_hits_recursion_limit unexpectedly succeeded.");
    }
    if (result.error.find("Guru Meditation") == std::string::npos
        && result.error.find("recursion_limit_exceeded") == std::string::npos) {
        throw std::runtime_error("unexpected error: " + result.error);
    }
}

void test_string_concat_dollar_operator_with_call_parses_and_executes() {
    expect_ok_value(
        "string_concat_dollar_operator_with_call_parses_and_executes",
        "fun getAnswer() { return 42 } "
        "var s = \"The answer of all questions is: \"$getAnswer() "
        "1",
        1);
}

void test_string_interpolation_braces_with_call_parses_and_executes() {
    expect_ok_value(
        "string_interpolation_braces_with_call_parses_and_executes",
        "fun getAnswer() { return 42 } "
        "var s = \"The answer of all questions is: ${getAnswer()}\" "
        "1",
        1);
}

const std::vector<TestCase>& all_tests() {
    static const std::vector<TestCase> tests = {
        {"sms_events_parse_with_event_handlers_parses_top_level_handlers", test_parse_with_event_handlers_parses_top_level_handlers},
        {"sms_events_parse_with_invalid_event_syntax_rejected_empty_param_comma", test_parse_with_invalid_event_syntax_rejected_empty_param_comma},
        {"sms_events_parse_with_invalid_event_syntax_rejected_missing_comma", test_parse_with_invalid_event_syntax_rejected_missing_comma},
        {"sms_events_parse_with_duplicate_event_handlers_throws", test_parse_with_duplicate_event_handlers_throws},
        {"sms_events_runtime_invoke_event_with_no_handler_returns_error", test_runtime_invoke_event_with_no_handler_returns_error},
        {"sms_events_runtime_invoke_event_binds_args_and_executes_body", test_runtime_invoke_event_binds_args_and_executes_body},
        {"sms_events_runtime_invoke_event_with_arg_count_mismatch_throws", test_runtime_invoke_event_with_arg_count_mismatch_throws},
        {"sms_parser_multiline_global_function_call_parses", test_parse_multiline_global_function_call_parses},
        {"sms_parser_multiline_method_call_parses", test_parse_multiline_method_call_parses},
        {"sms_parser_multiline_nested_calls_parses", test_parse_multiline_nested_calls_parses},
        {"sms_parser_integer_literal_uses_integer_runtime_value", test_parse_integer_literal_uses_integer_runtime_value},
        {"sms_parser_double_literals_are_rejected_in_native_subset", test_parse_double_literals_are_rejected_in_native_subset},
        {"sms_parser_exponentials_are_rejected", test_parse_exponentials_are_rejected},
        {"sms_runtime_recursive_function_hits_recursion_limit", test_runtime_recursive_function_hits_recursion_limit},
        {"sms_parser_string_concat_dollar_operator_with_call_parses_and_executes", test_string_concat_dollar_operator_with_call_parses_and_executes},
        {"sms_parser_string_interpolation_braces_with_call_parses_and_executes", test_string_interpolation_braces_with_call_parses_and_executes},
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
                    std::cout << "sms_native_parser_events_tests: " << test.name << " passed\n";
                    return 0;
                }
            }
            throw std::runtime_error("unknown test case: " + requested);
        }

        for (const auto& test : tests) {
            test.fn();
        }
        std::cout << "sms_native_parser_events_tests: all tests passed (" << tests.size() << ")\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "sms_native_parser_events_tests failed: " << ex.what() << "\n";
        return 1;
    }
}

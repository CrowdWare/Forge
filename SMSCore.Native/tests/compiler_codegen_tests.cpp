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
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

static const std::string kMantra = "Tu keinem Lebewesen Leid an.\n";

struct ApiResult {
    int rc = 0;
    std::string text;
    std::int64_t value = 0;
};

void assert_true(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

ApiResult codegen_llvm_ir(const std::string& source, int out_capacity = 1024 * 128) {
    const std::string full = kMantra + source;
    std::vector<char> out(static_cast<std::size_t>(out_capacity), '\0');
    char error[2048] = {};
    const int rc = sms_native_codegen_llvm_ir(
        full.c_str(),
        out.data(),
        out_capacity,
        error,
        static_cast<int>(sizeof(error)));
    return {rc, rc == 0 ? std::string(out.data()) : std::string(error), 0};
}

ApiResult aot_invoke(const std::string& source, const std::string& target, const std::string& event, const std::string& args_json) {
    const std::string full = kMantra + source;
    char error[4096] = {};
    std::int64_t result = 0;
    const int rc = sms_native_aot_invoke(
        full.c_str(),
        target.c_str(),
        event.c_str(),
        args_json.c_str(),
        &result,
        error,
        static_cast<int>(sizeof(error)));
    return {rc, std::string(error), result};
}

bool command_exists_in_path(const std::string& command) {
    if (command.empty()) {
        return false;
    }
    const std::filesystem::path as_path(command);
    if (as_path.is_absolute() || command.find('/') != std::string::npos || command.find('\\') != std::string::npos) {
        return std::filesystem::exists(as_path);
    }

    const char* path_env = std::getenv("PATH");
    if (path_env == nullptr || path_env[0] == '\0') {
        return false;
    }
    const std::string path(path_env);
#if defined(_WIN32)
    const char sep = ';';
#else
    const char sep = ':';
#endif
    std::size_t start = 0;
    while (start <= path.size()) {
        std::size_t end = path.find(sep, start);
        if (end == std::string::npos) end = path.size();
        const auto dir = path.substr(start, end - start);
        if (!dir.empty()) {
            std::filesystem::path candidate = std::filesystem::path(dir) / command;
            if (std::filesystem::exists(candidate)) return true;
#if defined(_WIN32)
            candidate = std::filesystem::path(dir) / (command + ".exe");
            if (std::filesystem::exists(candidate)) return true;
#endif
        }
        if (end == path.size()) break;
        start = end + 1;
    }
    return false;
}

void set_env_var(const std::string& key, const std::string& value) {
#if defined(_WIN32)
    _putenv_s(key.c_str(), value.c_str());
#else
    setenv(key.c_str(), value.c_str(), 1);
#endif
}

void unset_env_var(const std::string& key) {
#if defined(_WIN32)
    _putenv_s(key.c_str(), "");
#else
    unsetenv(key.c_str());
#endif
}

struct ScopedEnvVar {
    std::string key;
    bool had_old = false;
    std::string old_value;

    ScopedEnvVar(const std::string& k, const std::string& value) : key(k) {
        const char* old = std::getenv(key.c_str());
        if (old != nullptr) {
            had_old = true;
            old_value = old;
        }
        set_env_var(key, value);
    }

    ~ScopedEnvVar() {
        if (had_old) {
            set_env_var(key, old_value);
        } else {
            unset_env_var(key);
        }
    }
};

void test_codegen_llvm_ir_emits_main_for_top_level_call() {
    const auto result = codegen_llvm_ir(
        "fun getAnswer() { return 42 }\n"
        "getAnswer()\n");
    assert_true(result.rc == 0, "llvm codegen failed: " + result.text);
    assert_true(result.text.find("define i32 @main()") != std::string::npos, "missing llvm main function");
    assert_true(result.text.find("call i64 @sms_getAnswer()") != std::string::npos, "missing getAnswer call in llvm IR");
}

void test_codegen_llvm_ir_uses_main_function_fallback() {
    const auto result = codegen_llvm_ir(
        "fun main() { return 42 }\n");
    assert_true(result.rc == 0, "llvm codegen failed: " + result.text);
    assert_true(result.text.find("call i64 @sms_main()") != std::string::npos, "missing main() fallback call in llvm IR");
}

void test_codegen_llvm_ir_reports_buffer_too_small() {
    const auto result = codegen_llvm_ir("fun main() { return 42 }\n", 16);
    assert_true(result.rc != 0, "llvm codegen unexpectedly succeeded with tiny output buffer");
    assert_true(result.text.find("output buffer too small") != std::string::npos, "unexpected error: " + result.text);
}

void test_codegen_llvm_ir_reports_unsupported_expression() {
    const auto result = codegen_llvm_ir(
        "fun main() { var xs = [1,2,3] return 42 }\n");
    assert_true(result.rc != 0, "llvm codegen unexpectedly succeeded for unsupported expression");
    assert_true(result.text.find("unsupported expression type") != std::string::npos, "unexpected error: " + result.text);
}

void test_aot_invoke_validates_required_arguments() {
    char error[512] = {};
    const int rc = sms_native_aot_invoke(nullptr, "x", "y", "[]", nullptr, error, static_cast<int>(sizeof(error)));
    assert_true(rc == 2, "aot invoke should reject null required arguments");
    assert_true(std::string(error).find("must not be null") != std::string::npos, "unexpected error: " + std::string(error));
}

void test_aot_invoke_reports_clang_failure() {
    const auto tmp = std::filesystem::temp_directory_path() / "sms_native_aot_codegen_test_cache";
    std::error_code ec;
    std::filesystem::remove_all(tmp, ec);
    std::filesystem::create_directories(tmp, ec);

    ScopedEnvVar cache_env("SMS_NATIVE_AOT_CACHE_DIR", tmp.string());
    ScopedEnvVar clang_env("SMS_NATIVE_CLANG", "clang_definitely_not_installed_for_test");

    const auto result = aot_invoke(
        "fun getAnswer() { return 42 }\n"
        "getAnswer()\n",
        "mainWindow",
        "ready",
        "[]");
    assert_true(result.rc != 0, "aot invoke unexpectedly succeeded with invalid clang command");
    assert_true(result.text.find("aot clang build failed") != std::string::npos, "unexpected error: " + result.text);
}

void test_aot_invoke_succeeds_when_clang_available_or_skips() {
    const char* env_clang = std::getenv("SMS_NATIVE_CLANG");
    const std::string clang_cmd = (env_clang != nullptr && env_clang[0] != '\0')
        ? std::string(env_clang)
        : std::string("clang");
    if (!command_exists_in_path(clang_cmd)) {
        std::cout << "sms_native_compiler_codegen_tests: skip aot success test (clang not available)\n";
        return;
    }

    const auto tmp = std::filesystem::temp_directory_path() / "sms_native_aot_codegen_test_cache_success";
    std::error_code ec;
    std::filesystem::remove_all(tmp, ec);
    std::filesystem::create_directories(tmp, ec);
    ScopedEnvVar cache_env("SMS_NATIVE_AOT_CACHE_DIR", tmp.string());

    const auto result = aot_invoke(
        "fun getAnswer() { return 42 }\n"
        "getAnswer()\n",
        "mainWindow",
        "ready",
        "[]");
    assert_true(result.rc == 0, "aot invoke failed: " + result.text);
    assert_true(result.value == 42, "aot invoke expected 42, got " + std::to_string(result.value));
}

void test_aot_invoke_reuses_cached_binary_without_clang_or_skips() {
    const char* env_clang = std::getenv("SMS_NATIVE_CLANG");
    const std::string clang_cmd = (env_clang != nullptr && env_clang[0] != '\0')
        ? std::string(env_clang)
        : std::string("clang");
    if (!command_exists_in_path(clang_cmd)) {
        std::cout << "sms_native_compiler_codegen_tests: skip aot cache reuse test (clang not available)\n";
        return;
    }

    const auto tmp = std::filesystem::temp_directory_path() / "sms_native_aot_codegen_test_cache_reuse";
    std::error_code ec;
    std::filesystem::remove_all(tmp, ec);
    std::filesystem::create_directories(tmp, ec);
    ScopedEnvVar cache_env("SMS_NATIVE_AOT_CACHE_DIR", tmp.string());

    const std::string source =
        "fun getAnswer() { return 42 }\n"
        "getAnswer()\n";

    const auto first = aot_invoke(source, "mainWindow", "ready", "[]");
    assert_true(first.rc == 0, "first aot invoke failed: " + first.text);
    assert_true(first.value == 42, "first aot invoke expected 42, got " + std::to_string(first.value));

    ScopedEnvVar clang_env("SMS_NATIVE_CLANG", "clang_definitely_not_installed_for_cache_reuse_test");
    const auto second = aot_invoke(source, "mainWindow", "ready", "[]");
    assert_true(second.rc == 0, "second aot invoke should succeed from cache, got: " + second.text);
    assert_true(second.value == 42, "second aot invoke expected 42, got " + std::to_string(second.value));
}

void test_aot_invoke_emits_shared_library_artifact_or_skips() {
    const char* env_clang = std::getenv("SMS_NATIVE_CLANG");
    const std::string clang_cmd = (env_clang != nullptr && env_clang[0] != '\0')
        ? std::string(env_clang)
        : std::string("clang");
    if (!command_exists_in_path(clang_cmd)) {
        std::cout << "sms_native_compiler_codegen_tests: skip aot shared-lib artifact test (clang not available)\n";
        return;
    }

    const auto tmp = std::filesystem::temp_directory_path() / "sms_native_aot_codegen_test_cache_artifact";
    std::error_code ec;
    std::filesystem::remove_all(tmp, ec);
    std::filesystem::create_directories(tmp, ec);
    ScopedEnvVar cache_env("SMS_NATIVE_AOT_CACHE_DIR", tmp.string());

    const auto result = aot_invoke(
        "fun getAnswer() { return 42 }\n"
        "getAnswer()\n",
        "mainWindow",
        "ready",
        "[]");
    assert_true(result.rc == 0, "aot invoke failed: " + result.text);

#if defined(_WIN32)
    const std::string expected_ext = ".dll";
#elif defined(__APPLE__)
    const std::string expected_ext = ".dylib";
#else
    const std::string expected_ext = ".so";
#endif

    bool found_shared_lib = false;
    for (const auto& entry : std::filesystem::directory_iterator(tmp)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension().string() == expected_ext) {
            found_shared_lib = true;
            break;
        }
    }
    assert_true(found_shared_lib, "expected compiled shared library artifact (" + expected_ext + ") in AOT cache");
}

using TestFn = void(*)();
struct TestCase { const char* name; TestFn fn; };

const std::vector<TestCase>& all_tests() {
    static const std::vector<TestCase> tests = {
        {"sms_compiler_codegen_llvm_ir_emits_main_for_top_level_call", test_codegen_llvm_ir_emits_main_for_top_level_call},
        {"sms_compiler_codegen_llvm_ir_uses_main_function_fallback", test_codegen_llvm_ir_uses_main_function_fallback},
        {"sms_compiler_codegen_llvm_ir_reports_buffer_too_small", test_codegen_llvm_ir_reports_buffer_too_small},
        {"sms_compiler_codegen_llvm_ir_reports_unsupported_expression", test_codegen_llvm_ir_reports_unsupported_expression},
        {"sms_compiler_aot_invoke_validates_required_arguments", test_aot_invoke_validates_required_arguments},
        {"sms_compiler_aot_invoke_reports_clang_failure", test_aot_invoke_reports_clang_failure},
        {"sms_compiler_aot_invoke_succeeds_when_clang_available_or_skips", test_aot_invoke_succeeds_when_clang_available_or_skips},
        {"sms_compiler_aot_invoke_reuses_cached_binary_without_clang_or_skips", test_aot_invoke_reuses_cached_binary_without_clang_or_skips},
        {"sms_compiler_aot_invoke_emits_shared_library_artifact_or_skips", test_aot_invoke_emits_shared_library_artifact_or_skips},
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
                    std::cout << "sms_native_compiler_codegen_tests: " << test.name << " passed\n";
                    return 0;
                }
            }
            throw std::runtime_error("unknown test case: " + requested);
        }

        for (const auto& test : tests) {
            test.fn();
        }
        std::cout << "sms_native_compiler_codegen_tests: all tests passed (" << tests.size() << ")\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "sms_native_compiler_codegen_tests failed: " << ex.what() << "\n";
        return 1;
    }
}

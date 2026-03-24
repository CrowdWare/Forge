/*
#############################################################################
# Copyright (C) 2026 CrowdWare
#
# This file is part of Forge.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-CrowdWare-Commercial
#############################################################################
*/

#include "sms_native.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

// ── Path B: equivalent native C++ source (compiled externally with -O0) ─────

static const char* k_native_cpp_source =
    "#include <cstdint>\n"
    "#include <chrono>\n"
    "#include <cstdio>\n"
    "\n"
    "static int64_t bench() {\n"
    "    int64_t i = 0, a = 0, b = 1;\n"
    "    while (i < 1000000) {\n"
    "        i = i + 1;\n"
    "        int64_t c = a + b;\n"
    "        a = b;\n"
    "        b = c;\n"
    "    }\n"
    "    return b;\n"
    "}\n"
    "\n"
    "int main() {\n"
    "    auto t0 = std::chrono::high_resolution_clock::now();\n"
    "    int64_t result = bench();\n"
    "    auto t1 = std::chrono::high_resolution_clock::now();\n"
    "    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();\n"
    "    printf(\"RESULT:%lld\\n\", (long long)result);\n"
    "    printf(\"TIME_US:%.1f\\n\", us);\n"
    "    return 0;\n"
    "}\n";

// ── Path A: SMS interpreter ──────────────────────────────────────────────────

static const char* k_sms_source =
    "fun bench() {\n"
    "    var i = 0\n"
    "    var a = 0\n"
    "    var b = 1\n"
    "    while (i < 1000000) {\n"
    "        i = i + 1\n"
    "        var c = a + b\n"
    "        a = b\n"
    "        b = c\n"
    "    }\n"
    "    return b\n"
    "}\n"
    "bench()\n";

static bool compile_and_run(const char* source, const char* src_path, const char* bin_path,
                             const char* compiler_cmd_fmt,
                             int64_t& out_result, double& out_us) {
    std::ofstream f(src_path);
    f << source;
    f.close();

    char cmd[512];
    std::snprintf(cmd, sizeof(cmd), compiler_cmd_fmt, bin_path, src_path);
    FILE* fp = popen(cmd, "r");
    if (!fp) return false;
    char compile_out[1024] = {};
    const auto n = std::fread(compile_out, 1, sizeof(compile_out) - 1, fp);
    (void)n;
    if (pclose(fp) != 0) {
        std::fprintf(stderr, "compile error: %s\n", compile_out);
        return false;
    }

    FILE* run = popen(bin_path, "r");
    if (!run) return false;
    char line[256];
    bool got_result = false, got_time = false;
    while (std::fgets(line, sizeof(line), run)) {
        if (std::strncmp(line, "RESULT:", 7) == 0) {
            out_result = std::strtoll(line + 7, nullptr, 10);
            got_result = true;
        } else if (std::strncmp(line, "TIME_US:", 8) == 0) {
            out_us = std::strtod(line + 8, nullptr);
            got_time = true;
        }
    }
    pclose(run);
    return got_result && got_time;
}

int main() {
    // warmup for interpreter
    {
        std::int64_t r = 0;
        char err[256] = {};
        sms_native_execute(k_sms_source, &r, err, static_cast<int>(sizeof(err)));
    }

    // ── Path A: SMS interpreter ──
    std::int64_t sms_result = 0;
    char error[256] = {};
    auto t0 = std::chrono::high_resolution_clock::now();
    const int rc = sms_native_execute(k_sms_source, &sms_result, error, static_cast<int>(sizeof(error)));
    auto t1 = std::chrono::high_resolution_clock::now();

    if (rc != 0) {
        std::fprintf(stderr, "SMS error: %s\n", error);
        return 1;
    }
    const double sms_us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    // ── Path B: hand-written C++ equivalent, compiled -O0 ──
    double native_us = -1.0;
    int64_t native_result = -1;
    const bool native_ok = compile_and_run(
        k_native_cpp_source, "/tmp/sms_native_bench.cpp", "/tmp/sms_native_bench",
        "g++ -O2 -std=c++17 -o %s %s 2>&1",
        native_result, native_us);

    // ── Path C: SMS → generated C++ compiled with -O0 ──
    double aot_us = -1.0;
    int64_t aot_result = -1;
    bool aot_ok = false;
    {
        char cpp_buf[65536] = {};
        char cg_err[512] = {};
        const int cg_rc = sms_native_codegen_cpp(k_sms_source, cpp_buf, static_cast<int>(sizeof(cpp_buf)), cg_err, static_cast<int>(sizeof(cg_err)));
        if (cg_rc != 0) {
            std::fprintf(stderr, "codegen error: %s\n", cg_err);
        } else {
            aot_ok = compile_and_run(cpp_buf, "/tmp/sms_aot_bench.cpp", "/tmp/sms_aot_bench",
                "g++ -O2 -std=c++17 -o %s %s 2>&1", aot_result, aot_us);
        }
    }

    // ── Path D: SMS → LLVM IR compiled with clang -O2 ──
    double llvm_us = -1.0;
    int64_t llvm_result = -1;
    bool llvm_ok = false;
    {
        char ir_buf[65536] = {};
        char ir_err[512] = {};
        const int ir_rc = sms_native_codegen_llvm_ir(k_sms_source, ir_buf, static_cast<int>(sizeof(ir_buf)), ir_err, static_cast<int>(sizeof(ir_err)));
        if (ir_rc != 0) {
            std::fprintf(stderr, "llvm codegen error: %s\n", ir_err);
        } else {
            llvm_ok = compile_and_run(ir_buf, "/tmp/sms_llvm_bench.ll", "/tmp/sms_llvm_bench",
                "clang -O2 -o %s %s 2>&1", llvm_result, llvm_us);
        }
    }

    // ── Output ──
    const bool match_ab = native_ok && (sms_result == native_result);
    const bool match_ac = aot_ok   && (sms_result == aot_result);
    const bool match_ad = llvm_ok  && (sms_result == llvm_result);
    const double factor_b = (native_ok && native_us > 0.001) ? (sms_us / native_us) : 0.0;
    const double factor_c = (aot_ok   && aot_us   > 0.001) ? (sms_us / aot_us)   : 0.0;
    const double factor_d = (llvm_ok  && llvm_us  > 0.001) ? (sms_us / llvm_us)  : 0.0;

    std::printf("\n");
    std::printf("SMS Benchmark -- CrowdWare Forge 2026\n");
    std::printf("sum 1..1,000,000 | int64_t | no I/O in loop\n");
    std::printf("----------------------------------------------------------\n");
    std::printf("  Path A  SMS interpreter       : %10.1f us  result=%lld\n",
                sms_us, (long long)sms_result);
    if (native_ok)
        std::printf("  Path B  native C++    (-O2)   : %10.1f us  result=%lld  (%.0fx faster)\n",
                    native_us, (long long)native_result, factor_b);
    else
        std::printf("  Path B  native C++    (-O2)   : g++ not available\n");
    if (aot_ok)
        std::printf("  Path C  SMS -> C++    (-O2)   : %10.1f us  result=%lld  (%.0fx faster)\n",
                    aot_us, (long long)aot_result, factor_c);
    else
        std::printf("  Path C  SMS -> C++    (-O2)   : g++ not available or compile failed\n");
    if (llvm_ok)
        std::printf("  Path D  SMS -> LLVM IR(-O2)   : %10.1f us  result=%lld  (%.0fx faster)\n",
                    llvm_us, (long long)llvm_result, factor_d);
    else
        std::printf("  Path D  SMS -> LLVM IR(-O2)   : clang not available or compile failed\n");
    std::printf("----------------------------------------------------------\n");
    if (native_ok) std::printf("  Result check A==B : %s\n", match_ab ? "OK" : "MISMATCH");
    if (aot_ok)    std::printf("  Result check A==C : %s\n", match_ac ? "OK" : "MISMATCH");
    if (llvm_ok)   std::printf("  Result check A==D : %s\n", match_ad ? "OK" : "MISMATCH");
    std::printf("\n");

    return ((!native_ok || match_ab) && (!aot_ok || match_ac) && (!llvm_ok || match_ad)) ? 0 : 1;
}

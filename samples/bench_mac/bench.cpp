// C++ Benchmark — CrowdWare Forge 2026
// Compiled: clang -O0 (same optimizer level as SMS → LLVM IR path)
// Exact same algorithm as bench.sms

#include <cstdint>
#include <cstdio>
#include <chrono>

static int64_t fib(int64_t n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

static int64_t lcgChain(int64_t seed, int64_t count) {
    int64_t acc = seed;
    int64_t i = 0;
    while (i < count) {
        acc = (acc * 1664525 + 1013904223) % 1000000007;
        i = i + 1;
    }
    return acc;
}

static int64_t nestedMod(int64_t n) {
    int64_t sum = 0, i = 0;
    while (i < n) {
        int64_t j = 1;
        while (j <= n) {
            sum = (sum + i * j) % 999983;
            j = j + 1;
        }
        i = i + 1;
    }
    return sum;
}

int main() {
    auto t0 = std::chrono::high_resolution_clock::now();
    int64_t r1 = fib(36);
    int64_t r2 = lcgChain(42, 2000000);
    int64_t r3 = nestedMod(800);
    int64_t result = r1 + r2 + r3;
    auto t1 = std::chrono::high_resolution_clock::now();
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    printf("RESULT:%lld\n", (long long)result);
    printf("TIME_US:%.1f\n", us);
    return 0;
}

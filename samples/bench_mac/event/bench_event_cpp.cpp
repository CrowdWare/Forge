// C++ Event Dispatch Benchmark — CrowdWare Forge 2026
// Simulates equivalent event dispatch: unordered_map<string, function> lookup + call
// Compiled: clang -O0

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <unordered_map>

int main() {
    int64_t counter = 0;

    std::unordered_map<std::string, std::function<int64_t()>> dispatch;
    dispatch["bench.tick"] = [&counter]() -> int64_t {
        counter = counter + 1;
        return counter;
    };

    const int kWarmup = 1000;
    const int kRuns   = 100000;
    int64_t result = 0;

    // Warmup
    for (int i = 0; i < kWarmup; i++) {
        result = dispatch["bench.tick"]();
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kRuns; i++) {
        result = dispatch["bench.tick"]();
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    double total_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    double per_ns   = (total_us * 1000.0) / kRuns;

    printf("RESULT:%lld\n",       (long long)result);
    printf("TOTAL_US:%.1f\n",     total_us);
    printf("PER_EVENT_NS:%.1f\n", per_ns);
    return 0;
}

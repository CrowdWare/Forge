// SMS AOT Event Dispatch Benchmark — CrowdWare Forge 2026
// Compiled event handler called directly — no interpreter, no map lookup.
// sms_compile --emit-obj bench_event_sms_aot.sms → bench_event_sms_aot.o
// Linked with this harness: sms_on_bench_tick() is a plain native function.

#include <chrono>
#include <cstdint>
#include <cstdio>

// Symbol emitted by sms_compile for:  on bench.tick() { ... }
extern "C" int64_t sms_on_bench_tick();

int main() {
    const int kWarmup = 1000;
    const int kRuns   = 100000;
    int64_t result    = 0;

    for (int i = 0; i < kWarmup; i++) result = sms_on_bench_tick();

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kRuns; i++) result = sms_on_bench_tick();
    auto t1 = std::chrono::high_resolution_clock::now();

    double total_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    double per_ns   = (total_us * 1000.0) / kRuns;

    printf("RESULT:%lld\n",       (long long)result);
    printf("TOTAL_US:%.1f\n",     total_us);
    printf("PER_EVENT_NS:%.1f\n", per_ns);
    return 0;
}

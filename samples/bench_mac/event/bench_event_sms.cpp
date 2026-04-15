// SMS Event Dispatch Benchmark — CrowdWare Forge 2026
// Measures: on bench.tick() { ... } dispatch latency via SMS interpreter
// Links against libsms_native (sms-cpp)

#include "sms_native.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>

static const char* kSmsSource = R"SMS(
var counter = 0

on bench.tick() {
    counter = counter + 1
    return counter
}
)SMS";

int main() {
    char error[512] = {};
    int64_t session = -1;

    if (sms_native_session_create(&session, error, (int)sizeof(error)) != 0) {
        fprintf(stderr, "session_create failed: %s\n", error);
        return 1;
    }
    if (sms_native_session_load(session, kSmsSource, error, (int)sizeof(error)) != 0) {
        fprintf(stderr, "session_load failed: %s\n", error);
        sms_native_session_dispose(session, nullptr, 0);
        return 1;
    }

    const int kWarmup = 1000;
    const int kRuns   = 100000;
    int64_t result = 0;

    // Warmup
    for (int i = 0; i < kWarmup; i++) {
        sms_native_session_invoke(session, "bench", "tick", "[]", &result, error, (int)sizeof(error));
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kRuns; i++) {
        sms_native_session_invoke(session, "bench", "tick", "[]", &result, error, (int)sizeof(error));
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    double total_us  = std::chrono::duration<double, std::micro>(t1 - t0).count();
    double per_ns    = (total_us * 1000.0) / kRuns;

    sms_native_session_dispose(session, nullptr, 0);

    printf("RESULT:%lld\n",      (long long)result);
    printf("TOTAL_US:%.1f\n",    total_us);
    printf("PER_EVENT_NS:%.1f\n", per_ns);
    return 0;
}

// bench_sms_aot.cpp
// SMS benchmark translated to native C++ (AOT path).
// Matches kSmsBenchmarkSource in sms_bridge_jni.cpp exactly:
//   fib(24), loopTest (10 000), nestedTest (25^3), mathCheck (2 000), stringBench (10 000).
// No interpreter, no SMS runtime, no clang subprocess — compiled directly by the NDK.

#include <cstdint>
#include <string>

namespace {

int64_t sms_fib(int64_t n) {
    if (n <= 1) return n;
    return sms_fib(n - 1) + sms_fib(n - 2);
}

int64_t sms_double(int64_t x) {
    return x * 2;
}

int64_t sms_loopTest() {
    int64_t sum = 0, i = 0;
    while (i < 10000) {
        sum = sum + sms_double(i);
        i = i + 1;
    }
    return sum;
}

int64_t sms_nestedTest() {
    int64_t sum = 0, i = 0;
    while (i < 25) {
        int64_t j = 0;
        while (j < 25) {
            int64_t k = 0;
            while (k < 25) {
                sum = sum + 1;
                k = k + 1;
            }
            j = j + 1;
        }
        i = i + 1;
    }
    return sum;
}

int64_t sms_mathCheck() {
    int64_t acc = 7, i = 1;
    while (i <= 2000) {
        acc = (acc * 31 + i * 17) % 1000000007LL;
        i = i + 1;
    }
    return acc;
}

int64_t sms_stringTouch(const std::string& a, const std::string& b, int64_t sel) {
    const std::string& chosen = (sel == 1) ? b : a;
    std::string joined = chosen + "-books";
    return (joined == "crowdware-books") ? 11 : 17;
}

int64_t sms_stringBench() {
    int64_t acc = 0, i = 0;
    while (i < 10000) {
        int64_t sel = i % 2;
        acc = acc + sms_stringTouch("crowdware", "forge", sel);
        i = i + 1;
    }
    return acc;
}

int64_t sms_runBench() {
    int64_t t1 = sms_fib(24);
    int64_t t2 = sms_loopTest();
    int64_t t3 = sms_nestedTest();
    int64_t t4 = sms_mathCheck();
    int64_t t5 = sms_stringBench();
    return t1 + t2 + t3 + t4 + t5;
}

} // namespace

extern "C" int64_t sms_bench_aot_run() {
    return sms_runBench();
}

extern "C" __attribute__((noinline)) void sms_bench_aot_killer() {
    volatile int depth = 0;
    (void)depth;
    sms_bench_aot_killer();
}

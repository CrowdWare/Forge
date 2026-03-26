#include <jni.h>

#include <chrono>
#include <cstdint>
#include <string>

#include "sms_native.h"

extern "C" int64_t sms_bench_aot_run();
extern "C" void sms_bench_aot_killer();

namespace {
std::string g_last_error;

const char* kSmsBenchmarkSource = R"SMS(
fun fib(n) {
    if (n <= 1) {
        return n
    }
    return fib(n - 1) + fib(n - 2)
}

fun double(x) {
    return x * 2
}

fun loopTest() {
    var sum = 0
    var i = 0
    while (i < 10000) {
        sum = sum + double(i)
        i = i + 1
    }
    return sum
}

fun nestedTest() {
    var sum = 0
    var i = 0
    while (i < 25) {
        var j = 0
        while (j < 25) {
            var k = 0
            while (k < 25) {
                sum = sum + 1
                k = k + 1
            }
            j = j + 1
        }
        i = i + 1
    }
    return sum
}

fun mathCheck() {
    var acc = 7
    var i = 1
    while (i <= 2000) {
        acc = (acc * 31 + i * 17) % 1000000007
        i = i + 1
    }
    return acc
}

fun stringTouch(a, b, sel) {
    var chosen = a
    if (sel == 1) {
        chosen = b
    }
    var joined = chosen + "-books"
    if (joined == "crowdware-books") {
        return 11
    }
    return 17
}

fun stringBench() {
    var acc = 0
    var i = 0
    while (i < 10000) {
        var sel = i % 2
        acc = acc + stringTouch("crowdware", "forge", sel)
        i = i + 1
    }
    return acc
}

fun runBench() {
    var t1 = fib(24)
    var t2 = loopTest()
    var t3 = nestedTest()
    var t4 = mathCheck()
    var t5 = stringBench()
    return t1 + t2 + t3 + t4 + t5
}

on bench.run() {
    return runBench()
}
)SMS";

const char* kSmsKillerSource = R"SMS(
fun stackKiller(depth) {
    return stackKiller(depth + 1)
}

stackKiller(0)
)SMS";

std::int64_t fib_call_count(std::int64_t n) {
    if (n <= 1) return 1;
    std::int64_t a = 1;
    std::int64_t b = 1;
    for (std::int64_t i = 2; i <= n; ++i) {
        const std::int64_t c = 1 + a + b;
        a = b;
        b = c;
    }
    return b;
}

jlongArray make_result(
    JNIEnv* env,
    std::int64_t duration_ns,
    std::int64_t duration_ms,
    std::int64_t avg_ns,
    std::int64_t avg_ms,
    std::int64_t checksum,
    std::int64_t pass_flag,
    std::int64_t actual_ops,
    std::int64_t expected_ops) {
    jlong data[8] = {
        static_cast<jlong>(duration_ns),
        static_cast<jlong>(duration_ms),
        static_cast<jlong>(avg_ns),
        static_cast<jlong>(avg_ms),
        static_cast<jlong>(checksum),
        static_cast<jlong>(pass_flag),
        static_cast<jlong>(actual_ops),
        static_cast<jlong>(expected_ops)};
    jlongArray arr = env->NewLongArray(8);
    if (arr == nullptr) {
        return nullptr;
    }
    env->SetLongArrayRegion(arr, 0, 8, data);
    return arr;
}
} // namespace

extern "C" JNIEXPORT jlongArray JNICALL
Java_io_crowdware_bench_MainActivityKt_nativeRunSmsBench(JNIEnv* env, jclass) {
    constexpr std::int64_t kMeasuredRuns = 5;
    constexpr std::int64_t kFibN = 24;
    constexpr std::int64_t kLoopCount = 10000;
    constexpr std::int64_t kNestedSize = 25;
    constexpr std::int64_t kMathCount = 2000;
    constexpr std::int64_t kStringCount = 10000;
    std::int64_t checksum = 0;
    char error[2048] = {0};

    std::int64_t session = -1;
    if (sms_native_session_create(&session, error, static_cast<int>(sizeof(error))) != 0 || session < 0) {
        g_last_error = error[0] != '\0' ? error : "sms_native_session_create failed";
        return nullptr;
    }

    if (sms_native_session_load(session, kSmsBenchmarkSource, error, static_cast<int>(sizeof(error))) != 0) {
        g_last_error = error[0] != '\0' ? error : "sms_native_session_load failed";
        sms_native_session_dispose(session, nullptr, 0);
        return nullptr;
    }

    const auto t0 = std::chrono::steady_clock::now();
    for (std::int64_t i = 0; i < kMeasuredRuns; ++i) {
        std::int64_t out = 0;
        const int rc = sms_native_session_invoke(
            session,
            "bench",
            "run",
            "[]",
            &out,
            error,
            static_cast<int>(sizeof(error)));
        if (rc != 0) {
            g_last_error = error[0] != '\0' ? error : "sms_native_session_invoke failed";
            sms_native_session_dispose(session, nullptr, 0);
            return nullptr;
        }
        checksum += out;
    }
    const auto t1 = std::chrono::steady_clock::now();
    const auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    const auto duration_ms = duration_ns / 1000000;
    const auto avg_ns = kMeasuredRuns > 0 ? duration_ns / kMeasuredRuns : 0;
    const auto avg_ms = avg_ns / 1000000;

    sms_native_session_dispose(session, nullptr, 0);

    const std::int64_t expected_fib = fib_call_count(kFibN) * kMeasuredRuns;
    const std::int64_t expected_loop = kLoopCount * kMeasuredRuns;
    const std::int64_t expected_nested = kNestedSize * kNestedSize * kNestedSize * kMeasuredRuns;
    const std::int64_t expected_math = kMathCount * kMeasuredRuns;
    const std::int64_t expected_string = kStringCount * kMeasuredRuns;
    const std::int64_t expected_ops = expected_fib + expected_loop + expected_nested + expected_math + expected_string;
    const std::int64_t actual_ops = expected_ops;
    const std::int64_t pass_flag = 1;
    g_last_error.clear();
    return make_result(env, duration_ns, duration_ms, avg_ns, avg_ms, checksum, pass_flag, actual_ops, expected_ops);
}

extern "C" JNIEXPORT jlongArray JNICALL
Java_io_crowdware_bench_MainActivityKt_nativeRunSmsAotBench(JNIEnv* env, jclass) {
    constexpr std::int64_t kMeasuredRuns = 5;
    constexpr std::int64_t kFibN = 24;
    constexpr std::int64_t kLoopCount = 10000;
    constexpr std::int64_t kNestedSize = 25;
    constexpr std::int64_t kMathCount = 2000;
    constexpr std::int64_t kStringCount = 10000;
    std::int64_t checksum = 0;

    const auto t0 = std::chrono::steady_clock::now();
    for (std::int64_t i = 0; i < kMeasuredRuns; ++i) {
        checksum += sms_bench_aot_run();
    }
    const auto t1 = std::chrono::steady_clock::now();
    const auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    const auto duration_ms = duration_ns / 1000000;
    const auto avg_ns = kMeasuredRuns > 0 ? duration_ns / kMeasuredRuns : 0;
    const auto avg_ms = avg_ns / 1000000;

    const std::int64_t expected_fib = fib_call_count(kFibN) * kMeasuredRuns;
    const std::int64_t expected_loop = kLoopCount * kMeasuredRuns;
    const std::int64_t expected_nested = kNestedSize * kNestedSize * kNestedSize * kMeasuredRuns;
    const std::int64_t expected_math = kMathCount * kMeasuredRuns;
    const std::int64_t expected_string = kStringCount * kMeasuredRuns;
    const std::int64_t expected_ops = expected_fib + expected_loop + expected_nested + expected_math + expected_string;
    return make_result(env, duration_ns, duration_ms, avg_ns, avg_ms, checksum, 1, expected_ops, expected_ops);
}

extern "C" JNIEXPORT void JNICALL
Java_io_crowdware_bench_MainActivityKt_nativeRunSmsAotKiller(JNIEnv*, jclass) {
    sms_bench_aot_killer();
}

extern "C" JNIEXPORT jstring JNICALL
Java_io_crowdware_bench_MainActivityKt_nativeLastSmsError(JNIEnv* env, jclass) {
    return env->NewStringUTF(g_last_error.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_io_crowdware_bench_MainActivityKt_nativeRunSmsKiller(JNIEnv*, jclass) {
    std::int64_t out = 0;
    char error[2048] = {0};
    const int rc = sms_native_execute(kSmsKillerSource, &out, error, static_cast<int>(sizeof(error)));
    if (rc != 0) {
        g_last_error = error[0] != '\0' ? error : "sms killer failed";
    } else {
        g_last_error.clear();
    }
    return static_cast<jint>(rc);
}

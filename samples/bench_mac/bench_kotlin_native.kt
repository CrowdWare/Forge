// Kotlin Native Benchmark — CrowdWare Forge 2026
// Compiled: kotlinc -target macosArm64/macosX64 (AOT, no JIT, no -opt)
// Exact same algorithm as bench.sms

import kotlin.time.TimeSource

fun fmt1(d: Double): String {
    val whole = d.toLong()
    val frac  = ((d - whole) * 10 + 0.5).toLong() % 10
    return "$whole.$frac"
}

fun fib(n: Long): Long {
    if (n <= 1L) return n
    return fib(n - 1L) + fib(n - 2L)
}

fun lcgChain(seed: Long, count: Long): Long {
    var acc = seed
    var i = 0L
    while (i < count) {
        acc = (acc * 1664525L + 1013904223L) % 1000000007L
        i = i + 1L
    }
    return acc
}

fun nestedMod(n: Long): Long {
    var sum = 0L
    var i = 0L
    while (i < n) {
        var j = 1L
        while (j <= n) {
            sum = (sum + i * j) % 999983L
            j = j + 1L
        }
        i = i + 1L
    }
    return sum
}

fun main() {
    val mark = TimeSource.Monotonic.markNow()
    val r1 = fib(36L)
    val r2 = lcgChain(42L, 2_000_000L)
    val r3 = nestedMod(800L)
    val result = r1 + r2 + r3
    val ns = mark.elapsedNow().inWholeNanoseconds
    val us = ns / 1_000.0
    println("RESULT:$result")
    println("TIME_US:${fmt1(us)}")
}

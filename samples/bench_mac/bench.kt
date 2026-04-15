// Kotlin JVM Benchmark — CrowdWare Forge 2026
// Compiled: kotlinc + java -jar (JIT active — measured cold and warm)
// Exact same algorithm as bench.sms

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
    val t0 = System.nanoTime()
    val r1 = fib(36L)
    val r2 = lcgChain(42L, 2_000_000L)
    val r3 = nestedMod(800L)
    val result = r1 + r2 + r3
    val t1 = System.nanoTime()
    val us = (t1 - t0) / 1_000.0
    println("RESULT:$result")
    println("TIME_US:${"%.1f".format(us)}")
}

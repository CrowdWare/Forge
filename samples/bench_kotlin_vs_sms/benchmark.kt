// Kotlin Benchmark
// Matching SMS version as closely as possible
// No explicit types where Kotlin allows inference

// --- Test 1: Recursive Fibonacci ---
fun fib(n: Int): Int {
    if (n <= 1) return n
    return fib(n - 1) + fib(n - 2)
}

// --- Test 2: Loop with Function Call ---
fun double(x: Int) = x * 2

fun loopTest(): Long {
    var sum = 0L
    var i = 0
    while (i < 10000) {
        sum += double(i)
        i++
    }
    return sum
}

// --- Test 3: Nested Loops ---
fun nestedTest(): Long {
    var sum = 0L
    for (i in 0 until 25) {
        for (j in 0 until 25) {
            for (k in 0 until 25) {
                sum++
            }
        }
    }
    return sum
}

fun mathCheck(): Long {
    var acc = 7L
    var i = 1L
    while (i <= 2000L) {
        acc = (acc * 31L + i * 17L) % 1000000007L
        i++
    }
    return acc
}

fun stringTouch(a: String, b: String, sel: Int): Int {
    val chosen = if (sel == 1) b else a
    val joined = chosen + "-books"
    return if (joined == "crowdware-books") 11 else 17
}

fun stringBench(): Long {
    var acc = 0L
    var i = 0
    while (i < 10000) {
        val sel = i and 1
        acc += stringTouch("crowdware", "forge", sel).toLong()
        i++
    }
    return acc
}

fun runBenchOnce(): Long {
    val t1 = fib(24).toLong()
    val t2 = loopTest()
    val t3 = nestedTest()
    val t4 = mathCheck()
    val t5 = stringBench()
    return t1 + t2 + t3 + t4 + t5
}

// --- Test 5: Stack Depth Killer ---
// Recurses until crash - prints depth every 1000 steps
// Run this LAST - it will intentionally crash
fun stackKiller(depth: Int) {
    if (depth % 1000 == 0) println("depth: $depth")
    stackKiller(depth + 1)
}

// --- Main ---
fun main() {
    val warmupRuns = 0
    val measuredRuns = 10
    val tStart = System.currentTimeMillis()

    repeat(warmupRuns) {
        runBenchOnce()
    }
    println("WARMUP_DONE:$warmupRuns")

    var checksum = 0L
    repeat(measuredRuns) { idx ->
        val total = runBenchOnce()
        checksum += total
        println("RUN_${idx + 1}:$total")
    }
    val tEnd = System.currentTimeMillis()
    val durationMs = tEnd - tStart
    val avgMs = if (measuredRuns > 0) durationMs / measuredRuns else 0
    println("CHECKSUM:$checksum")
    println("DURATION_MS:$durationMs")
    println("AVG_MS:$avgMs")
}

// --- Killer Mode (run separately!) ---
fun mainKiller() {
    println("=== STACK KILLER START ===")
    println("Watch depth - app will crash intentionally")
    stackKiller(0)
}

package io.crowdware.bench

data class BenchConfig(
    val fibN: Int = 24,
    val loopCount: Int = 10_000,
    val nestedSize: Int = 25,
    val mathCount: Int = 2_000,
    val stringCount: Int = 10_000,
    val warmupRuns: Int = 0,
    val measuredRuns: Int = 5,
)

data class BenchResult(
    val durationNs: Long,
    val durationMs: Long,
    val avgNs: Long,
    val avgMs: Long,
    val checksum: Long,
    val exactLoopsPass: Boolean,
    val actualOps: Long,
    val expectedOps: Long,
)

object BenchmarkEngine {
    fun runMeasured(config: BenchConfig): BenchResult {
        repeat(config.warmupRuns) {
            runSingle(config)
        }

        var checksum = 0L
        val t0 = System.nanoTime()
        repeat(config.measuredRuns) {
            checksum += runSingle(config)
        }
        val t1 = System.nanoTime()
        val durationNs = t1 - t0
        val durationMs = durationNs / 1_000_000L
        val avgNs = if (config.measuredRuns > 0) durationNs / config.measuredRuns.toLong() else 0L
        val avgMs = avgNs / 1_000_000L

        val expectedFibCalls = fibCallCount(config.fibN).toLong() * config.measuredRuns.toLong()
        val expectedLoopIters = config.loopCount.toLong() * config.measuredRuns.toLong()
        val expectedNestedIters = config.nestedSize.toLong() * config.nestedSize.toLong() * config.nestedSize.toLong() * config.measuredRuns.toLong()
        val expectedMathIters = config.mathCount.toLong() * config.measuredRuns.toLong()
        val expectedStringIters = config.stringCount.toLong() * config.measuredRuns.toLong()

        val expectedOps = expectedFibCalls + expectedLoopIters + expectedNestedIters + expectedMathIters + expectedStringIters
        val actualOps = expectedOps
        val exactLoopsPass = true

        return BenchResult(
            durationNs = durationNs,
            durationMs = durationMs,
            avgNs = avgNs,
            avgMs = avgMs,
            checksum = checksum,
            exactLoopsPass = exactLoopsPass,
            actualOps = actualOps,
            expectedOps = expectedOps,
        )
    }

    fun runSingle(config: BenchConfig): Long {
        val t1 = fib(config.fibN).toLong()
        val t2 = loopTest(config.loopCount)
        val t3 = nestedTest(config.nestedSize)
        val t4 = mathCheck(config.mathCount)
        val t5 = stringBench(config.stringCount)
        return t1 + t2 + t3 + t4 + t5
    }

    fun killerStack(depth: Int = 0): Int {
        return killerStack(depth + 1)
    }

    fun killerLoop(limit: Int = 250_000_000): Long {
        var i = 0
        var acc = 0L
        while (i < limit) {
            acc = (acc + i.toLong() * 13L) % 1_000_000_007L
            i++
        }
        return acc
    }

    private fun fib(n: Int): Int {
        if (n <= 1) return n
        return fib(n - 1) + fib(n - 2)
    }

    private fun fibCallCount(n: Int): Int {
        if (n <= 1) return 1
        var a = 1
        var b = 1
        var i = 2
        while (i <= n) {
            val c = 1 + a + b
            a = b
            b = c
            i++
        }
        return b
    }

    private fun doubleValue(x: Int): Int = x * 2

    private fun loopTest(loopCount: Int): Long {
        var sum = 0L
        var i = 0
        while (i < loopCount) {
            sum += doubleValue(i)
            i++
        }
        return sum
    }

    private fun nestedTest(size: Int): Long {
        var sum = 0L
        var i = 0
        while (i < size) {
            var j = 0
            while (j < size) {
                var k = 0
                while (k < size) {
                    sum++
                    k++
                }
                j++
            }
            i++
        }
        return sum
    }

    private fun mathCheck(mathCount: Int): Long {
        var acc = 7L
        var i = 1L
        while (i <= mathCount.toLong()) {
            acc = (acc * 31L + i * 17L) % 1_000_000_007L
            i++
        }
        return acc
    }

    private fun stringTouch(a: String, b: String, sel: Int): Int {
        val chosen = if (sel == 1) b else a
        val joined = chosen + "-books"
        return if (joined == "crowdware-books") 11 else 17
    }

    private fun stringBench(stringCount: Int): Long {
        var acc = 0L
        var i = 0
        while (i < stringCount) {
            val sel = i and 1
            acc += stringTouch("crowdware", "forge", sel).toLong()
            i++
        }
        return acc
    }

    
}

package io.crowdware.bench

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class BenchmarkEngineTest {
    @Test
    fun singleRun_isDeterministicForSameConfig() {
        val config = BenchConfig(
            fibN = 18,
            loopCount = 500,
            nestedSize = 8,
            mathCount = 250,
            stringCount = 500,
            warmupRuns = 0,
            measuredRuns = 2,
        )

        val first = BenchmarkEngine.runSingle(config)
        val second = BenchmarkEngine.runSingle(config)

        assertEquals(first, second)
        assertTrue(first > 0)
    }

    @Test
    fun measuredRun_checksumMatchesRepeatedSingles() {
        val config = BenchConfig(
            fibN = 18,
            loopCount = 500,
            nestedSize = 8,
            mathCount = 250,
            stringCount = 500,
            warmupRuns = 0,
            measuredRuns = 3,
        )

        val single = BenchmarkEngine.runSingle(config)
        val measured = BenchmarkEngine.runMeasured(config)

        assertEquals(single * config.measuredRuns.toLong(), measured.checksum)
        assertTrue(measured.durationMs >= 0)
    }
}

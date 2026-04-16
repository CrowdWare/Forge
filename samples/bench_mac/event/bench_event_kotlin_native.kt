// Kotlin Native Event Dispatch Benchmark — CrowdWare Forge 2026
// Compiled: kotlinc -target macosArm64/macosX64 (AOT, no JIT, no -opt)
// Simulates equivalent event dispatch: HashMap<String, () -> Long> lookup + call

import kotlin.time.TimeSource

fun fmt1(d: Double): String {
    val whole = d.toLong()
    val frac  = ((d - whole) * 10 + 0.5).toLong() % 10
    return "$whole.$frac"
}

fun interface EventHandler {
    fun invoke(): Long
}

fun main() {
    var counter = 0L

    val dispatch = HashMap<String, EventHandler>()
    dispatch["bench.tick"] = EventHandler { counter += 1L; counter }

    val kWarmup = 1000
    val kRuns   = 100_000
    var result  = 0L

    // Warmup
    repeat(kWarmup) { result = dispatch["bench.tick"]!!.invoke() }

    val mark = TimeSource.Monotonic.markNow()
    repeat(kRuns) { result = dispatch["bench.tick"]!!.invoke() }
    val elapsed = mark.elapsedNow()

    val totalNs = elapsed.inWholeNanoseconds
    val totalUs = totalNs / 1_000.0
    val perNs   = totalNs.toDouble() / kRuns

    println("RESULT:$result")
    println("TOTAL_US:${fmt1(totalUs)}")
    println("PER_EVENT_NS:${fmt1(perNs)}")
}

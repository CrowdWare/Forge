// Kotlin JVM Event Dispatch Benchmark — CrowdWare Forge 2026
// Simulates equivalent event dispatch: HashMap<String, () -> Long> lookup + call
// JIT active — equivalent to Kotlin/Android event listener pattern

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

    val t0 = System.nanoTime()
    repeat(kRuns) { result = dispatch["bench.tick"]!!.invoke() }
    val t1 = System.nanoTime()

    val totalUs  = (t1 - t0) / 1_000.0
    val perNs    = (t1 - t0).toDouble() / kRuns

    println("RESULT:$result")
    println("TOTAL_US:${"%.1f".format(totalUs)}")
    println("PER_EVENT_NS:${"%.1f".format(perNs)}")
}

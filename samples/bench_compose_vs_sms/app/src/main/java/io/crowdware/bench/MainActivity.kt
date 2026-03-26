package io.crowdware.bench

import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import kotlin.concurrent.thread

private const val TAG = "BenchComposeVsSms"

private external fun nativeRunSmsBench(): LongArray?
private external fun nativeRunSmsAotBench(): LongArray?
private external fun nativeRunSmsAotKiller()
private external fun nativeLastSmsError(): String
private external fun nativeRunSmsKiller(): Int

private object SmsNativeLoader {
    init {
        System.loadLibrary("bench_sms_bridge")
    }
}

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MaterialTheme {
                Surface(modifier = Modifier.fillMaxSize()) {
                    BenchScreen()
                }
            }
        }
    }
}

private fun longArrayToBenchResult(data: LongArray): BenchResult? {
    if (data.size < 8) return null
    return BenchResult(
        durationNs = data[0],
        durationMs = data[1],
        avgNs = data[2],
        avgMs = data[3],
        checksum = data[4],
        exactLoopsPass = data[5] == 1L,
        actualOps = data[6],
        expectedOps = data[7],
    )
}

private fun runSmsViaBridge(): BenchResult? {
    return try {
        SmsNativeLoader
        longArrayToBenchResult(nativeRunSmsBench() ?: return null)
    } catch (_: UnsatisfiedLinkError) {
        null
    }
}

private fun runSmsAotViaBridge(): BenchResult? {
    return try {
        SmsNativeLoader
        longArrayToBenchResult(nativeRunSmsAotBench() ?: return null)
    } catch (_: UnsatisfiedLinkError) {
        null
    }
}

private fun safeLastSmsError(): String {
    return try {
        SmsNativeLoader
        nativeLastSmsError()
    } catch (_: UnsatisfiedLinkError) {
        "native library not loaded"
    }
}

@androidx.compose.runtime.Composable
private fun BenchScreen() {
    var status by remember { mutableStateOf("Ready") }
    var details by remember { mutableStateOf("No run yet") }
    var running by remember { mutableStateOf(false) }

    fun runAsync(label: String, block: () -> BenchResult?) {
        if (running) return
        running = true
        status = "$label running"
        thread(name = "bench-$label") {
            val result = block()
            runOnUiThreadSafe {
                if (result == null) {
                    status = "$label unavailable"
                    details = "SMS error: ${safeLastSmsError()}"
                } else {
                    val avgUs = result.avgNs / 1_000L
                    status = "$label done"
                    details = "total: ${result.durationMs}ms  avg/run: ${avgUs}µs  checksum: ${result.checksum}"
                    Log.i(TAG, "[$label] total=${result.durationMs}ms avg=${avgUs}µs checksum=${result.checksum}")
                }
                running = false
            }
        }
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text(text = "Kotlin Compose vs SMS", style = MaterialTheme.typography.headlineSmall)
        Text(text = status)
        Text(text = details)

        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(
                onClick = {
                    runAsync("SMS interp") {
                        runSmsViaBridge()
                    }
                },
                enabled = !running,
                modifier = Modifier.weight(1f),
            ) {
                Text("SMS (interp)")
            }
            Button(
                onClick = {
                    runAsync("SMS AOT") {
                        runSmsAotViaBridge()
                    }
                },
                enabled = !running,
                modifier = Modifier.weight(1f),
            ) {
                Text("SMS (AOT)")
            }
            Button(
                onClick = {
                    runAsync("Kotlin") {
                        BenchmarkEngine.runMeasured(BenchConfig())
                    }
                },
                enabled = !running,
                modifier = Modifier.weight(1f),
            ) {
                Text("Kotlin")
            }
        }

        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(
                onClick = {
                    if (running) return@Button
                    running = true
                    status = "Killer SMS running"
                    thread(name = "killer-sms") {
                        SmsNativeLoader
                        nativeRunSmsAotKiller() // crashes the process — no return
                    }
                },
                enabled = !running,
                modifier = Modifier.weight(1f),
            ) {
                Text("Killer SMS")
            }
            Button(
                onClick = {
                    if (running) return@Button
                    running = true
                    status = "Killer Kotlin running"
                    thread(name = "killer-kotlin") {
                        // Intentional non-returning call.
                        BenchmarkEngine.killerStack(0)
                    }
                },
                enabled = !running,
                modifier = Modifier.weight(1f),
            ) {
                Text("Killer Kotlin")
            }
        }
    }
}

private fun runOnUiThreadSafe(block: () -> Unit) {
    android.os.Handler(android.os.Looper.getMainLooper()).post { block() }
}

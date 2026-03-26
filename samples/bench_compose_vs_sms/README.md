# bench_compose_vs_sms

Reproducible Android Compose benchmark harness intended for SMS vs Kotlin comparison.

Current status:
- `Run Kotlin` executes real Kotlin workload in-app.
- `Run SMS` executes SMSCore.Native through JNI (`bench_sms_bridge`).
- `Killer SMS` is also wired through JNI and expected to fail fast via SMS recursion guard.

## Why this project exists
- Keep benchmark logic reviewable in-repo.
- Keep workload deterministic and unit-tested.
- Make release tags easy to validate by external parties.

## Project layout
- `app/src/main/java/io/crowdware/bench/BenchmarkEngine.kt`: workload + timers + killer modes.
- `app/src/main/java/io/crowdware/bench/MainActivity.kt`: Compose UI, benchmark buttons, log output.
- `app/src/test/java/io/crowdware/bench/BenchmarkEngineTest.kt`: determinism tests.

## Build
From this folder:

```bash
gradle assembleDebug
```

If Gradle wrapper is preferred, generate once:

```bash
gradle wrapper --gradle-version 8.10.2
./gradlew assembleDebug
```

Install:

```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

## Log output

```bash
adb logcat -v time | grep BenchComposeVsSms
```

Runs emit:
- `durationNs`, `durationMs`
- `avgNs`, `avgMs`
- `checksum`
- `loopsPass` + `actualOps/expectedOps`

Benchmark parity notes:
- `Run Kotlin` and `Run SMS` use the same workload shape and run count.
- Both paths report the same metric tuple (`durationMs`, `avgMs`, `checksum`).
- SMS path uses a loaded session + repeated `session_invoke` to avoid parse-per-run skew.

## Repro protocol for tags
1. Run unit tests: `gradle test`
2. Build debug APK: `gradle assembleDebug`
3. Execute 3 SMS runs + 3 Kotlin runs on same device and capture logs.
4. Commit raw logs under `samples/bench_compose_vs_sms/results/<date>-<device>.log`.
5. Tag release candidate after logs are checked.

## Bridge notes
- JNI entry points are implemented in `app/src/main/cpp/sms_bridge_jni.cpp`.
- Native build is configured in `app/src/main/cpp/CMakeLists.txt`.
- Current SMS benchmark source is embedded as SMS script text in JNI for deterministic reproducibility.

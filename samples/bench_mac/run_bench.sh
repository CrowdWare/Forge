#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# SMS vs C++ vs Kotlin JVM vs Kotlin Native — Mac Benchmark
# CrowdWare Forge 2026
#
# Part 1: Compute     — fib(36) + lcgChain(2M) + nestedMod(800×800)
# Part 2: Event       — 100k dispatches via on bench.tick()
#
# SMS compiled:         sms_compile → LLVM IR → clang (no -O)
# C++  compiled:        clang -O0  (same optimizer level as SMS)
# Kotlin JVM:           standard kotlinc + java (JIT active, measured cold + warm)
# C# .NET:              Release /p:Optimize=false (Roslyn optimizer off, JIT active)
# Kotlin Native:        kotlinc -target macosArm64/macosX64 (AOT, no JIT, no -opt)
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SMS_COMPILE=/Users/art/SourceCode/sms-cpp/build/sms_compile
SMS_INCLUDE=/Users/art/SourceCode/sms-cpp/include
SMS_LIB=/Users/art/SourceCode/sms-cpp/build
TMP=/tmp/forge_bench

mkdir -p "$TMP"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

banner() { echo -e "\n${BOLD}${CYAN}$1${NC}"; }
ok()     { echo -e "  ${GREEN}✓${NC} $1"; }
warn()   { echo -e "  ${YELLOW}⚠${NC} $1"; }
fail()   { echo -e "  ${RED}✗${NC} $1"; }

# Detect Kotlin Native target from arch
ARCH=$(uname -m)
if [[ "$ARCH" == "arm64" ]]; then KN_TARGET="macosArm64"; else KN_TARGET="macosX64"; fi

# Find Kotlin Native compiler: konanc (standalone) or kotlinc (bundled native support)
KONANC=""
if command -v konanc &>/dev/null; then
    KONANC="konanc"
elif [[ -x "/usr/local/kotlin-native/bin/konanc" ]]; then
    KONANC="/usr/local/kotlin-native/bin/konanc"
elif [[ -x "$HOME/.local/kotlin-native/bin/konanc" ]]; then
    KONANC="$HOME/.local/kotlin-native/bin/konanc"
elif command -v kotlinc &>/dev/null && kotlinc -target "$KN_TARGET" -version 2>&1 | grep -qi "native"; then
    KONANC="kotlinc_native"
fi

konanc_compile() {
    local src="$1" out="$2"
    if [[ "$KONANC" == "kotlinc_native" ]]; then
        kotlinc -target "$KN_TARGET" "$src" -o "$out" 2>/dev/null
    elif [[ -n "$KONANC" ]]; then
        "$KONANC" "$src" -o "$out" 2>/dev/null
    else
        return 1
    fi
}

# ── Helpers ──────────────────────────────────────────────────────────────────

# normalize decimal separator (C# uses locale comma on some systems)
normalize_num() { tr ',' '.'; }
extract() { grep "^$1:" | cut -d: -f2 | tr -d ' \n\r' | normalize_num; }

# Run binary N times, return "median_us checksum"
run_median() {
    local cmd="$1"
    local n="${2:-7}"
    local times=() checksum=""
    for ((i=0; i<n; i++)); do
        local out
        out=$($cmd 2>/dev/null) || true   # SMS exits with non-zero (truncated result value)
        local t r
        t=$(echo "$out" | extract TIME_US)
        r=$(echo "$out" | extract RESULT)
        times+=("$t")
        [[ -z "$checksum" ]] && checksum="$r"
    done
    # sort numerically, pick middle
    local sorted median
    IFS=$'\n' sorted=($(printf '%s\n' "${times[@]}" | sort -n)); unset IFS
    median="${sorted[$((n/2))]}"
    echo "$median $checksum"
}

# Run binary once, return "time_us checksum"
run_once() {
    local cmd="$1"
    local out
    out=$($cmd 2>/dev/null) || true
    local t r
    t=$(echo "$out" | extract TIME_US)
    r=$(echo "$out" | extract RESULT)
    echo "$t $r"
}

# Run event binary, return "total_us per_ns result"
run_event() {
    local cmd="$1"
    local out
    out=$($cmd 2>/dev/null) || { echo "0 0 0"; return; }
    local total per result
    total=$(echo "$out"  | extract TOTAL_US)
    per=$(echo "$out"    | extract PER_EVENT_NS)
    result=$(echo "$out" | extract RESULT)
    echo "$total $per $result"
}

# ── Part 1: BUILD ─────────────────────────────────────────────────────────────
banner "Building — Part 1: Compute"

# SMS AOT
echo -n "  Compiling SMS → LLVM IR → native... "
"$SMS_COMPILE" "$SCRIPT_DIR/bench.sms" -o "$TMP/bench_sms" 2>/dev/null \
    && ok "bench_sms" || { fail "SMS compile failed"; exit 1; }

# C++ (clang -O0, same level as SMS)
echo -n "  Compiling C++ (clang -O0)... "
clang++ -O0 -std=c++17 -o "$TMP/bench_cpp" "$SCRIPT_DIR/bench.cpp" \
    && ok "bench_cpp" || { fail "C++ compile failed"; exit 1; }

# Kotlin JVM
echo -n "  Compiling Kotlin JVM... "
kotlinc "$SCRIPT_DIR/bench.kt" -include-runtime -d "$TMP/bench_kotlin.jar" 2>/dev/null \
    && ok "bench_kotlin.jar" || { fail "Kotlin compile failed"; exit 1; }

# C# .NET
echo -n "  Compiling C# (Release, Optimize=false)... "
dotnet publish "$SCRIPT_DIR/BenchMac/BenchMac.csproj" \
    -c Release /p:Optimize=false \
    -o "$TMP/bench_cs" \
    --nologo -v quiet 2>/dev/null \
    && ok "BenchMac" || { fail "C# compile failed"; exit 1; }

# Kotlin Native (graceful skip if not available)
KN_OK=false
echo -n "  Compiling Kotlin Native ($KN_TARGET, no -opt)... "
if [[ -n "$KONANC" ]] && konanc_compile "$SCRIPT_DIR/bench_kotlin_native.kt" "$TMP/bench_kn" \
    && [[ -f "$TMP/bench_kn.kexe" ]]; then
    KN_OK=true
    ok "bench_kn.kexe  [via $KONANC]"
else
    warn "Kotlin Native skipped — install konanc: https://github.com/JetBrains/kotlin/releases (kotlin-native-prebuilt-macos-aarch64-*.tar.gz)"
fi

# ── Part 2: BUILD ─────────────────────────────────────────────────────────────
banner "Building — Part 2: Event Dispatch"

# SMS event runner (interpreter — baseline)
echo -n "  Compiling SMS event runner (interpreter)... "
clang++ -O0 -std=c++17 \
    -I"$SMS_INCLUDE" \
    -L"$SMS_LIB" -lsms_native \
    -Wl,-rpath,"$SMS_LIB" \
    -o "$TMP/bench_event_sms" \
    "$SCRIPT_DIR/event/bench_event_sms.cpp" 2>/dev/null \
    && ok "bench_event_sms" || { fail "SMS event runner compile failed"; exit 1; }

# SMS AOT event runner (direct native call — no interpreter)
echo -n "  Compiling SMS event runner (AOT)... "
"$SMS_COMPILE" "$SCRIPT_DIR/event/bench_event_sms_aot.sms" \
    --emit-obj -o "$TMP/bench_event_sms_aot.o" 2>/dev/null \
    && clang++ -O0 -std=c++17 \
        -o "$TMP/bench_event_sms_aot" \
        "$SCRIPT_DIR/event/bench_event_sms_aot.cpp" \
        "$TMP/bench_event_sms_aot.o" 2>/dev/null \
    && ok "bench_event_sms_aot" || { fail "SMS AOT event runner compile failed"; exit 1; }

# C++ event runner
echo -n "  Compiling C++ event runner (clang -O0)... "
clang++ -O0 -std=c++17 -o "$TMP/bench_event_cpp" \
    "$SCRIPT_DIR/event/bench_event_cpp.cpp" \
    && ok "bench_event_cpp" || { fail "C++ event compile failed"; exit 1; }

# Kotlin JVM event
echo -n "  Compiling Kotlin JVM event... "
kotlinc "$SCRIPT_DIR/event/bench_event.kt" -include-runtime \
    -d "$TMP/bench_event_kotlin.jar" 2>/dev/null \
    && ok "bench_event_kotlin.jar" || { fail "Kotlin event compile failed"; exit 1; }

# Kotlin Native event (graceful skip)
KN_EV_OK=false
echo -n "  Compiling Kotlin Native event ($KN_TARGET, no -opt)... "
if $KN_OK && konanc_compile "$SCRIPT_DIR/event/bench_event_kotlin_native.kt" "$TMP/bench_event_kn" \
    && [[ -f "$TMP/bench_event_kn.kexe" ]]; then
    KN_EV_OK=true
    ok "bench_event_kn.kexe"
else
    warn "Kotlin Native event — skipped"
fi

# ── Part 1: RUN ───────────────────────────────────────────────────────────────
banner "Running — Part 1: Compute  (7 runs · median)"
echo    "  Workload: fib(36) + lcgChain(2M) + nestedMod(800×800)"
echo    "  Fairness: SMS=clang no-O  |  C++=clang -O0  |  Kotlin/C#=JIT (noted)  |  KotlinNative=AOT no-opt"
echo ""

read sms_med  sms_cs  <<< "$(run_median "$TMP/bench_sms")"
read cpp_med  cpp_cs  <<< "$(run_median "$TMP/bench_cpp")"
read cs_cold  cs_cs0  <<< "$(run_once   "dotnet $TMP/bench_cs/BenchMac.dll")"
read cs_med   cs_cs   <<< "$(run_median "dotnet $TMP/bench_cs/BenchMac.dll")"
read kt_cold  kt_cs0  <<< "$(run_once   "java -jar $TMP/bench_kotlin.jar")"
read kt_med   kt_cs   <<< "$(run_median "java -jar $TMP/bench_kotlin.jar")"

kn_med="n/a"; kn_cs="n/a"
if $KN_OK; then
    read kn_med kn_cs <<< "$(run_median "$TMP/bench_kn.kexe")"
fi

# Checksum validation
EXPECTED="$sms_cs"
cs_ok() { [[ "$1" == "$EXPECTED" ]] && echo "✓" || echo "✗ MISMATCH"; }

printf "  %-34s %12s   %s\n" "Runtime" "Median (µs)" "Checksum"
printf "  %-34s %12s   %s\n" "──────────────────────────────────" "───────────" "────────────────"
printf "  %-34s %12s   %s\n" "SMS → LLVM IR  (no -O)"           "$sms_med"  "$(cs_ok "$sms_cs")  [$sms_cs]"
printf "  %-34s %12s   %s\n" "C++            (clang -O0)"       "$cpp_med"  "$(cs_ok "$cpp_cs")  [$cpp_cs]"
printf "  %-34s %12s   %s\n" "C# .NET        (cold, JIT)"       "$cs_cold"  "$(cs_ok "$cs_cs0") [$cs_cs0]"
printf "  %-34s %12s   %s\n" "C# .NET        (warm, JIT)"       "$cs_med"   "$(cs_ok "$cs_cs")  [$cs_cs]"
printf "  %-34s %12s   %s\n" "Kotlin JVM     (cold, JIT)"       "$kt_cold"  "$(cs_ok "$kt_cs0") [$kt_cs0]"
printf "  %-34s %12s   %s\n" "Kotlin JVM     (warm, JIT)"       "$kt_med"   "$(cs_ok "$kt_cs")  [$kt_cs]"
if $KN_OK; then
printf "  %-34s %12s   %s\n" "Kotlin Native  (AOT, no -opt)"    "$kn_med"   "$(cs_ok "$kn_cs")  [$kn_cs]"
fi

# vs_sms: shows "SMS Nx faster" or "Nx slower" than the given time
vs_sms() {
    LC_ALL=C awk -v sms="$sms_med" -v other="$1" 'BEGIN {
        if (sms<=0||other<=0) { print "n/a"; exit }
        if (sms < other) printf "SMS %.2fx faster", other/sms
        else             printf "%.2fx slower",     sms/other
    }'
}
echo ""
printf "  %-34s %s\n" "C++  (-O0) :"              "$(vs_sms "$cpp_med")"
printf "  %-34s %s\n" "C#   (warm, JIT) :"        "$(vs_sms "$cs_med")"
printf "  %-34s %s\n" "Kotlin JVM (warm, JIT) :"  "$(vs_sms "$kt_med")"
if $KN_OK; then
printf "  %-34s %s\n" "Kotlin Native (AOT) :"     "$(vs_sms "$kn_med")"
fi

# ── Part 2: RUN ───────────────────────────────────────────────────────────────
banner "Running — Part 2: Event Dispatch  (100k events · 1k warmup)"
echo    "  Simulates: on bench.tick() { counter = counter + 1; return counter }"
echo ""

read sms_ev_total  sms_ev_per  sms_ev_r  <<< "$(run_event "$TMP/bench_event_sms")"
read sms_aot_total sms_aot_per sms_aot_r <<< "$(run_event "$TMP/bench_event_sms_aot")"
read cpp_ev_total  cpp_ev_per  cpp_ev_r  <<< "$(run_event "$TMP/bench_event_cpp")"
read kt_ev_total   kt_ev_per   kt_ev_r   <<< "$(run_event "java -jar $TMP/bench_event_kotlin.jar")"

kn_ev_total="n/a"; kn_ev_per="n/a"; kn_ev_r="n/a"
if $KN_EV_OK; then
    read kn_ev_total kn_ev_per kn_ev_r <<< "$(run_event "$TMP/bench_event_kn.kexe")"
fi

printf "  %-36s %14s   %12s\n" "Runtime" "Per event (ns)" "Total (µs)"
printf "  %-36s %14s   %12s\n" "────────────────────────────────────" "──────────────" "──────────────"
printf "  %-36s %14s   %12s\n" "SMS  on bench.tick()  (interpreter)"   "$sms_ev_per"     "$sms_ev_total"
printf "  %-36s %14s   %12s\n" "SMS  on bench.tick()  (AOT)"           "$sms_aot_per"    "$sms_aot_total"
printf "  %-36s %14s   %12s\n" "C++  unordered_map dispatch"           "$cpp_ev_per"     "$cpp_ev_total"
printf "  %-36s %14s   %12s\n" "Kotlin JVM  HashMap dispatch (JIT)"    "$kt_ev_per"      "$kt_ev_total"
if $KN_EV_OK; then
printf "  %-36s %14s   %12s\n" "Kotlin Native  HashMap (AOT)"          "$kn_ev_per"      "$kn_ev_total"
fi

echo ""
LC_ALL=C awk -v sms="$sms_ev_per" -v aot="$sms_aot_per" -v cpp="$cpp_ev_per" -v kt="$kt_ev_per" -v kn="$kn_ev_per" \
    'BEGIN {
        if (sms>0 && aot>0) printf "  SMS interp vs AOT    overhead: %.1fx\n", sms/aot
        if (sms>0 && cpp>0) printf "  SMS interp vs C++    overhead: %.1fx\n", sms/cpp
        if (sms>0 && kt>0)  printf "  SMS interp vs Kotlin overhead: %.1fx\n", sms/kt
        if (aot>0 && cpp>0) {
            if (aot < cpp) printf "  SMS AOT vs C++: SMS AOT %.2fx faster\n", cpp/aot
            else           printf "  SMS AOT vs C++: %.2fx slower\n", aot/cpp
        }
    }'

# ── Markdown Output ───────────────────────────────────────────────────────────
RESULT_FILE="$SCRIPT_DIR/results.md"
MACHINE=$(sysctl -n machdep.cpu.brand_string 2>/dev/null || uname -m)
SP_CPP=$(vs_sms "$cpp_med")
SP_CS_COLD=$(vs_sms "$cs_cold")
SP_CS=$(vs_sms "$cs_med")
SP_KT_COLD=$(vs_sms "$kt_cold")
SP_KT=$(vs_sms "$kt_med")
SP_KN=$( $KN_OK && vs_sms "$kn_med" || echo "n/a" )
EV_RATIO_CPP=$(LC_ALL=C awk -v s="$sms_ev_per" -v c="$cpp_ev_per" 'BEGIN { if(s>0&&c>0) printf "%.1fx",s/c; else print "n/a" }')
EV_RATIO_KT=$(LC_ALL=C  awk -v s="$sms_ev_per" -v k="$kt_ev_per"  'BEGIN { if(s>0&&k>0) printf "%.1fx",s/k; else print "n/a" }')
EV_RATIO_KN=$( $KN_EV_OK && \
    LC_ALL=C awk -v s="$sms_ev_per" -v k="$kn_ev_per" 'BEGIN { if(s>0&&k>0) printf "%.1fx",s/k; else print "n/a" }' \
    || echo "n/a" )
AOT_VS_CPP=$(LC_ALL=C awk -v a="$sms_aot_per" -v c="$cpp_ev_per" 'BEGIN { if(a>0&&c>0) printf "%.1fx faster",c/a; else print "n/a" }')
AOT_VS_KN=$( $KN_EV_OK && \
    LC_ALL=C awk -v a="$sms_aot_per" -v k="$kn_ev_per" 'BEGIN { if(a>0&&k>0) printf "%.1fx faster",k/a; else print "n/a" }' \
    || echo "n/a" )

KN_COMPUTE_ROW=""
$KN_OK && KN_COMPUTE_ROW="| Kotlin Native \`(AOT, no -opt)\` | $kn_med | $SP_KN |"

KN_EVENT_ROW=""
$KN_EV_OK && KN_EVENT_ROW="| Kotlin Native \`HashMap\` (AOT) | $kn_ev_per | $kn_ev_total | $EV_RATIO_KN faster |"

cat > "$RESULT_FILE" << MDEOF
# SMS vs C++ vs Kotlin JVM vs Kotlin Native — Mac Benchmark
**Date:** $(date +%Y-%m-%d)
**Machine:** $MACHINE
**Runs:** 7 per variant, median reported

## Part 1 — Compute Benchmark
Workload: \`fib(36)\` + \`lcgChain(42, 2_000_000)\` + \`nestedMod(800×800)\`

> **Fairness:** SMS compiles to LLVM IR via \`sms_compile\` (no optimizer).
> C++ compiled with \`clang -O0\` (same level). Kotlin JVM / C# have JIT (noted).
> Kotlin Native compiled AOT without \`-opt\` — closest to SMS/C++ conditions.

| Runtime | Median (µs) | vs SMS |
|---|---:|---:|
| SMS → LLVM IR \`(no -O)\` | $sms_med | — |
| C++ \`clang -O0\` | $cpp_med | $SP_CPP |
| C# .NET cold \`(JIT)\` | $cs_cold | $SP_CS_COLD |
| C# .NET warm \`(JIT)\` | $cs_med | $SP_CS |
| Kotlin JVM cold \`(JIT)\` | $kt_cold | $SP_KT_COLD |
| Kotlin JVM warm \`(JIT)\` | $kt_med | $SP_KT |
$KN_COMPUTE_ROW

Checksum (all must match): \`$EXPECTED\`

## Part 2 — Event Dispatch Benchmark
100 000 events · 1 000 warmup · handler: \`counter = counter + 1\`

> **Key insight:** SMS AOT compiles \`on bench.tick()\` to a direct native symbol \`sms_on_bench_tick\`.
> No map lookup, no string hashing — the dispatch cost is a single function call.

| Runtime | Per event (ns) | Total 100k (µs) | vs SMS interp |
|---|---:|---:|---:|
| SMS \`on bench.tick()\` interpreter | $sms_ev_per | $sms_ev_total | — |
| **SMS \`on bench.tick()\` AOT** | **$sms_aot_per** | **$sms_aot_total** | **$AOT_VS_CPP vs C++** |
| C++ \`unordered_map\` dispatch | $cpp_ev_per | $cpp_ev_total | $EV_RATIO_CPP faster |
| Kotlin JVM \`HashMap\` dispatch | $kt_ev_per | $kt_ev_total | $EV_RATIO_KT faster |
$KN_EVENT_ROW

MDEOF

banner "Done"
ok "Results written to: $RESULT_FILE"
echo ""

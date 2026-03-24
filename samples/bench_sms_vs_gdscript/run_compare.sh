#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/../.. && pwd)"
THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

FORGECLI_BIN="${FORGECLI_BIN:-$ROOT_DIR/ForgeCli.Native/build/forgecli-native}"
SMS_NATIVE_LIB_DIR="${SMS_NATIVE_LIB_DIR:-$ROOT_DIR/SMSCore.Native/build}"
GODOT_BIN="${GODOT_BIN:-}"
DOTNET_BIN="${DOTNET_BIN:-}"

if [[ -z "$GODOT_BIN" ]]; then
  if [[ -x "$ROOT_DIR/.godot-bin/Godot_mono.app/Contents/MacOS/Godot" ]]; then
    GODOT_BIN="$ROOT_DIR/.godot-bin/Godot_mono.app/Contents/MacOS/Godot"
  elif [[ -x "$ROOT_DIR/.godot-bin/Godot_v4.6-stable_mono_linux_x86_64/Godot_v4.6-stable_mono_linux_x86_64" ]]; then
    GODOT_BIN="$ROOT_DIR/.godot-bin/Godot_v4.6-stable_mono_linux_x86_64/Godot_v4.6-stable_mono_linux_x86_64"
  elif [[ -x "$ROOT_DIR/.godot-bin/Godot_v4.6-stable_mono_linux_arm64/Godot_v4.6-stable_mono_linux_arm64" ]]; then
    GODOT_BIN="$ROOT_DIR/.godot-bin/Godot_v4.6-stable_mono_linux_arm64/Godot_v4.6-stable_mono_linux_arm64"
  elif command -v godot >/dev/null 2>&1; then
    GODOT_BIN="$(command -v godot)"
  fi
fi

if [[ -z "$DOTNET_BIN" ]]; then
  if command -v dotnet >/dev/null 2>&1; then
    DOTNET_BIN="$(command -v dotnet)"
  fi
fi

if [[ ! -x "$FORGECLI_BIN" ]]; then
  echo "ERROR: forgecli-native not found: $FORGECLI_BIN" >&2
  echo "Build first: ./run.sh build" >&2
  exit 1
fi

if [[ ! -d "$SMS_NATIVE_LIB_DIR" ]]; then
  echo "ERROR: SMS native lib dir not found: $SMS_NATIVE_LIB_DIR" >&2
  echo "Build first: ./run.sh build" >&2
  exit 1
fi

if [[ -z "$GODOT_BIN" || ! -x "$GODOT_BIN" ]]; then
  echo "ERROR: Godot binary not found. Set GODOT_BIN=/abs/path/to/godot" >&2
  echo "Hint: run ./scripts/setup_godot.sh once (from repo root)." >&2
  exit 1
fi

SMS_SRC="$THIS_DIR/sms_bench.sms"
SMS_BIN="$THIS_DIR/sms_bench_native"
GD_SRC="$THIS_DIR/gdscript_bench.gd"
CS_PROJ="$THIS_DIR/csharp_bench/csharp_bench.csproj"

RUNS="${RUNS:-10}"
WARMUP="${WARMUP:-3}"

if ! [[ "$RUNS" =~ ^[0-9]+$ ]] || (( RUNS <= 0 )); then
  echo "ERROR: RUNS must be a positive integer (current: $RUNS)." >&2
  exit 1
fi
if ! [[ "$WARMUP" =~ ^[0-9]+$ ]] || (( WARMUP < 0 )); then
  echo "ERROR: WARMUP must be a non-negative integer (current: $WARMUP)." >&2
  exit 1
fi

extract_result() {
  local text="$1"
  awk -F: '/^RESULT:/{print $2; exit}' <<<"$text" | tr -d '[:space:]'
}

extract_time_us_int() {
  local text="$1"
  local raw
  raw="$(awk -F: '/^TIME_US:/{print $2; exit}' <<<"$text" | tr -d '[:space:]')"
  if [[ -z "$raw" ]]; then
    echo ""
    return 0
  fi
  awk -v v="$raw" 'BEGIN { printf "%.0f", v + 0.0 }'
}

run_sms_once() {
  local out
  out="$("$SMS_BIN")"
  local result
  local time_us
  result="$(extract_result "$out")"
  time_us="$(extract_time_us_int "$out")"
  if [[ -z "$result" || -z "$time_us" ]]; then
    echo "ERROR: Failed to parse SMS output." >&2
    echo "$out" >&2
    exit 1
  fi
  echo "$result $time_us"
}

run_gd_once() {
  local out
  out="$("$GODOT_BIN" --headless --script "$GD_SRC")"
  local result
  local time_us
  result="$(extract_result "$out")"
  time_us="$(extract_time_us_int "$out")"
  if [[ -z "$result" || -z "$time_us" ]]; then
    echo "ERROR: Failed to parse GDScript output." >&2
    echo "$out" >&2
    exit 1
  fi
  echo "$result $time_us"
}

run_cs_once() {
  local out
  out="$("$DOTNET_BIN" run --configuration Release --project "$CS_PROJ" --nologo)"
  local result
  local time_us
  result="$(extract_result "$out")"
  time_us="$(extract_time_us_int "$out")"
  if [[ -z "$result" || -z "$time_us" ]]; then
    echo "ERROR: Failed to parse C# output." >&2
    echo "$out" >&2
    exit 1
  fi
  echo "$result $time_us"
}

median_of_lines() {
  local lines="$1"
  local n
  n="$(awk 'NF{c++} END{print c+0}' <<<"$lines")"
  if (( n <= 0 )); then
    echo 0
    return
  fi
  local sorted
  sorted="$(sort -n <<<"$lines")"
  if (( n % 2 == 1 )); then
    awk -v n="$n" 'NR==((n+1)/2){print; exit}' <<<"$sorted"
  else
    awk -v n="$n" '
      NR==(n/2){a=$1}
      NR==(n/2+1){b=$1; print int((a+b)/2); exit}
    ' <<<"$sorted"
  fi
}

p95_of_lines() {
  local lines="$1"
  local n
  n="$(awk 'NF{c++} END{print c+0}' <<<"$lines")"
  if (( n <= 0 )); then
    echo 0
    return
  fi
  local idx=$(( (95 * n + 99) / 100 ))
  if (( idx < 1 )); then idx=1; fi
  if (( idx > n )); then idx="$n"; fi
  sort -n <<<"$lines" | awk -v i="$idx" 'NR==i{print; exit}'
}

echo "== SMS Native build =="
SMS_NATIVE_LIB_DIR="$SMS_NATIVE_LIB_DIR" \
  "$FORGECLI_BIN" sms build "$SMS_SRC" --out "$SMS_BIN"

echo
echo "== Benchmark config =="
echo "RUNS=$RUNS, WARMUP=$WARMUP"

sms_times=()
gd_times=()
cs_times=()
sms_result_ref=""
gd_result_ref=""
cs_result_ref=""
have_cs=0

if [[ -n "$DOTNET_BIN" && -x "$DOTNET_BIN" && -f "$CS_PROJ" ]]; then
  have_cs=1
fi

echo
echo "== Warmup =="
for ((i = 1; i <= WARMUP; ++i)); do
  read -r s_res s_t <<<"$(run_sms_once)"
  read -r g_res g_t <<<"$(run_gd_once)"
  if (( have_cs == 1 )); then
    read -r c_res c_t <<<"$(run_cs_once)"
    printf "warmup %02d | SMS=%sus | GDScript=%sus | C#=%sus\n" "$i" "$s_t" "$g_t" "$c_t"
    if [[ -z "$cs_result_ref" ]]; then cs_result_ref="$c_res"; fi
  else
    printf "warmup %02d | SMS=%sus | GDScript=%sus\n" "$i" "$s_t" "$g_t"
  fi
  if [[ -z "$sms_result_ref" ]]; then sms_result_ref="$s_res"; fi
  if [[ -z "$gd_result_ref" ]]; then gd_result_ref="$g_res"; fi
done

echo
echo "== Measured runs =="
for ((i = 1; i <= RUNS; ++i)); do
  read -r s_res s_t <<<"$(run_sms_once)"
  read -r g_res g_t <<<"$(run_gd_once)"
  sms_times+=("$s_t")
  gd_times+=("$g_t")

  if (( have_cs == 1 )); then
    read -r c_res c_t <<<"$(run_cs_once)"
    cs_times+=("$c_t")
    printf "run %02d    | SMS=%sus | GDScript=%sus | C#=%sus\n" "$i" "$s_t" "$g_t" "$c_t"
  else
    printf "run %02d    | SMS=%sus | GDScript=%sus\n" "$i" "$s_t" "$g_t"
  fi

  if [[ "$s_res" != "$sms_result_ref" ]]; then
    echo "ERROR: SMS result changed across runs ($sms_result_ref vs $s_res)." >&2
    exit 1
  fi
  if [[ "$g_res" != "$gd_result_ref" ]]; then
    echo "ERROR: GDScript result changed across runs ($gd_result_ref vs $g_res)." >&2
    exit 1
  fi
  if (( have_cs == 1 )) && [[ "$c_res" != "$cs_result_ref" ]]; then
    echo "ERROR: C# result changed across runs ($cs_result_ref vs $c_res)." >&2
    exit 1
  fi
done

if [[ "$sms_result_ref" != "$gd_result_ref" ]]; then
  echo "ERROR: Result mismatch between SMS and GDScript ($sms_result_ref vs $gd_result_ref)." >&2
  exit 1
fi
if (( have_cs == 1 )) && [[ "$sms_result_ref" != "$cs_result_ref" ]]; then
  echo "ERROR: Result mismatch between SMS and C# ($sms_result_ref vs $cs_result_ref)." >&2
  exit 1
fi

sms_lines="$(printf "%s\n" "${sms_times[@]}")"
gd_lines="$(printf "%s\n" "${gd_times[@]}")"
sms_median="$(median_of_lines "$sms_lines")"
sms_p95="$(p95_of_lines "$sms_lines")"
gd_median="$(median_of_lines "$gd_lines")"
gd_p95="$(p95_of_lines "$gd_lines")"

speedup_median="$(awk -v a="$gd_median" -v b="$sms_median" 'BEGIN{ if (b <= 0) print "inf"; else printf "%.2f", a / b }')"
speedup_p95="$(awk -v a="$gd_p95" -v b="$sms_p95" 'BEGIN{ if (b <= 0) print "inf"; else printf "%.2f", a / b }')"

if (( have_cs == 1 )); then
  cs_lines="$(printf "%s\n" "${cs_times[@]}")"
  cs_median="$(median_of_lines "$cs_lines")"
  cs_p95="$(p95_of_lines "$cs_lines")"
  cs_vs_sms_median="$(awk -v a="$cs_median" -v b="$sms_median" 'BEGIN{ if (b <= 0) print "inf"; else printf "%.2f", a / b }')"
  cs_vs_sms_p95="$(awk -v a="$cs_p95" -v b="$sms_p95" 'BEGIN{ if (b <= 0) print "inf"; else printf "%.2f", a / b }')"
fi

echo
echo "== Summary (microseconds) =="
printf "%-12s | %-10s | %-10s\n" "Runtime" "Median" "P95"
printf "%-12s | %-10s | %-10s\n" "SMS Native" "$sms_median" "$sms_p95"
printf "%-12s | %-10s | %-10s\n" "GDScript" "$gd_median" "$gd_p95"
if (( have_cs == 1 )); then
  printf "%-12s | %-10s | %-10s\n" "C#" "$cs_median" "$cs_p95"
else
  echo "C# benchmark: skipped (dotnet not found)."
fi

echo
echo "== Speedup (GDScript / SMS Native) =="
echo "Median: ${speedup_median}x"
echo "P95:    ${speedup_p95}x"
if (( have_cs == 1 )); then
  echo
  echo "== Speedup (C# / SMS Native) =="
  echo "Median: ${cs_vs_sms_median}x"
  echo "P95:    ${cs_vs_sms_p95}x"
fi
echo "RESULT: $sms_result_ref"

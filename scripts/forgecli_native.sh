#!/usr/bin/env bash
set -euo pipefail

# Resolve REPO_ROOT even when this script is called via a symlink.
SCRIPT_PATH="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/$(basename -- "${BASH_SOURCE[0]}")"
if [[ -L "$SCRIPT_PATH" ]]; then
  SCRIPT_PATH="$(readlink "$SCRIPT_PATH")"
fi
REPO_ROOT="$(cd -- "$(dirname -- "$SCRIPT_PATH")/.." && pwd)"

CLI_NATIVE_DIR="$REPO_ROOT/ForgeCli.Native"
CLI_NATIVE_BUILD_DIR="$CLI_NATIVE_DIR/build"
CLI_NATIVE_BIN="$CLI_NATIVE_BUILD_DIR/forgecli-native"

is_android_build() {
  local prev=""
  for arg in "$@"; do
    if [[ "$prev" == "build" && "$arg" == "android" ]]; then return 0; fi
    prev="$arg"
  done
  return 1
}

ensure_cli_native_built() {
  if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake not found. Install cmake to build ForgeCli.Native." >&2
    exit 1
  fi
  # Always run cmake --build (incremental: only recompiles changed sources).
  cmake -S "$CLI_NATIVE_DIR" -B "$CLI_NATIVE_BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -Wno-dev 2>/dev/null
  cmake --build "$CLI_NATIVE_BUILD_DIR" --config Release --target forgecli-native -j"$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"
}

ensure_android_native_built() {
  local so="$REPO_ROOT/ForgeRunner.Native/build-android/arm64-v8a/libforge_runner_native.so"
  if [[ -f "$so" ]]; then return 0; fi
  echo "Android native libraries missing — building now..." >&2
  local build_script="$REPO_ROOT/scripts/build_android_native.sh"
  if [[ ! -f "$build_script" ]]; then
    echo "ERROR: $build_script not found." >&2
    exit 1
  fi
  bash "$build_script"
}

set_env_defaults() {
  export SML_NATIVE_LIB_DIR="${SML_NATIVE_LIB_DIR:-$CLI_NATIVE_BUILD_DIR}"
  export SMS_NATIVE_LIB_DIR="${SMS_NATIVE_LIB_DIR:-$CLI_NATIVE_BUILD_DIR}"
  export FORGE_HOST_PROJECT_DIR="${FORGE_HOST_PROJECT_DIR:-$REPO_ROOT/ForgeRunner.Native/host}"
  # Prefer built output; fall back to the committed dylib in host/ (available after git clone).
  # For Android builds, use build-android/ which contains ABI-specific .so files.
  if [[ -z "${FORGE_NATIVE_LIB_DIR:-}" ]]; then
    if is_android_build "$@" && [[ -d "$REPO_ROOT/ForgeRunner.Native/build-android" ]]; then
      export FORGE_NATIVE_LIB_DIR="$REPO_ROOT/ForgeRunner.Native/build-android"
    elif [[ -d "$REPO_ROOT/ForgeRunner.Native/build" ]]; then
      export FORGE_NATIVE_LIB_DIR="$REPO_ROOT/ForgeRunner.Native/build"
    else
      export FORGE_NATIVE_LIB_DIR="$REPO_ROOT/ForgeRunner.Native/host"
    fi
  fi

  # Fall back to the Android debug keystore for dev builds.
  if is_android_build "$@" && [[ -z "${FORGE_ANDROID_RELEASE_KEYSTORE:-}" ]]; then
    local debug_ks="$HOME/.android/debug.keystore"
    if [[ -f "$debug_ks" ]]; then
      export FORGE_ANDROID_RELEASE_KEYSTORE="$debug_ks"
      export FORGE_ANDROID_RELEASE_USER="${FORGE_ANDROID_RELEASE_USER:-androiddebugkey}"
      export FORGE_ANDROID_RELEASE_PASSWORD="${FORGE_ANDROID_RELEASE_PASSWORD:-android}"
      echo "[WARN] Using debug keystore for Android signing. Set FORGE_ANDROID_RELEASE_* for production builds." >&2
    fi
  fi
}

ensure_cli_native_built
if is_android_build "$@"; then ensure_android_native_built; fi
set_env_defaults "$@"
exec "$CLI_NATIVE_BIN" "$@"

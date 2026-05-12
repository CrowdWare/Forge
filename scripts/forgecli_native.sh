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

ensure_cli_native_built() {
  if [[ -x "$CLI_NATIVE_BIN" ]]; then
    return 0
  fi
  if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake not found. Install cmake to build ForgeCli.Native." >&2
    exit 1
  fi
  echo "ForgeCli.Native binary missing — building now..."
  cmake -S "$CLI_NATIVE_DIR" -B "$CLI_NATIVE_BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
  cmake --build "$CLI_NATIVE_BUILD_DIR" --config Release
}

set_env_defaults() {
  export SML_NATIVE_LIB_DIR="${SML_NATIVE_LIB_DIR:-$CLI_NATIVE_BUILD_DIR}"
  export SMS_NATIVE_LIB_DIR="${SMS_NATIVE_LIB_DIR:-$CLI_NATIVE_BUILD_DIR}"
  export FORGE_HOST_PROJECT_DIR="${FORGE_HOST_PROJECT_DIR:-$REPO_ROOT/ForgeRunner.Native/host}"
  # Prefer built output; fall back to the committed dylib in host/ (available after git clone).
  if [[ -z "${FORGE_NATIVE_LIB_DIR:-}" ]]; then
    if [[ -d "$REPO_ROOT/ForgeRunner.Native/build" ]]; then
      export FORGE_NATIVE_LIB_DIR="$REPO_ROOT/ForgeRunner.Native/build"
    else
      export FORGE_NATIVE_LIB_DIR="$REPO_ROOT/ForgeRunner.Native/host"
    fi
  fi
}

ensure_cli_native_built
set_env_defaults
exec "$CLI_NATIVE_BIN" "$@"


#!/usr/bin/env bash
# Builds ForgeRunner.Native Android shared libraries (.so) from source.
# Requires the Android NDK; set ANDROID_NDK or install via Android Studio.
# Run automatically by forgecli when build-android/ is missing.
set -euo pipefail

SCRIPT_PATH="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/$(basename -- "${BASH_SOURCE[0]}")"
if [[ -L "$SCRIPT_PATH" ]]; then SCRIPT_PATH="$(readlink "$SCRIPT_PATH")"; fi
REPO_ROOT="$(cd -- "$(dirname -- "$SCRIPT_PATH")/.." && pwd)"

NATIVE_DIR="$REPO_ROOT/ForgeRunner.Native"
GODOT_CPP_SRC="$REPO_ROOT/third_party/godot-cpp"
GODOT_CPP_BUILD="$REPO_ROOT/third_party/godot-cpp/build-android"
PLATFORM="${ANDROID_PLATFORM:-android-24}"

# ---------------------------------------------------------------------------
# Locate Android NDK
# ---------------------------------------------------------------------------
find_ndk() {
  if [[ -n "${ANDROID_NDK:-}" && -d "$ANDROID_NDK" ]]; then echo "$ANDROID_NDK"; return; fi
  local sdk="${ANDROID_HOME:-}"
  if [[ -z "$sdk" ]]; then
    for candidate in "$HOME/Library/Android/sdk" "$HOME/Android/Sdk"; do
      [[ -d "$candidate" ]] && sdk="$candidate" && break
    done
  fi
  if [[ -n "$sdk" && -d "$sdk/ndk" ]]; then
    local latest
    latest="$(ls -1 "$sdk/ndk" | sort -V | tail -1)"
    [[ -n "$latest" ]] && echo "$sdk/ndk/$latest" && return
  fi
  echo ""
}

NDK_DIR="$(find_ndk)"
if [[ -z "$NDK_DIR" ]]; then
  echo "ERROR: Android NDK not found." >&2
  echo "  Install via Android Studio → SDK Manager → SDK Tools → NDK" >&2
  echo "  or set ANDROID_NDK=/path/to/ndk" >&2
  exit 1
fi

TOOLCHAIN="$NDK_DIR/build/cmake/android.toolchain.cmake"
echo "NDK:      $NDK_DIR"
echo "Platform: $PLATFORM"

# ---------------------------------------------------------------------------
# Step 1: Build godot-cpp for Android (outputs lib to third_party/godot-cpp/bin/)
# ---------------------------------------------------------------------------
build_godot_cpp_abi() {
  local abi="$1"
  local build_dir="$GODOT_CPP_BUILD/$abi"
  local arch=$( [[ "$abi" == arm64-v8a ]] && echo arm64 || echo arm32 )
  local bin_dir="$GODOT_CPP_SRC/bin"
  local gen_dir="$GODOT_CPP_SRC/gen"

  # Check if lib already present in source tree
  if compgen -G "$bin_dir/libgodot-cpp.android.*.$arch.a" > /dev/null 2>&1; then
    echo "[SKIP] godot-cpp $abi already built"
    return
  fi

  echo ""
  echo "--- Building godot-cpp $abi ---"
  cmake \
    -S "$GODOT_CPP_SRC" \
    -B "$build_dir" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DANDROID_ABI="$abi" \
    -DANDROID_PLATFORM="$PLATFORM" \
    -DCMAKE_BUILD_TYPE=Release \
    -DGODOTCPP_TARGET=template_release
  cmake --build "$build_dir" --config Release

  # godot-cpp outputs to <build>/bin/ — copy into source tree for ForgeRunner to find
  mkdir -p "$bin_dir"
  cp "$build_dir"/bin/libgodot-cpp.android.*.$arch.a "$bin_dir/"

  # Copy gen/ (generated bindings) into source tree
  if [[ -d "$build_dir/gen" && ! -d "$gen_dir" ]]; then
    cp -r "$build_dir/gen" "$gen_dir"
  fi
}

# ---------------------------------------------------------------------------
# Step 2: Build ForgeRunner.Native for Android
# ---------------------------------------------------------------------------
build_frn_abi() {
  local abi="$1"
  local build_dir="$NATIVE_DIR/build-android/$abi"
  local godot_gen="$GODOT_CPP_SRC/gen"

  echo ""
  echo "--- Building ForgeRunner.Native $abi ---"
  cmake \
    -S "$NATIVE_DIR" \
    -B "$build_dir" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DANDROID_ABI="$abi" \
    -DANDROID_PLATFORM="$PLATFORM" \
    -DCMAKE_BUILD_TYPE=Release \
    -DGODOT_CPP_DIR="$GODOT_CPP_SRC" \
    -DGODOT_CPP_GEN_DIR="$godot_gen"
  cmake --build "$build_dir" --config Release
}

build_godot_cpp_abi "arm64-v8a"
build_frn_abi "arm64-v8a"

echo ""
echo "[OK] Android native libraries ready in $NATIVE_DIR/build-android"

#!/usr/bin/env bash
set -euo pipefail
echo ""
echo "███████╗ ██████╗ ██████╗  ██████╗ ███████╗   ██╗  ██╗██████╗"
echo "██╔════╝██╔═══██╗██╔══██╗██╔════╝ ██╔════╝   ██║  ██║██╔══██╗"
echo "█████╗  ██║   ██║██████╔╝██║  ███╗█████╗     ███████║██║  ██║"
echo "██╔══╝  ██║   ██║██╔══██╗██║   ██║██╔══╝     ╚════██║██║  ██║"
echo "██║     ╚██████╔╝██║  ██║╚██████╔╝███████╗        ██║██████╔╝"
echo "╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝        ╚═╝╚═════╝"
echo "Built with love, coffee, and a stubborn focus on simplicity."
echo ""
REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
OS_NAME="$(uname -s)"
LOCAL_RUN_INCLUDE="$REPO_ROOT/run.local.sh"
LOCAL_GODOT_CPP_PATH_FILE="$REPO_ROOT/.godot_cpp_dir"
NATIVE_HOST_DIR="$REPO_ROOT/ForgeRunner.Native/host"

# Load optional local overrides/env (git-ignored).
if [[ -f "$LOCAL_RUN_INCLUDE" ]]; then
  # shellcheck source=/dev/null
  source "$LOCAL_RUN_INCLUDE"
fi

# Allow persisting GODOT_CPP_DIR in a git-ignored local file.
if [[ -z "${GODOT_CPP_DIR:-}" && -f "$LOCAL_GODOT_CPP_PATH_FILE" ]]; then
  GODOT_CPP_DIR="$(head -n 1 "$LOCAL_GODOT_CPP_PATH_FILE" | tr -d '\r')"
  export GODOT_CPP_DIR
fi

build_native_lib() {
  local name="$1"
  local src_dir="$2"
  local build_dir="$3"
  local with_tests="${4:-false}"

  if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake not found. Install cmake to build ${name}." >&2
    return 1
  fi

  if [[ ! -d "$src_dir" ]]; then
    echo "ERROR: Missing source directory for ${name}: $src_dir" >&2
    return 1
  fi

  mkdir -p "$build_dir"
  echo "Configuring ${name}..."
  if [[ "$with_tests" == "true" ]]; then
    cmake -S "$src_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
  else
    cmake -S "$src_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
  fi
  echo "Building ${name}..."
  cmake --build "$build_dir" --config Release
}

build_forge_runner_native_host() {
  local src_dir="$REPO_ROOT/ForgeRunner.Native"
  local build_dir="$src_dir/build"
  local out_dir="$src_dir/dist"
  local host_dir="$src_dir/host"

  if [[ -z "${GODOT_CPP_DIR:-}" ]]; then
    echo "ERROR: GODOT_CPP_DIR is required to build ForgeRunner.Native." >&2
    echo "       Example: export GODOT_CPP_DIR=/absolute/path/to/godot-cpp" >&2
    echo "       Or store it once in $LOCAL_GODOT_CPP_PATH_FILE" >&2
    return 1
  fi

  if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake not found." >&2
    return 1
  fi

  mkdir -p "$build_dir"
  echo "Configuring ForgeRunner.Native..."
  cmake -S "$src_dir" -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON \
    -DGODOT_CPP_DIR="$GODOT_CPP_DIR"
  echo "Building ForgeRunner.Native..."
  cmake --build "$build_dir" --config Release

  mkdir -p "$out_dir"
  local artifact=""
  local exe_artifact=""
  case "$OS_NAME" in
    Darwin)
      artifact="$build_dir/libforge_runner_native.dylib"
      exe_artifact="$build_dir/forge-runner-native"
      ;;
    Linux)
      artifact="$build_dir/libforge_runner_native.so"
      exe_artifact="$build_dir/forge-runner-native"
      ;;
    *)
      artifact="$build_dir/forge_runner_native.dll"
      exe_artifact="$build_dir/forge-runner-native.exe"
      ;;
  esac

  if [[ ! -f "$artifact" ]]; then
    echo "ERROR: ForgeRunner.Native artifact not found: $artifact" >&2
    return 1
  fi

  cp "$artifact" "$out_dir/"
  echo "ForgeRunner.Native artifact copied to $out_dir/$(basename "$artifact")"
  mkdir -p "$host_dir"
  cp "$artifact" "$host_dir/"
  echo "ForgeRunner.Native artifact copied to $host_dir/$(basename "$artifact")"

  if [[ ! -f "$exe_artifact" ]]; then
    echo "ERROR: ForgeRunner.Native executable not found: $exe_artifact" >&2
    return 1
  fi

  cp "$exe_artifact" "$out_dir/"
  chmod +x "$out_dir/$(basename "$exe_artifact")" || true
  echo "ForgeRunner.Native executable copied to $out_dir/$(basename "$exe_artifact")"
}

usage() {
  cat <<USAGE
Usage:
  ./run.sh [runner-args...]   Run ForgeRunner.Native with passed arguments (for example --url ...)
  ./run.sh run [runner-args]  Same as above (explicit mode)
  ./run.sh none               Run ForgeRunner.Native without arguments (runtime may restore last URL)
  ./run.sh default            Run local Default app (file://)
  ./run.sh designer           Run local ForgeDesigner app (file://)
  ./run.sh remote-default     Run hosted Default app (AppServer manifest)
  ./run.sh remote-designer    Run hosted ForgeDesigner app (AppServer manifest)
  ./run.sh poser              Run local ForgePoser app
  ./run.sh url <url>          Run any app by URL (file:// or http(s)://)
  ./run.sh docs               Generate SML/SMS documentation (headless Godot)
  ./run.sh pub "msg"          Local publish override flow (run.local.sh)
  ./run.sh build              Build native stack (default)
  ./run.sh build-native       Same as build
  ./run.sh build-host         Build ForgeRunner.Native only
  ./run.sh build-mac [forgecli-build-args]
                             Build macOS app zip via ForgeCli.Native (wraps 'forgecli-native build mac')
  ./run.sh build-android [forgecli-build-args]
                             Build Android apk via ForgeCli.Native (wraps 'forgecli-native build android')
  ./run.sh build-android-autosign [forgecli-build-args]
                             Build Android apk and force local test signing config
  ./run.sh test               Run native tests (ForgeRunner.Native)
  ./run.sh clean              Remove native build/dist folders
  ./run.sh pin <file-or-dir> [--name <n>]
                             Pin a file or directory to IPFS via Pinata (requires PINATA_JWT env var)

Environment:
  FORGE_RUNNER_NATIVE_BIN     Optional explicit native runner executable path
  FORGE_NATIVE_ANDROID_LIB_DIR Optional dir with Android ForgeRunner .so libs (used by build-android)
  ANDROID_SDK_ROOT / ANDROID_HOME Optional Android SDK root (used for NDK + godot-cpp Android build)
  ANDROID_NDK_ROOT            Optional explicit Android NDK path for Android native build
  PINATA_JWT                  Pinata API JWT — required for the 'pin' mode
  FORGE_ANDROID_RELEASE_KEYSTORE Optional path to Android release keystore for Godot signing
  FORGE_ANDROID_RELEASE_USER      Optional keystore alias/user for Godot signing
  FORGE_ANDROID_RELEASE_PASSWORD  Optional keystore password for Godot signing
                                 Required for build-android.
                                 build-android-autosign creates/uses local test signing.
  FORGE_ANDROID_PACKAGE_ID       Optional Android app id (e.g. com.example.myapp)
  FORGE_ANDROID_VERSION_CODE     Optional explicit Android version/code override
  GODOT_CPP_DIR               Required for build-host/build
  ./.godot_cpp_dir            Optional file with one line: absolute GODOT_CPP_DIR
  FORGE_APP_SERVER_BASE_URL   Base URL for hosted app manifests (default: https://crowdware.github.io/Forge)
USAGE
}

FORGE_RESOLVED_ANDROID_NATIVE_LIB_DIR=""

resolve_android_java_home() {
  if [[ -n "${FORGE_ANDROID_JAVA_HOME:-}" ]]; then
    if [[ -x "${FORGE_ANDROID_JAVA_HOME}/bin/jlink" ]]; then
      echo "$FORGE_ANDROID_JAVA_HOME"
      return 0
    fi
    echo "ERROR: FORGE_ANDROID_JAVA_HOME is set but invalid (missing bin/jlink): $FORGE_ANDROID_JAVA_HOME" >&2
    return 1
  fi

  local candidates=(
    "/Applications/Android Studio.app/Contents/jbr/Contents/Home"
    "/Library/Java/JavaVirtualMachines/temurin-21.jdk/Contents/Home"
    "/Library/Java/JavaVirtualMachines/temurin-17.jdk/Contents/Home"
    "/Library/Java/JavaVirtualMachines/jdk-21.jdk/Contents/Home"
    "/Library/Java/JavaVirtualMachines/jdk-17.jdk/Contents/Home"
  )

  local detected_java_home=""
  detected_java_home="$(/usr/libexec/java_home -v 21 2>/dev/null || true)"
  if [[ -n "$detected_java_home" ]]; then
    candidates+=("$detected_java_home")
  fi
  detected_java_home="$(/usr/libexec/java_home -v 17 2>/dev/null || true)"
  if [[ -n "$detected_java_home" ]]; then
    candidates+=("$detected_java_home")
  fi

  local candidate=""
  for candidate in "${candidates[@]}"; do
    [[ -n "$candidate" ]] || continue
    [[ -x "$candidate/bin/jlink" ]] || continue
    if [[ "$candidate" == *graal* || "$candidate" == *Graal* ]]; then
      continue
    fi
    echo "$candidate"
    return 0
  done

  if [[ -n "${JAVA_HOME:-}" && -x "${JAVA_HOME}/bin/jlink" ]]; then
    if [[ "$JAVA_HOME" == *graal* || "$JAVA_HOME" == *Graal* ]]; then
      echo "ERROR: JAVA_HOME points to GraalVM, which breaks Android jlink in this build path: $JAVA_HOME" >&2
      echo "Set FORGE_ANDROID_JAVA_HOME to a non-Graal JDK (Android Studio JBR or Temurin)." >&2
      return 1
    fi
    echo "$JAVA_HOME"
    return 0
  fi

  return 1
}

ensure_android_java_env() {
  local java_home=""
  if ! java_home="$(resolve_android_java_home)"; then
    echo "ERROR: Could not resolve a compatible Android JDK with jlink." >&2
    echo "Set FORGE_ANDROID_JAVA_HOME explicitly (non-Graal), e.g. Android Studio JBR." >&2
    return 1
  fi

  export JAVA_HOME="$java_home"
  export PATH="$JAVA_HOME/bin:$PATH"
  export GRADLE_OPTS="-Dorg.gradle.java.home=$JAVA_HOME ${GRADLE_OPTS:-}"
  export ORG_GRADLE_PROJECT_org_gradle_java_home="$JAVA_HOME"
  echo "Using Android JAVA_HOME: $JAVA_HOME"
  if [[ "$JAVA_HOME" == *graal* || "$JAVA_HOME" == *Graal* ]]; then
    echo "WARN: JAVA_HOME points to GraalVM; Android Gradle jlink may fail." >&2
  fi
}

run_forgecli_build_target() {
  local target="$1"
  shift || true

  build_native_lib "ForgeCli.Native" "$REPO_ROOT/ForgeCli.Native" "$REPO_ROOT/ForgeCli.Native/build" false
  build_forge_runner_native_host

  local forgecli_bin="$REPO_ROOT/ForgeCli.Native/build/forgecli-native"
  if [[ "$OS_NAME" != "Darwin" && "$OS_NAME" != "Linux" ]]; then
    forgecli_bin="$REPO_ROOT/ForgeCli.Native/build/forgecli-native.exe"
  fi
  if [[ ! -x "$forgecli_bin" ]]; then
    echo "ERROR: ForgeCli.Native executable not found: $forgecli_bin" >&2
    return 1
  fi

  local native_lib_dir="$REPO_ROOT/ForgeRunner.Native/build"
  if [[ "$target" == "android" ]]; then
    if ! ensure_android_signing_env; then
      return 1
    fi
    if ! ensure_android_java_env; then
      return 1
    fi
    if ! ensure_incremented_android_version_code; then
      return 1
    fi
    if ! resolve_android_native_lib_dir; then
      return 1
    fi
    native_lib_dir="$FORGE_RESOLVED_ANDROID_NATIVE_LIB_DIR"
  fi

  local default_project="$REPO_ROOT/samples/${target}_demo"
  echo "Running forgecli build ${target}..."
  SML_NATIVE_LIB_DIR="$REPO_ROOT/ForgeCli.Native/build" \
  SMS_NATIVE_LIB_DIR="$REPO_ROOT/ForgeCli.Native/build" \
  FORGE_HOST_PROJECT_DIR="$REPO_ROOT/ForgeRunner.Native/host" \
  FORGE_NATIVE_LIB_DIR="$native_lib_dir" \
    "$forgecli_bin" build "$target" --project "$default_project" "$@"
}

ensure_android_signing_env() {
  local has_keystore=false
  local has_user=false
  local has_password=false
  [[ -n "${FORGE_ANDROID_RELEASE_KEYSTORE:-}" ]] && has_keystore=true
  [[ -n "${FORGE_ANDROID_RELEASE_USER:-}" ]] && has_user=true
  [[ -n "${FORGE_ANDROID_RELEASE_PASSWORD:-}" ]] && has_password=true

  if [[ "$has_keystore" == true && "$has_user" == true && "$has_password" == true ]]; then
    return 0
  fi

  if [[ "$has_keystore" == true || "$has_user" == true || "$has_password" == true ]]; then
    echo "ERROR: Android signing config is partially set." >&2
    echo "Set all 3 env vars together or none:" >&2
    echo "  FORGE_ANDROID_RELEASE_KEYSTORE" >&2
    echo "  FORGE_ANDROID_RELEASE_USER" >&2
    echo "  FORGE_ANDROID_RELEASE_PASSWORD" >&2
    return 1
  fi

  echo "ERROR: Missing Android release signing config." >&2
  echo "Set FORGE_ANDROID_RELEASE_KEYSTORE, FORGE_ANDROID_RELEASE_USER, FORGE_ANDROID_RELEASE_PASSWORD." >&2
  echo "Or use: ./run.sh build-android-autosign" >&2
  return 1
}

ensure_android_test_signing_env() {
  if ! command -v keytool >/dev/null 2>&1; then
    echo "ERROR: keytool not found. Install a JDK or set FORGE_ANDROID_RELEASE_* manually." >&2
    return 1
  fi

  local signing_dir="$REPO_ROOT/.local/android-signing"
  local keystore_path="$signing_dir/forge-test.keystore"
  local alias="forge-test"
  local password="android"
  mkdir -p "$signing_dir"

  if [[ ! -f "$keystore_path" ]]; then
    echo "Creating local Android test keystore: $keystore_path"
    if ! keytool -genkeypair \
      -keystore "$keystore_path" \
      -storepass "$password" \
      -keypass "$password" \
      -alias "$alias" \
      -keyalg RSA \
      -keysize 2048 \
      -validity 10000 \
      -dname "CN=Forge Test,O=CrowdWare,C=DE" >/dev/null 2>&1; then
      echo "ERROR: Failed to create Android test keystore." >&2
      return 1
    fi
  fi

  export FORGE_ANDROID_RELEASE_KEYSTORE="$keystore_path"
  export FORGE_ANDROID_RELEASE_USER="$alias"
  export FORGE_ANDROID_RELEASE_PASSWORD="$password"
  echo "Using local Android test signing config from run.sh (keystore: $keystore_path, alias: $alias)."
  return 0
}

ensure_incremented_android_version_code() {
  if [[ -n "${FORGE_ANDROID_VERSION_CODE:-}" ]]; then
    return 0
  fi

  local channel="${FORGE_ANDROID_BUILD_CHANNEL:-release}"
  local state_dir="$REPO_ROOT/.local/android-build"
  local state_file="$state_dir/version_code_${channel}.txt"
  mkdir -p "$state_dir"

  local current=0
  if [[ -f "$state_file" ]]; then
    local raw=""
    raw="$(tr -d '[:space:]' < "$state_file")"
    if [[ "$raw" =~ ^[0-9]+$ ]]; then
      current="$raw"
    fi
  fi

  local next=$((current + 1))
  printf '%s\n' "$next" > "$state_file"
  export FORGE_ANDROID_VERSION_CODE="$next"
  echo "Using Android version/code: $FORGE_ANDROID_VERSION_CODE (channel: $channel)"
  return 0
}

set_autosign_android_package_id() {
  local base_id="${FORGE_ANDROID_PACKAGE_ID:-}"
  local autosign_suffix="autosign"
  local autosign_id=""

  if [[ -n "$base_id" ]]; then
    if [[ "$base_id" == *".${autosign_suffix}" ]]; then
      autosign_id="$base_id"
    else
      autosign_id="${base_id}.${autosign_suffix}"
    fi
  else
    autosign_id="io.crowdware.forge.autosign"
  fi

  export FORGE_ANDROID_PACKAGE_ID="$autosign_id"
  echo "Using autosign Android package id: $FORGE_ANDROID_PACKAGE_ID"
  return 0
}

resolve_android_native_lib_dir() {
  has_android_lib_token() {
    local dir="$1"
    local token="$2"
    [[ -d "$dir" ]] && find "$dir" -type f -name "*.so" -print | grep -i "$token" >/dev/null 2>&1
  }

  if [[ -n "${FORGE_NATIVE_ANDROID_LIB_DIR:-}" ]]; then
    if has_android_lib_token "$FORGE_NATIVE_ANDROID_LIB_DIR" "forge_runner_native" && \
       has_android_lib_token "$FORGE_NATIVE_ANDROID_LIB_DIR" "sms_native"; then
      FORGE_RESOLVED_ANDROID_NATIVE_LIB_DIR="$FORGE_NATIVE_ANDROID_LIB_DIR"
      return 0
    fi
    echo "ERROR: FORGE_NATIVE_ANDROID_LIB_DIR must contain both forge_runner_native and sms_native Android .so libs: $FORGE_NATIVE_ANDROID_LIB_DIR" >&2
    return 1
  fi

  local candidates=(
    "$REPO_ROOT/ForgeRunner.Native/build-android"
    "$REPO_ROOT/ForgeRunner.Native/build/android"
    "$REPO_ROOT/ForgeRunner.Native/android"
    "$REPO_ROOT/ForgeRunner.Native/dist/android"
    "$REPO_ROOT/ForgeRunner.Native/build"
  )

  local candidate=""
  for candidate in "${candidates[@]}"; do
    if has_android_lib_token "$candidate" "forge_runner_native" && \
       has_android_lib_token "$candidate" "sms_native"; then
      FORGE_RESOLVED_ANDROID_NATIVE_LIB_DIR="$candidate"
      return 0
    fi
  done

  echo "No prebuilt Android ForgeRunner libs found. Building now..." >&2
  if build_forge_runner_native_android; then
    for candidate in "${candidates[@]}"; do
      if has_android_lib_token "$candidate" "forge_runner_native" && \
         has_android_lib_token "$candidate" "sms_native"; then
        FORGE_RESOLVED_ANDROID_NATIVE_LIB_DIR="$candidate"
        return 0
      fi
    done
  fi

  echo "ERROR: No Android ForgeRunner native libraries found (*.so)." >&2
  echo "Set FORGE_NATIVE_ANDROID_LIB_DIR to a directory containing forge_runner_native Android libs." >&2
  echo "Example:" >&2
  echo "  FORGE_NATIVE_ANDROID_LIB_DIR=/abs/path/to/android/libs ./run.sh build-android" >&2
  return 1
}

resolve_android_ndk_root() {
  local required_ndk_version="28.1.13356709"

  if [[ -n "${ANDROID_NDK_ROOT:-}" && -d "$ANDROID_NDK_ROOT" ]]; then
    echo "$ANDROID_NDK_ROOT"
    return 0
  fi

  local sdk_root="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
  if [[ -n "$sdk_root" && -d "$sdk_root/ndk/${required_ndk_version}" ]]; then
    echo "$sdk_root/ndk/${required_ndk_version}"
    return 0
  fi
  if [[ -n "$sdk_root" && -d "$sdk_root/ndk" ]]; then
    local ndk_dir=""
    ndk_dir="$(find "$sdk_root/ndk" -mindepth 1 -maxdepth 1 -type d | sort -V | tail -n 1)"
    if [[ -n "$ndk_dir" ]]; then
      echo "$ndk_dir"
      return 0
    fi
  fi
  return 1
}

resolve_android_sdk_root() {
  if [[ -n "${ANDROID_SDK_ROOT:-}" && -d "$ANDROID_SDK_ROOT" ]]; then
    echo "$ANDROID_SDK_ROOT"
    return 0
  fi
  if [[ -n "${ANDROID_HOME:-}" && -d "$ANDROID_HOME" ]]; then
    echo "$ANDROID_HOME"
    return 0
  fi
  return 1
}

resolve_android_ndk_version() {
  local ndk_root="$1"
  basename "$ndk_root"
}

godot_cpp_has_android_lib() {
  local abi_key="$1"
  local bin_dir="${GODOT_CPP_DIR}/bin"
  if [[ ! -d "$bin_dir" ]]; then
    return 1
  fi
  find "$bin_dir" -maxdepth 1 -type f -name "*android*.${abi_key}.a" -print -quit | grep -q .
}

build_godot_cpp_android_libs() {
  local ndk_root="$1"

  if ! command -v scons >/dev/null 2>&1; then
    echo "ERROR: scons not found. Install scons to build godot-cpp Android libs." >&2
    return 1
  fi

  local sdk_root=""
  if ! sdk_root="$(resolve_android_sdk_root)"; then
    echo "ERROR: Android SDK not found. Set ANDROID_SDK_ROOT or ANDROID_HOME." >&2
    return 1
  fi

  local ndk_version=""
  ndk_version="$(resolve_android_ndk_version "$ndk_root")"
  if [[ -z "$ndk_version" ]]; then
    echo "ERROR: Failed to resolve NDK version from path: $ndk_root" >&2
    return 1
  fi

  echo "Building godot-cpp Android libs with SDK=$sdk_root, NDK=$ndk_version..."
  (
    cd "$GODOT_CPP_DIR"
    ANDROID_HOME="$sdk_root" scons platform=android target=template_release arch=arm64 api_version=4.6 ndk_version="$ndk_version"
    ANDROID_HOME="$sdk_root" scons platform=android target=template_release arch=arm32 api_version=4.6 ndk_version="$ndk_version"
  )
}

build_forge_runner_native_android() {
  if [[ -z "${GODOT_CPP_DIR:-}" || ! -d "$GODOT_CPP_DIR" ]]; then
    echo "ERROR: GODOT_CPP_DIR is required for Android native build." >&2
    echo "       Example: export GODOT_CPP_DIR=/Users/art/SourceCode/godot-cpp" >&2
    return 1
  fi

  local ndk_root=""
  if ! ndk_root="$(resolve_android_ndk_root)"; then
    echo "ERROR: Android NDK not found. Required version: 28.1.13356709" >&2
    echo "Set ANDROID_NDK_ROOT or ANDROID_SDK_ROOT/ANDROID_HOME." >&2
    return 1
  fi

  local missing=()
  if ! godot_cpp_has_android_lib "arm64"; then
    missing+=("arm64")
  fi
  if ! godot_cpp_has_android_lib "arm32"; then
    missing+=("arm32")
  fi
  if [[ ${#missing[@]} -gt 0 ]]; then
    echo "Missing godot-cpp Android static libs for: ${missing[*]} (trying auto-build)..." >&2
    if ! build_godot_cpp_android_libs "$ndk_root"; then
      echo "ERROR: Failed to build missing godot-cpp Android static libs." >&2
      return 1
    fi
    missing=()
    if ! godot_cpp_has_android_lib "arm64"; then
      missing+=("arm64")
    fi
    if ! godot_cpp_has_android_lib "arm32"; then
      missing+=("arm32")
    fi
    if [[ ${#missing[@]} -gt 0 ]]; then
      echo "ERROR: godot-cpp Android static libs still missing after auto-build: ${missing[*]}" >&2
      return 1
    fi
  fi

  local frn_dir="$REPO_ROOT/ForgeRunner.Native"
  local toolchain_file="$ndk_root/build/cmake/android.toolchain.cmake"
  if [[ ! -f "$toolchain_file" ]]; then
    echo "ERROR: Android toolchain file not found: $toolchain_file" >&2
    return 1
  fi

  local abi=""
  local build_dir=""
  for abi in arm64-v8a; do
    build_dir="$frn_dir/build-android/$abi"
    mkdir -p "$build_dir"
    echo "Configuring ForgeRunner.Native Android ($abi)..."
    cmake -S "$frn_dir" -B "$build_dir" \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=OFF \
      -DGODOT_CPP_DIR="$GODOT_CPP_DIR" \
      -DCMAKE_TOOLCHAIN_FILE="$toolchain_file" \
      -DANDROID_ABI="$abi" \
      -DANDROID_PLATFORM=android-24
    echo "Building ForgeRunner.Native Android ($abi)..."
    cmake --build "$build_dir" --config Release --target forge_runner_native sms_native
  done

  return 0
}

resolve_native_runner_bin() {
  if [[ -n "${FORGE_RUNNER_NATIVE_BIN:-}" ]]; then
    echo "$FORGE_RUNNER_NATIVE_BIN"
    return 0
  fi

  local dist="$REPO_ROOT/ForgeRunner.Native/dist"
  case "$OS_NAME" in
    Darwin|Linux)
      echo "$dist/forge-runner-native"
      ;;
    *)
      echo "$dist/forge-runner-native.exe"
      ;;
  esac
}

resolve_native_host_lib() {
  local src_dir="$REPO_ROOT/ForgeRunner.Native"
  case "$OS_NAME" in
    Darwin)
      if [[ -f "$src_dir/dist/libforge_runner_native.dylib" ]]; then
        echo "$src_dir/dist/libforge_runner_native.dylib"
      else
        echo "$src_dir/build/libforge_runner_native.dylib"
      fi
      ;;
    Linux)
      if [[ -f "$src_dir/dist/libforge_runner_native.so" ]]; then
        echo "$src_dir/dist/libforge_runner_native.so"
      else
        echo "$src_dir/build/libforge_runner_native.so"
      fi
      ;;
    *)
      if [[ -f "$src_dir/dist/forge_runner_native.dll" ]]; then
        echo "$src_dir/dist/forge_runner_native.dll"
      else
        echo "$src_dir/build/forge_runner_native.dll"
      fi
      ;;
  esac
}

resolve_godot_bin() {
  if [[ -n "${GODOT_BIN:-}" && -x "$GODOT_BIN" ]]; then
    echo "$GODOT_BIN"
    return 0
  fi
  # Bundled via scripts/setup_godot.sh
  local bundled_macos="$REPO_ROOT/.godot-bin/Godot_mono.app/Contents/MacOS/Godot"
  if [[ -x "$bundled_macos" ]]; then
    echo "$bundled_macos"
    return 0
  fi
  local bundled_linux_x64="$REPO_ROOT/.godot-bin/Godot_v4.6-stable_mono_linux_x86_64/Godot_v4.6-stable_mono_linux_x86_64"
  if [[ -x "$bundled_linux_x64" ]]; then
    echo "$bundled_linux_x64"
    return 0
  fi
  local bundled_linux_arm64="$REPO_ROOT/.godot-bin/Godot_v4.6-stable_mono_linux_arm64/Godot_v4.6-stable_mono_linux_arm64"
  if [[ -x "$bundled_linux_arm64" ]]; then
    echo "$bundled_linux_arm64"
    return 0
  fi
  if command -v godot4 >/dev/null 2>&1; then
    command -v godot4
    return 0
  fi
  if command -v godot >/dev/null 2>&1; then
    command -v godot
    return 0
  fi
  if [[ -x "/Applications/Godot.app/Contents/MacOS/Godot" ]]; then
    echo "/Applications/Godot.app/Contents/MacOS/Godot"
    return 0
  fi
  if [[ -x "/Applications/Godot_mono.app/Contents/MacOS/Godot" ]]; then
    echo "/Applications/Godot_mono.app/Contents/MacOS/Godot"
    return 0
  fi
  if [[ -x "$HOME/Applications/Godot.app/Contents/MacOS/Godot" ]]; then
    echo "$HOME/Applications/Godot.app/Contents/MacOS/Godot"
    return 0
  fi
  if [[ -x "$HOME/Applications/Godot_mono.app/Contents/MacOS/Godot" ]]; then
    echo "$HOME/Applications/Godot_mono.app/Contents/MacOS/Godot"
    return 0
  fi
  return 1
}

try_local_mode_override() {
  if ! declare -F forge_local_handle_mode >/dev/null 2>&1; then
    return 2
  fi

  case "$MODE" in
    release|docs|test|export|app|poser|pub)
      forge_local_handle_mode "$MODE" "$@"
      return $?
      ;;
    *)
      return 2
      ;;
  esac
}

run_native_window_host() {
  local url="$1"
  if [[ ! -d "$NATIVE_HOST_DIR" ]]; then
    echo "ERROR: Native host project not found: $NATIVE_HOST_DIR" >&2
    return 1
  fi

  local host_lib
  host_lib="$(resolve_native_host_lib)"
  if [[ ! -f "$host_lib" ]]; then
    echo "ERROR: ForgeRunner.Native host library not found: $host_lib" >&2
    echo "Build it via './run.sh build-host' first." >&2
    return 1
  fi

  cp "$host_lib" "$NATIVE_HOST_DIR/" || true

  local godot_bin
  if ! godot_bin="$(resolve_godot_bin)"; then
    echo "ERROR: Godot binary not found." >&2
    echo "Run ./scripts/setup_godot.sh to download the correct version automatically." >&2
    return 1
  fi

  echo "Starting ForgeRunner.Native window host with url: $url"
  FORGE_RUNNER_URL="$url" \
  FORGE_RUNNER_APPRES_ROOT="$REPO_ROOT/ForgeRunner.Native" \
    exec "$godot_bin" --path "$NATIVE_HOST_DIR"
}

MODE="${1:-}"
if [[ -n "$MODE" ]]; then
  shift || true
fi

RUNNER_ARGS=()
if [[ -n "$MODE" && "${MODE:0:1}" == "-" ]]; then
  RUNNER_ARGS+=("$MODE" "$@")
  MODE="run"
elif [[ "$MODE" == "run" ]]; then
  RUNNER_ARGS=("$@")
fi

if [[ -z "$MODE" ]]; then
  echo "Bitte Modus wählen (Native):"
  echo "  1) none          -> ForgeRunner.Native ohne Argumente starten (letzte URL durch Runtime)"
  echo "  2) default         -> Lokales Default (file://)"
  echo "  3) designer        -> Lokales ForgeDesigner (file://)"
  echo "  4) poser           -> Lokales ForgePoser (file://)"
  echo "  5) remote-default  -> Hosted Default (AppServer)"
  echo "  6) remote-designer -> Hosted ForgeDesigner (AppServer)"
  echo "  7) docs            -> SML/SMS Docs generieren (headless Godot)"
  echo "  8) build           -> Native Stack bauen (ForgeCli.Native, ForgeRunner.Native)"
  echo "  9) build-host      -> nur ForgeRunner.Native bauen"
  echo " 10) build-mac       -> forgecli build mac (inkl. prerequisites)"
  echo " 11) build-android   -> forgecli build android (inkl. prerequisites)"
  echo " 12) build-android-autosign -> forgecli build android mit lokalem Test-Signing"
  echo " 13) test            -> Native Tests (ForgeRunner.Native)"
  echo " 14) clean           -> Native Build-Artefakte entfernen"
  echo " 15) pub             -> Lokaler Publish-Override (run.local.sh)"
  echo " 16) help            -> Hilfe anzeigen"
  read -r -p "Auswahl [1-16] (Default 1): " CHOICE || true
  CHOICE="$(printf '%s' "${CHOICE:-}" | tr -d '[:space:]')"
  if [[ -z "$CHOICE" ]]; then
    CHOICE="1"
  fi

  case "$CHOICE" in
    1|none) MODE="none" ;;
    2|default) MODE="default" ;;
    3|designer) MODE="designer" ;;
    4|poser) MODE="poser" ;;
    5|remote-default) MODE="remote-default" ;;
    6|remote-designer) MODE="remote-designer" ;;
    7|docs) MODE="docs" ;;
    8|build|build-native) MODE="build" ;;
    9|build-host|build-native-host) MODE="build-host" ;;
   10|build-mac) MODE="build-mac" ;;
   11|build-android) MODE="build-android" ;;
   12|build-android-autosign) MODE="build-android-autosign" ;;
   13|test|test-native) MODE="test" ;;
   14|clean) MODE="clean" ;;
   15|pub) MODE="pub" ;;
   16|help|-h|--help) MODE="help" ;;
    *)
      echo "Ungültige Auswahl. Abbruch."
      exit 1
      ;;
esac
fi

if try_local_mode_override "$@"; then
  exit 0
else
  local_override_rc=$?
  if [[ $local_override_rc -ne 2 ]]; then
    exit "$local_override_rc"
  fi
fi

case "$MODE" in
  run)
    native_runner_bin="$(resolve_native_runner_bin)"
    if [[ ! -x "$native_runner_bin" ]]; then
      echo "ERROR: ForgeRunner.Native executable not found: $native_runner_bin" >&2
      echo "Build it via './run.sh build-host' or set FORGE_RUNNER_NATIVE_BIN explicitly." >&2
      exit 1
    fi
    echo "Starting ForgeRunner.Native with arguments: ${RUNNER_ARGS[*]}"
    exec "$native_runner_bin" "${RUNNER_ARGS[@]}"
    ;;
  none)
    if [[ $# -gt 0 ]]; then
      echo "WARNING: mode 'none' ignores runner arguments: $*" >&2
    fi
    native_runner_bin="$(resolve_native_runner_bin)"
    if [[ ! -x "$native_runner_bin" ]]; then
      echo "ERROR: ForgeRunner.Native executable not found: $native_runner_bin" >&2
      echo "Build it via './run.sh build-host' or set FORGE_RUNNER_NATIVE_BIN explicitly." >&2
      exit 1
    fi
    echo "Starting ForgeRunner.Native (no arguments)..."
    exec "$native_runner_bin"
    ;;
  default)
    default_url="file://$REPO_ROOT/docs/Default/app.sml"
    run_native_window_host "$default_url"
    ;;
  designer)
    designer_url="file://$REPO_ROOT/docs/ForgeDesigner/app.sml"
    run_native_window_host "$designer_url"
    ;;
  remote-default)
    app_server_base_url="${FORGE_APP_SERVER_BASE_URL:-https://crowdware.github.io/Forge}"
    default_manifest_url="${app_server_base_url%/}/Default/manifest.sml"
    run_native_window_host "$default_manifest_url"
    ;;
  remote-designer)
    app_server_base_url="${FORGE_APP_SERVER_BASE_URL:-https://crowdware.github.io/Forge}"
    designer_manifest_url="${app_server_base_url%/}/ForgeDesigner/manifest.sml"
    run_native_window_host "$designer_manifest_url"
    ;;
  poser)
    poser_url="file://$REPO_ROOT/ForgePoser/app.sml"
    run_native_window_host "$poser_url"
    ;;
  url)
    if [[ $# -lt 1 ]]; then
      echo "ERROR: 'url' mode requires a URL argument, e.g. ./run.sh url http://localhost:8765/manifest.sml" >&2
      exit 1
    fi
    run_native_window_host "$1"
    ;;
  docs)
    docs_godot_bin=""
    if ! docs_godot_bin="$(resolve_godot_bin)"; then
      echo "ERROR: Godot binary not found." >&2
      echo "Run ./scripts/setup_godot.sh to download the correct version automatically." >&2
      exit 1
    fi
    echo "Generating SML element docs..."
    "$docs_godot_bin" --headless --path "$REPO_ROOT/ForgeRunner.Native/host" --script "$REPO_ROOT/tools/generate_sml_element_docs.gd" -- --out "$REPO_ROOT/docs"
    echo "Generating SMS runtime function docs..."
    "$docs_godot_bin" --headless --path "$REPO_ROOT/ForgeRunner.Native/host" --script "$REPO_ROOT/tools/generate_sms_functions_docs.gd"
    echo "Generating SML resource system docs..."
    "$docs_godot_bin" --headless --path "$REPO_ROOT/ForgeRunner.Native/host" --script "$REPO_ROOT/tools/generate_sml_resources_docs.gd"
    echo "Documentation generation completed."
    ;;
  build|build-native)
    build_native_lib "ForgeCli.Native" "$REPO_ROOT/ForgeCli.Native" "$REPO_ROOT/ForgeCli.Native/build" false
    build_forge_runner_native_host
    ;;
  build-host|build-native-host)
    build_forge_runner_native_host
    ;;
  build-mac)
    run_forgecli_build_target "mac" "$@"
    ;;
  build-android)
    export FORGE_ANDROID_BUILD_CHANNEL="release"
    run_forgecli_build_target "android" "$@"
    ;;
  build-android-autosign)
    if ! ensure_android_test_signing_env; then
      exit 1
    fi
    if ! set_autosign_android_package_id; then
      exit 1
    fi
    export FORGE_ANDROID_BUILD_CHANNEL="autosign"
    run_forgecli_build_target "android" "$@"
    ;;
  test|test-native)
    if [[ ! -f "$REPO_ROOT/ForgeRunner.Native/build/CTestTestfile.cmake" ]]; then
      echo "ERROR: ForgeRunner.Native tests are not configured. Run './run.sh build' first." >&2
      exit 1
    fi

    echo "Running ForgeRunner.Native tests..."
    ctest --test-dir "$REPO_ROOT/ForgeRunner.Native/build" --output-on-failure
    ;;
  clean)
    rm -rf "$REPO_ROOT/ForgeCli.Native/build" \
           "$REPO_ROOT/ForgeRunner.Native/build" \
           "$REPO_ROOT/ForgeRunner.Native/build-android" \
           "$REPO_ROOT/ForgeRunner.Native/dist"
    echo "Native build artifacts cleaned."
    ;;
  pin)
    if ! command -v go >/dev/null 2>&1; then
      echo "ERROR: 'go' not found in PATH. Install Go to use the pin mode." >&2
      exit 1
    fi
    go run "$REPO_ROOT/tools/pinata/main.go" pin "$@"
    ;;
  pub)
    echo "ERROR: mode 'pub' is not handled by default run.sh. Add it to run.local.sh via forge_local_handle_mode()." >&2
    exit 1
    ;;
  help|-h|--help)
    usage
    ;;
  *)
    echo "ERROR: Unknown mode '$MODE'." >&2
    usage
    exit 1
    ;;
esac

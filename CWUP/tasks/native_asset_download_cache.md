# Native Asset Download + Cache Manager

## Goal
Port `AssetCacheManager.cs` to C++ so that `ForgeRunner.Native` can fetch remote
manifests + assets over HTTP/HTTPS, verify content hashes, store them in a local
cache directory, and serve them to the UI builder - matching the C# behaviour.

## Context
`ForgeRunner.Native/src/main.cpp` (1665 lines) already has download scaffolding
and a `ManifestInfo` struct, but the persistent disk-level cache (hash verification,
metadata index, atomic writes) and the HTTP fetch integration are incomplete.

`ForgeRunner.Native/src/forge_runner_main.cpp` currently only supports `file://` URLs
and cannot fetch `http://` / `https://` manifests at all.

## Subsystems to Port

### HTTP Fetch (libcurl or Godot HTTPClient)
- Prefer `godot::HTTPClient` (already in godot-cpp) to avoid adding libcurl as a
  build dependency.
- Support progress callbacks for the splash screen progress bar.
- Retry logic: 3 attempts with exponential back-off on 5xx / network errors.

### Cache Directory
- Location: OS-appropriate cache dir (e.g. `~/.cache/forge-runner/assets/` on Linux/macOS,
  `%LOCALAPPDATA%\ForgeRunner\cache\` on Windows).
- Index file: `cache/index.sml` - one entry per asset:
  `url`, `hash` (SHA-256 hex), `local_file`, `last_fetched`.
- Atomic writes: write to `.tmp` → fsync → rename.

### Hash Verification
- SHA-256 of downloaded bytes must match the manifest-provided hash.
- On mismatch: delete cached file, re-download once, hard-error if still mismatching.

### Asset Resolver Integration
- `RunnerUriResolver` in C#: maps `res://` paths to either:
  a) Files in the local cache dir after download, or
  b) Files relative to `FORGE_RUNNER_APPRES_ROOT` (local dev mode).
- Port to `UiBuilder::resolve_asset_path()` in `forge_ui_builder.cpp`.

### Manifest Loader
- Parse manifest SML: `files:` list with `url` + `hash` + `path` entries.
- Determine which files need downloading (missing or hash mismatch).
- Download only the delta; show per-file progress.

## Acceptance Criteria
- `ForgeRunner.Native` can start with `FORGE_RUNNER_URL=https://...` and display
  the app after downloading + caching all required assets.
- On second run, no network requests are made if hashes match.
- Hash mismatch forces re-download.
- Progress is forwarded to the splash-screen progress bar.

## Implementation Status

- [x] `forge_asset_cache.h` - API skeleton (AssetCache: resolve, store, sha256_of)
- [x] `forge_asset_cache.cpp` - Full implementation:
  - `forge_cache_dir()` (OS-appropriate: XDG/HOME/.cache/forge-runner, LOCALAPPDATA)
  - `AssetCache::resolve()` with SHA-256 prefix normalization
  - `AssetCache::store()` - atomic write + flat URL→hash-named index
  - `AssetCache::sha256_of()` - via Godot `HashingContext`
  - `load_index()` / `save_index()` - SML-format CacheIndex with atomic rename
- [x] `forge_runner_main.cpp` - HTTP manifest support:
  - `is_http_url()` - detect `http://` / `https://` URLs
  - `show_loading_screen()` - built-in loading UI with ProgressBar
  - `start_manifest_download()` - via Godot `HTTPRequest` node
  - `on_manifest_downloaded()` - smlcore parse, manifest SHA check, delta list
  - `start_next_asset_download()` - sequential asset download with hash-delta skip
  - `on_asset_downloaded()` - atomic file save with 2 retry attempts on error
  - `on_all_assets_ready()` - metadata save + `show_sml(entry_path)`
  - `load_manifest_meta()` / `save_manifest_meta()` - per-app metadata.sml
  - `normalize_asset_path()` - security: reject `..` traversal
  - `save_file_atomic()` - write .tmp → rename
- [x] `CMakeLists.txt` - forge_asset_cache.cpp added to GDExtension sources
- [ ] Tests: mockup web server integration test

### Architecture Notes
- Per-manifest cache: `~/.cache/forge-runner/{sha256_of_url}/files/{asset.path}`
  - Preserves original directory structure → relative SML references work
- Flat URL cache (`AssetCache`): used for individual ad-hoc URL assets
- Delta download: second run skips unchanged files (manifest SHA + per-asset hash check)
- `HTTPRequest` node used (Godot-native, no libcurl dependency)

## Test Strategy - Mockup Web Server

For unit / integration tests a lightweight mock HTTP server should be set up
that serves static SML files and assets without requiring network access:

- Use Python's built-in `http.server` (one-liner) or a tiny C++ or Go server.
- Start it on a random port before each test run; shut it down after.
- Serve a fixture directory (`tests/fixtures/remote_app/`) containing:
  - `app.sml` - a minimal SML document
  - `manifest.sml` - a manifest referencing a small image and a CSS-like layout
  - `icon.png` - small test asset
- Test cases:
  1. First run: all files are downloaded and cached (index populated).
  2. Second run: no HTTP requests made (hashes match).
  3. Hash mismatch: stale cache entry is evicted and re-downloaded.
  4. Server unavailable: graceful error message, no crash.
- The server URL is passed via `FORGE_RUNNER_URL=http://localhost:{port}/app.sml`.

## Reference
- C#: `ForgeRunner/Runtime/Assets/AssetCacheManager.cs`
- C#: `ForgeRunner/Runtime/Assets/RunnerUriResolver.cs`
- C#: `ForgeRunner/Runtime/Manifest/ManifestLoader.cs`
- C++: `ForgeRunner.Native/src/main.cpp` (existing scaffold)

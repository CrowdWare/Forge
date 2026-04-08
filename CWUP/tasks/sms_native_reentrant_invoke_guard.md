# SMS Engine: Re-entrancy Guard for `sms_native_session_invoke`

## Status
**Ready for implementation.**
Failing (RED) test in `ForgeRunner.Native/tests/sms_runner_integration_tests.cpp`:
`runner_integration_setKeyframe_cycle_unprotected`

---

## The Bug

`sms_native_session_invoke` can be called re-entrantly from within its own
call stack via UI invoke callbacks. When this happens the engine recurses
without limit on the C stack, causing an OS-level stack overflow before any
SMS-level error is produced.

### Crash scenario (ForgePoser - reproduced by the RED test)

```
sms_native_session_invoke("editor", "poseChanged", ...)
  └─ SMS executes: timeline.setKeyframe(1, null)
       └─ C++ mock/SmsBridge: setKeyframe() emits keyframeAdded signal
            └─ sms_native_session_invoke("timeline", "keyframeAdded", ...)  ← re-entrant!
                 └─ SMS executes: editorRef.poseChanged(bone)
                      └─ C++ mock/SmsBridge: poseChanged() emits poseChanged
                           └─ sms_native_session_invoke("editor", "poseChanged", ...) ← re-entrant!
                                └─ ... infinite recursion on the C stack ...
zsh: abort  ./run.sh poser
```

---

## What to implement

Add a **per-session re-entrancy guard** to `sms_native_session_invoke` in
`SMSCore.Native/src/sms_native.cpp`.

### Option A - per-session flag (recommended)

Each `Session` object already exists at the call site. Add a `bool invoking`
member, set it on entry, clear it on exit, and return an error if it is
already set when a new invocation arrives:

```cpp
// In the Session struct (sms_native.cpp):
struct Session {
    // ... existing fields ...
    bool invoking = false;   // re-entrancy guard
};

// In sms_native_session_invoke:
int sms_native_session_invoke(int64_t session_id, ...) {
    Session* s = find_session(session_id);
    if (!s) { /* existing error */ }

    if (s->invoking) {
        // Another invocation is already on the C stack for this session.
        // Return a classifiable error instead of recursing.
        snprintf(out_error, out_error_cap,
            "RuntimeError: re-entrant sms_native_session_invoke for session %" PRId64
            " - UI callback triggered a new dispatch while a handler was executing "
            "(possible infinite loop in event handler).", session_id);
        return 1;
    }

    s->invoking = true;
    // ... existing invoke logic ...
    s->invoking = false;
    return rc;
}
```

Use RAII to guarantee the flag is cleared even on exception:

```cpp
struct InvokeGuard {
    bool& flag;
    explicit InvokeGuard(bool& f) : flag(f) { flag = true; }
    ~InvokeGuard() { flag = false; }
};

// Usage:
InvokeGuard guard(s->invoking);
// ... rest of invoke ...
```

### Option B - thread_local depth counter (acceptable alternative)

If per-session state is inconvenient, a `thread_local int` works as long as
invocations are single-threaded (which they are):

```cpp
thread_local int sms_invoke_depth = 0;
constexpr int kMaxSmsInvokeDepth  = 1;   // strict: no re-entrancy at all

if (sms_invoke_depth >= kMaxSmsInvokeDepth) {
    snprintf(out_error, out_error_cap,
        "RuntimeError: re-entrant sms_native_session_invoke ...");
    return 1;
}
++sms_invoke_depth;
// ... invoke ...
--sms_invoke_depth;
```

Option A is preferred because it is scoped to one session and will not
interfere if multi-session use is ever added.

---

## Error string requirements

The error string written to `out_error` **must** contain `"RuntimeError:"` so
that `forge::sms_error_requires_exit()` classifies it as a fatal error and
ForgeRunner.Native terminates cleanly instead of entering an undefined state.

```cpp
// forge_sms_error_policy.cpp - already handles this:
bool sms_error_requires_exit(const std::string& message) {
    if (message.find("RuntimeError:") != std::string::npos) return true;
    if (message.find("Stack overflow") != std::string::npos) return true;
    return false;
}
```

---

## Files to modify

| File | Change |
|---|---|
| `SMSCore.Native/src/sms_native.cpp` | Add `bool invoking` to `Session`; add re-entrancy check + RAII guard at the top of `sms_native_session_invoke` |

**No other files need to change.**

---

## Verification

After the fix, run:

```bash
./run.sh build && ./run.sh test
```

Expected:
- `runner_integration_setKeyframe_cycle_unprotected` → **PASSED** (was RED)
- All other tests → **PASSED** (must not regress)

The full individual test can also be run directly:

```bash
cd ForgeRunner.Native/build
ctest -R runner_integration_setKeyframe_cycle_unprotected --output-on-failure
```

---

## Why the existing `kMaxSmsDispatchDepth` guard is not sufficient

`SmsBridge::dispatch_event` in `forge_runner_extension.cpp` has a
`thread_local int dispatch_depth` guard limited to 256. This catches
runaway cascades at the bridge level, but:

1. The guard lives in **ForgeRunner.Native** (Godot extension), not in the
   SMS engine itself - so any direct caller of `sms_native_session_invoke`
   (test harnesses, future hosts) bypasses it.
2. 256 levels of `sms_native_session_invoke` frames on the C stack can
   overflow the OS stack before the counter reaches 256, because each
   invocation frame is not lightweight.

The engine-level guard in Option A above fires on the **second** re-entrant
call (depth 1 → reject), eliminating the problem at the root.

---

## Acceptance Criteria

- `runner_integration_setKeyframe_cycle_unprotected` passes (GREEN).
- All existing tests remain GREEN (no regression).
- The error string produced by the guard contains `"RuntimeError:"`.
- The `Session::invoking` flag is reset even if an exception propagates
  (RAII guard or equivalent).

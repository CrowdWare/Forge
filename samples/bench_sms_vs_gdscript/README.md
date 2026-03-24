# SMS Native vs GDScript Benchmark

This benchmark compares:

- SMS compiled to native (`ForgeCli.Native` -> executable)
- GDScript executed in Godot runtime (`--headless --script`)
- C# (`dotnet`, Release, same workload)

Both paths use the same integer-heavy workload and print:

- `RESULT:<value>`
- `TIME_US:<microseconds>`

## Files

- `sms_bench.sms`
- `gdscript_bench.gd`
- `csharp_bench/Program.cs`
- `run_compare.sh`

## Run

```bash
cd samples/bench_sms_vs_gdscript
./run_compare.sh
```

Custom run count / warmup:

```bash
RUNS=20 WARMUP=5 ./run_compare.sh
```

If your Godot binary is not auto-detected, set:

```bash
GODOT_BIN=/absolute/path/to/godot ./run_compare.sh
```

If your Forge/SMS binaries are not in default build locations, set:

```bash
FORGECLI_BIN=/abs/path/forgecli-native \
SMS_NATIVE_LIB_DIR=/abs/path/SMSCore.Native/build \
GODOT_BIN=/abs/path/to/godot \
./run_compare.sh
```

If `dotnet` is in a custom location:

```bash
DOTNET_BIN=/abs/path/to/dotnet ./run_compare.sh
```

## Fairness Notes

- Run in release builds.
- Use the same device for both runs.
- The script reports median and p95 over multiple runs.
- Keep thermal conditions stable (same power mode, low background activity).

# benchmark_portrait_demo

Portrait benchmark demo for local `ForgeRunner.Native`.

It validates native benchmark callbacks from SMS:

- `benchmark.start(name)`
- `on benchmark.progress(name, percent, stage)`
- `on benchmark.completed(name, durationMs, smsScore, kotlinScore, summary)`

Run:

```bash
./run.sh url "file://$(pwd)/samples/benchmark_portrait_demo/app.sml"
```

# mac_demo_aot

Small local smoke app to validate the SMS AOT path in `ForgeRunner.Native`.

## Run

```bash
./run.sh url "file://$(pwd)/samples/mac_demo_aot/app.sml"
```

## Expected

- Runner starts local app.
- Console shows: `SMS AOT active for local session ...`
- App logs: `The answer of all questions is: 42`

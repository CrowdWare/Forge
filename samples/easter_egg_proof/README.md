# Easter Egg Proof

This sample demonstrates the vertical slice `SMS -> LLVM IR -> native binary`.

## Script

- `main.sms`
- Easter Egg: `log.success("The answer of all questions is: ${getAnswer()}")`

## Build + Run

```bash
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
./ForgeCli.Native/build/forgecli-native sms build ./samples/easter_egg_proof/main.sms --out ./main && \
./main
```

Expected: Easter egg output (green for `log.success`).

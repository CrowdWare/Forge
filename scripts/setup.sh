#!/usr/bin/env bash
# Installs the forgecli shim to ~/.local/bin/forgecli.
# Run once per machine after cloning. Every Forge repo clone is then
# usable independently — forgecli auto-detects which clone you're in.
set -euo pipefail

INSTALL_DIR="$HOME/.local/bin"
INSTALL_PATH="$INSTALL_DIR/forgecli"

mkdir -p "$INSTALL_DIR"

# Remove any existing file or symlink before writing the shim.
rm -f "$INSTALL_PATH"

cat > "$INSTALL_PATH" << 'SHIM'
#!/usr/bin/env bash
# Static shim — never needs updating. Walks up from $PWD to find the
# scripts/forgecli_native.sh of whichever Forge clone you're working in.
dir="$PWD"
while [[ "$dir" != "/" ]]; do
  if [[ -f "$dir/scripts/forgecli_native.sh" ]]; then
    exec bash "$dir/scripts/forgecli_native.sh" "$@"
  fi
  dir="$(dirname "$dir")"
done
echo "ERROR: Not inside a Forge project (scripts/forgecli_native.sh not found)." >&2
exit 1
SHIM

chmod +x "$INSTALL_PATH"
echo "[OK] forgecli installed to $INSTALL_PATH"

# Warn if ~/.local/bin is not in PATH
if ! echo "$PATH" | tr ':' '\n' | grep -qx "$INSTALL_DIR"; then
  echo ""
  echo "[WARN] $INSTALL_DIR is not in your PATH."
  echo "  Add this to your shell profile (~/.zshrc or ~/.bashrc):"
  echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
fi

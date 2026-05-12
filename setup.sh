#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WRAPPER="$REPO_ROOT/scripts/forgecli_native.sh"
INSTALL_DIR="$HOME/.local/bin"
LINK="$INSTALL_DIR/forgecli"

echo "Forge setup"
echo "==========="

# ── 1. Build ForgeCli.Native (cmake handles incremental rebuilds) ────────────
CLI_BIN="$REPO_ROOT/ForgeCli.Native/build/forgecli-native"

echo "[1/3] Building ForgeCli.Native..."
if ! command -v cmake >/dev/null 2>&1; then
  echo "ERROR: cmake not found. Install cmake and re-run setup.sh." >&2
  exit 1
fi
cmake -S "$REPO_ROOT/ForgeCli.Native" \
      -B "$REPO_ROOT/ForgeCli.Native/build" \
      -DCMAKE_BUILD_TYPE=Release \
      --log-level=ERROR
cmake --build "$REPO_ROOT/ForgeCli.Native/build" --config Release
echo "  Ready: $CLI_BIN"

# ── 2. Install forgecli wrapper to ~/.local/bin ──────────────────────────────
echo "[2/3] Installing forgecli to $INSTALL_DIR ..."
mkdir -p "$INSTALL_DIR"
chmod +x "$WRAPPER"

if [[ -L "$LINK" || -e "$LINK" ]]; then
  rm "$LINK"
fi
ln -s "$WRAPPER" "$LINK"
echo "  Linked: $LINK -> $WRAPPER"

# ── 3. Add ~/.local/bin to PATH if missing ───────────────────────────────────
echo "[3/3] Checking PATH..."

SHELL_RC=""
if [[ -f "$HOME/.zshrc" ]]; then
  SHELL_RC="$HOME/.zshrc"
elif [[ -f "$HOME/.bashrc" ]]; then
  SHELL_RC="$HOME/.bashrc"
fi

PATH_LINE='export PATH="$HOME/.local/bin:$PATH"'

if echo "$PATH" | tr ':' '\n' | grep -qx "$INSTALL_DIR"; then
  echo "  ~/.local/bin already in PATH."
elif [[ -n "$SHELL_RC" ]] && grep -qF "$INSTALL_DIR" "$SHELL_RC" 2>/dev/null; then
  echo "  ~/.local/bin already in $SHELL_RC."
else
  if [[ -n "$SHELL_RC" ]]; then
    echo "" >> "$SHELL_RC"
    echo "# Added by Forge setup.sh" >> "$SHELL_RC"
    echo "$PATH_LINE" >> "$SHELL_RC"
    echo "  Added to $SHELL_RC — run: source $SHELL_RC"
  else
    echo "  Add this line to your shell config manually:"
    echo "  $PATH_LINE"
  fi
fi

# ── Done ─────────────────────────────────────────────────────────────────────
echo ""
echo "Done. Start a new terminal (or run 'source ~/.zshrc') then:"
echo "  forgecli --help"

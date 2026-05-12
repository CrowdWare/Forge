#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
WRAPPER="$REPO_ROOT/scripts/forgecli_native.sh"
INSTALL_DIR="${FORGECLI_INSTALL_DIR:-/usr/local/bin}"
LINK="$INSTALL_DIR/forgecli"

chmod +x "$WRAPPER"

if [[ -e "$LINK" || -L "$LINK" ]]; then
  echo "Removing existing $LINK"
  rm "$LINK"
fi

ln -s "$WRAPPER" "$LINK"
echo "Installed: forgecli -> $LINK"
echo "Run 'forgecli --help' to verify."

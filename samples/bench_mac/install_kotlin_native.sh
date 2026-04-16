#!/usr/bin/env bash
# Install Kotlin Native on macOS (aarch64)
set -euo pipefail

INSTALL_DIR="/usr/local/kotlin-native"

echo "Fetching latest Kotlin Native release..."
URL=$(curl -s https://api.github.com/repos/JetBrains/kotlin/releases/latest \
    | grep "browser_download_url.*kotlin-native-prebuilt-macos-aarch64.*\.tar\.gz\"" \
    | cut -d'"' -f4)

FILENAME=$(basename "$URL")
echo "Downloading $FILENAME..."
curl -L -O "$URL"

echo "Extracting..."
tar xzf "$FILENAME"

DIRNAME="${FILENAME%.tar.gz}"
echo "Installing to $INSTALL_DIR..."
sudo mv "$DIRNAME" "$INSTALL_DIR"
rm "$FILENAME"

EXPORT_LINE='export PATH="/usr/local/kotlin-native/bin:$PATH"'
if ! grep -q "kotlin-native" ~/.zshrc 2>/dev/null; then
    echo "$EXPORT_LINE" >> ~/.zshrc
    echo "Added to ~/.zshrc"
fi

export PATH="/usr/local/kotlin-native/bin:$PATH"
echo ""
echo "Done! Version:"
konanc -version

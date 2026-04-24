#!/usr/bin/env bash
# Installs the nuBASIC VS Code extension on macOS or Linux.
# Safe to run even if VS Code is not installed (exits silently).
set -euo pipefail

LOG="${TMPDIR:-/tmp}/nubasic-vscode-install.log"
exec > >(tee -a "$LOG") 2>&1

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
VSIX="${SCRIPT_DIR}/nubasic-0.1.2.vsix"

if [ ! -f "$VSIX" ]; then
    echo "VSIX not found at: $VSIX — skipping VS Code extension installation."
    exit 0
fi

# Locate VS Code CLI (code or code-insiders)
CODE_CLI=""
for candidate in code code-insiders; do
    if command -v "$candidate" >/dev/null 2>&1; then
        CODE_CLI="$candidate"
        break
    fi
done

# macOS: also check the application bundle when not on PATH
if [ -z "$CODE_CLI" ] && [ "$(uname -s)" = "Darwin" ]; then
    for app_dir in \
        "/Applications/Visual Studio Code.app" \
        "$HOME/Applications/Visual Studio Code.app"
    do
        bundle_cli="${app_dir}/Contents/Resources/app/bin/code"
        if [ -f "$bundle_cli" ]; then
            CODE_CLI="$bundle_cli"
            break
        fi
    done
fi

if [ -z "$CODE_CLI" ]; then
    echo "VS Code CLI not found — skipping extension installation."
    echo "To install manually run:"
    echo "  code --install-extension \"${VSIX}\" --force"
    exit 0
fi

echo "Installing nuBASIC VS Code extension via: $CODE_CLI"
"$CODE_CLI" --install-extension "$VSIX" --force
echo "VS Code extension installation complete."

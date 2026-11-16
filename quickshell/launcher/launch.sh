#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"

# Try toggling existing instance via Quickshell IPC if supported.
if command -v qs >/dev/null 2>&1; then
    if qs ipc -c "$DIR" call launcher toggle >/dev/null 2>&1; then
        exit 0
    fi
    # Some versions use: qs -c <name> ipc call ...
    if qs -c "$DIR" ipc call launcher toggle >/dev/null 2>&1; then
        exit 0
    fi
    exec qs -c "$DIR"
elif command -v quickshell >/dev/null 2>&1; then
    if quickshell ipc -c "$DIR" call launcher toggle >/dev/null 2>&1; then
        exit 0
    fi
    exec quickshell -c "$DIR"
fi

# Fallback: no quickshell installed. Notify user.
if command -v notify-send >/dev/null 2>&1; then
    notify-send "Launcher" "Quickshell is not installed. Launcher scaffold is at $DIR" 2>/dev/null || true
fi
exit 1

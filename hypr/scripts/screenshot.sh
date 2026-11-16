#!/usr/bin/env bash
# Screenshot helper for Hyprland using grim, slurp, and wl-copy

MODE="${1:-area}"
DIR="$HOME/Pictures/Screenshots"
if ! mkdir -p "$DIR" 2>/dev/null; then
    DIR="/tmp/Screenshots"
    mkdir -p "$DIR" 2>/dev/null
fi

TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
FILE="$DIR/Screenshot_${TIMESTAMP}.png"

case "$MODE" in
    full)
        grim "$FILE"
        ;;
    window)
        GEOM=$(hyprctl activewindow -j 2>/dev/null | python3 -c "
import sys, json
try:
    d = json.load(sys.stdin)
    x, y = d['at']
    w, h = d['size']
    print(f'{x},{y} {w}x{h}')
except Exception:
    pass
")
        if [ -n "$GEOM" ]; then
            grim -g "$GEOM" "$FILE"
        else
            grim "$FILE"
        fi
        ;;
    area|*)
        GEOM=$(slurp -b 16181d80 -c 7a8c9e -d)
        if [ -z "$GEOM" ]; then
            # Selection cancelled
            exit 0
        fi
        grim -g "$GEOM" "$FILE"
        ;;
esac

if [ -f "$FILE" ]; then
    wl-copy < "$FILE"
    notify-send -a "Screen Capture" -i "$FILE" "Screenshot Captured" "Saved to $(basename "$FILE") & copied to clipboard"
fi

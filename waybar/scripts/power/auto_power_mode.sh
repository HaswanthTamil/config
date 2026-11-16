#!/usr/bin/env bash
# Automatic power mode: performance on AC, power-saver on battery.
# Respects manual override stored in state file.
set -u

STATE_DIR="$HOME/.config/waybar/state"
STATE_FILE="$STATE_DIR/power_mode.json"
mkdir -p "$STATE_DIR"

ONCE=0
[ "${1:-}" = "--once" ] && ONCE=1

get_mode_pref() {
    python3 - "$STATE_FILE" <<'PY' 2>/dev/null || echo auto
import json, sys, os
f = sys.argv[1]
if os.path.exists(f):
    try:
        d = json.load(open(f))
        m = d.get("mode", "auto")
        print(m if m in ("auto","power-saver","balanced","performance") else "auto")
    except Exception:
        print("auto")
else:
    print("auto")
PY
}

ac_online() {
    for a in /sys/class/power_supply/AC* /sys/class/power_supply/ACAD* /sys/class/power_supply/ADP* /sys/class/power_supply/Mains*; do
        [ -e "$a/online" ] && [ "$(cat "$a/online" 2>/dev/null)" = "1" ] && return 0
    done
    return 1
}

apply_target() {
    local pref target
    pref=$(get_mode_pref)
    if [ "$pref" = "auto" ]; then
        if ac_online; then target="performance"; else target="power-saver"; fi
    else
        target="$pref"
    fi
    "$HOME/.config/waybar/scripts/power/power_mode_apply.sh" "$target" >/dev/null 2>&1 || true
}

apply_target
[ "$ONCE" -eq 1 ] && exit 0

# Watch for power supply changes via udev events when available, else poll.
if command -v udevadm >/dev/null 2>&1; then
    udevadm monitor --subsystem-match=power_supply --property 2>/dev/null | while read -r _; do
        apply_target
    done &
    MON_PID=$!
    trap 'kill "$MON_PID" 2>/dev/null' EXIT
fi

while sleep 5; do
    apply_target
done

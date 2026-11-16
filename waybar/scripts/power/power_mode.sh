#!/usr/bin/env bash
# Set power mode preference: auto or an explicit profile.
# Explicit profiles act as manual override until set back to auto.
set -u

MODE="${1:-}"
case "$MODE" in
    auto|power-saver|balanced|performance) ;;
    *) echo "Usage: $0 {auto|power-saver|balanced|performance}" >&2; exit 2 ;;
esac

STATE_DIR="$HOME/.config/waybar/state"
STATE_FILE="$STATE_DIR/power_mode.json"
mkdir -p "$STATE_DIR"

printf '{"mode":"%s"}\n' "$MODE" > "$STATE_FILE"

notify() {
    command -v notify-send >/dev/null 2>&1 && \
        notify-send -a "Power Mode" "Power mode: $MODE" 2>/dev/null || true
}

if [ "$MODE" = "auto" ]; then
    # Apply immediately based on current AC state.
    "$HOME/.config/waybar/scripts/power/auto_power_mode.sh" --once >/dev/null 2>&1 || true
    # Run one-shot apply inline (auto_power_mode loops by default).
    for a in /sys/class/power_supply/AC* /sys/class/power_supply/ACAD* /sys/class/power_supply/ADP* /sys/class/power_supply/Mains*; do
        if [ -e "$a/online" ] && [ "$(cat "$a/online" 2>/dev/null)" = "1" ]; then
            "$HOME/.config/waybar/scripts/power/power_mode_apply.sh" performance >/dev/null 2>&1 || true
            notify
            pkill -RTMIN+11 waybar 2>/dev/null || true
            exit 0
        fi
    done
    "$HOME/.config/waybar/scripts/power/power_mode_apply.sh" power-saver >/dev/null 2>&1 || true
    notify
    pkill -RTMIN+11 waybar 2>/dev/null || true
    exit 0
fi

"$HOME/.config/waybar/scripts/power/power_mode_apply.sh" "$MODE" >/dev/null 2>&1
rc=$?
if [ $rc -ne 0 ]; then
    # As a last resort for explicit manual choice, try pkexec helper if available.
    HELPER="$HOME/.config/waybar/scripts/power/power_mode_root.sh"
    if [ -x "$HELPER" ] && command -v pkexec >/dev/null 2>&1; then
        pkexec "$HELPER" "$MODE" >/dev/null 2>&1 && rc=0 || rc=1
    fi
fi

pkill -RTMIN+11 waybar 2>/dev/null || true
if [ $rc -eq 0 ]; then
    notify
    exit 0
fi

command -v notify-send >/dev/null 2>&1 && \
    notify-send -a "Power Mode" "Could not change power mode" "Needs power-profiles-daemon or write access to power sysfs." 2>/dev/null || true
exit 1

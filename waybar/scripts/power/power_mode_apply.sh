#!/usr/bin/env bash
# Apply a power profile without interactive prompts. Best-effort.
set -u

MODE="${1:-}"
case "$MODE" in
    power-saver|balanced|performance) ;;
    *) exit 2 ;;
esac

refresh_waybar() {
    pkill -RTMIN+11 waybar 2>/dev/null || true
}

# Prefer power-profiles-daemon (usually allowed for active sessions).
if command -v powerprofilesctl >/dev/null 2>&1; then
    if powerprofilesctl set "$MODE" >/dev/null 2>&1; then
        refresh_waybar
        exit 0
    fi
fi

# Try direct sysfs writes if permitted (no password prompt).
case "$MODE" in
    power-saver) PP="quiet";       GOV="powersave" ;;
    balanced)    PP="balanced";    GOV="schedutil" ;;
    performance) PP="performance"; GOV="performance" ;;
esac

changed=0
if [ -w /sys/firmware/acpi/platform_profile ]; then
    printf '%s\n' "$PP" > /sys/firmware/acpi/platform_profile 2>/dev/null && changed=1
fi
for g in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    [ -w "$g" ] && printf '%s\n' "$GOV" > "$g" 2>/dev/null && changed=1
done

refresh_waybar
[ "$changed" -eq 1 ] && exit 0
exit 1

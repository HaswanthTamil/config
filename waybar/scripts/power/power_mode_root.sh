#!/usr/bin/env bash
# Root-only helper invoked through pkexec.
set -u

PROFILE_FILE="/sys/firmware/acpi/platform_profile"
GOVERNOR_FILE="/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"

case "${1:-}" in
    power-saver) PP="quiet";       GOV="powersave" ;;
    balanced)    PP="balanced";    GOV="powersave" ;;
    performance) PP="performance"; GOV="performance" ;;
    *) exit 2 ;;
esac

changed=0
if [ -w "$PROFILE_FILE" ]; then
    printf '%s\n' "$PP" > "$PROFILE_FILE" 2>/dev/null && changed=1
fi
if [ -w "$GOVERNOR_FILE" ]; then
    printf '%s\n' "$GOV" > "$GOVERNOR_FILE" 2>/dev/null && changed=1
fi

[ "$changed" -eq 1 ] || exit 1
exit 0

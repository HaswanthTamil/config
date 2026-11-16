#!/usr/bin/env bash
# Battery capsule indicator. Shows charging/plugged state and updates frequently.

BAT=""
for b in /sys/class/power_supply/BAT*; do
    [ -e "$b/capacity" ] && BAT="$b" && break
done

AC_ONLINE=""
for a in /sys/class/power_supply/AC* /sys/class/power_supply/ACAD* /sys/class/power_supply/ADP* /sys/class/power_supply/Mains*; do
    [ -e "$a/online" ] && AC_ONLINE=$(cat "$a/online" 2>/dev/null) && break
done

if [ -n "$BAT" ]; then
    CAP=$(cat "$BAT/capacity" 2>/dev/null)
    STATUS=$(cat "$BAT/status" 2>/dev/null)
    [ -z "$CAP" ] && CAP="?"

    ICON="󰁹"
    PLUG=""
    if [ "$STATUS" = "Charging" ]; then
        ICON="󰂄"
        PLUG="󰚥 "
    elif [ "$STATUS" = "Full" ] || [ "$STATUS" = "Not charging" ]; then
        # "Not charging" on AC usually means plugged in at charge threshold.
        if [ "$AC_ONLINE" = "1" ]; then
            ICON="󰁹"
            PLUG="󰚥 "
            [ "$STATUS" = "Not charging" ] && STATUS="Plugged in"
        fi
    elif [ "$AC_ONLINE" = "1" ]; then
        PLUG="󰚥 "
    fi

    CLASS=$(echo "$STATUS" | tr '[:upper:]' '[:lower:]' | tr ' ' '-')
    echo "{\"text\": \"${PLUG}${ICON} ${CAP}\", \"class\": \"${CLASS}\", \"tooltip\": \"Battery: ${CAP}% (${STATUS})\"}"
else
    echo "{\"text\": \"󰚥 󰐥\", \"class\": \"ac\", \"tooltip\": \"AC Power Online\"}"
fi

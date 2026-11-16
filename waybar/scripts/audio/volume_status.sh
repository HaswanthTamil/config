#!/usr/bin/env bash
# Minimal pure-icon volume status for Waybar matching Image 1

VOL=""
MUTED=false

if command -v wpctl >/dev/null 2>&1; then
    WP_OUT=$(wpctl get-volume @DEFAULT_AUDIO_SINK@ 2>/dev/null)
    if [ -n "$WP_OUT" ]; then
        echo "$WP_OUT" | grep -q "\[MUTED\]" && MUTED=true
        RAW_VOL=$(echo "$WP_OUT" | awk '{print $2}')
        [ -n "$RAW_VOL" ] && VOL=$(awk -v v="$RAW_VOL" 'BEGIN {printf "%.0f", v * 100}')
    fi
fi

if [ -z "$VOL" ] && command -v pactl >/dev/null 2>&1; then
    pactl get-sink-mute @DEFAULT_SINK@ 2>/dev/null | grep -q "yes" && MUTED=true
    P_VOL=$(pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null | head -n 1 | awk '{print $5}' | tr -d '%')
    [ -n "$P_VOL" ] && VOL="$P_VOL"
fi

[ -z "$VOL" ] && VOL="50"

if [ "$MUTED" = true ]; then
    echo "{\"text\": \"󰖁\", \"class\": \"muted\", \"tooltip\": \"Audio Output: Muted ($VOL%)\"}"
else
    echo "{\"text\": \"󰕾\", \"class\": \"normal\", \"tooltip\": \"Audio Output: $VOL%\"}"
fi

#!/usr/bin/env bash
# Audio output volume indicator for Waybar Right Zone
# Supports PipeWire (wpctl), PulseAudio (pactl), and ALSA (amixer) with graceful fallback

VOL=""
MUTED=false

# 1. Try wpctl (PipeWire / WirePlumber)
if command -v wpctl >/dev/null 2>&1; then
    WP_OUT=$(wpctl get-volume @DEFAULT_AUDIO_SINK@ 2>/dev/null)
    if [ -n "$WP_OUT" ]; then
        if echo "$WP_OUT" | grep -q "\[MUTED\]"; then
            MUTED=true
        fi
        RAW_VOL=$(echo "$WP_OUT" | awk '{print $2}')
        if [ -n "$RAW_VOL" ]; then
            VOL=$(awk -v v="$RAW_VOL" 'BEGIN {printf "%.0f", v * 100}')
        fi
    fi
fi

# 2. Try pactl
if [ -z "$VOL" ] && command -v pactl >/dev/null 2>&1; then
    P_MUTE=$(pactl get-sink-mute @DEFAULT_SINK@ 2>/dev/null | awk '{print $2}')
    [ "$P_MUTE" = "yes" ] && MUTED=true
    P_VOL=$(pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null | head -n 1 | awk '{print $5}' | tr -d '%')
    [ -n "$P_VOL" ] && VOL="$P_VOL"
fi

# 3. Try amixer (ALSA)
if [ -z "$VOL" ] && command -v amixer >/dev/null 2>&1; then
    ALSA_OUT=$(amixer get Master 2>/dev/null)
    if [ -n "$ALSA_OUT" ]; then
        echo "$ALSA_OUT" | grep -q "\[off\]" && MUTED=true
        A_VOL=$(echo "$ALSA_OUT" | grep -o '\[[0-9]*%\]' | head -n 1 | tr -d '[]%')
        [ -n "$A_VOL" ] && VOL="$A_VOL"
    fi
fi

# Default fallback if sound system isn't running
[ -z "$VOL" ] && VOL="50"

if [ "$MUTED" = true ]; then
    ICON="󰖁"
    CLASS="muted"
    echo "{\"text\": \"$ICON $VOL%\", \"class\": \"$CLASS\", \"tooltip\": \"Audio Output: Muted ($VOL%)\\nClick: Mute/Unmute | Scroll: Volume | Right: Open Audio Panel\"}"
else
    ICON="󰕾"
    CLASS="normal"
    echo "{\"text\": \"$ICON $VOL%\", \"class\": \"$CLASS\", \"tooltip\": \"Audio Output: $VOL%\\nClick: Mute/Unmute | Scroll: Volume | Right: Open Audio Panel\"}"
fi

#!/usr/bin/env bash
# Audio volume and mute control helper
TARGET="$1"   # "output" or "input"
ACTION="$2"   # "up", "down", "mute", or a specific number 0-100

if [ "$TARGET" = "output" ]; then
    case "$ACTION" in
        up)
            wpctl set-volume -l 1.5 @DEFAULT_AUDIO_SINK@ 5%+ 2>/dev/null || pactl set-sink-volume @DEFAULT_SINK@ +5% 2>/dev/null
            ;;
        down)
            wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%- 2>/dev/null || pactl set-sink-volume @DEFAULT_SINK@ -5% 2>/dev/null
            ;;
        mute)
            wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle 2>/dev/null || pactl set-sink-mute @DEFAULT_SINK@ toggle 2>/dev/null
            ;;
        set)
            VAL="$3"
            VAL_FRAC=$(awk -v v="$VAL" 'BEGIN {printf "%.2f", v / 100}')
            wpctl set-volume @DEFAULT_AUDIO_SINK@ "$VAL_FRAC" 2>/dev/null || pactl set-sink-volume @DEFAULT_SINK@ "${VAL}%" 2>/dev/null
            ;;
    esac
elif [ "$TARGET" = "input" ]; then
    case "$ACTION" in
        up)
            wpctl set-volume -l 1.5 @DEFAULT_AUDIO_SOURCE@ 5%+ 2>/dev/null || pactl set-source-volume @DEFAULT_SOURCE@ +5% 2>/dev/null
            ;;
        down)
            wpctl set-volume @DEFAULT_AUDIO_SOURCE@ 5%- 2>/dev/null || pactl set-source-volume @DEFAULT_SOURCE@ -5% 2>/dev/null
            ;;
        mute)
            wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle 2>/dev/null || pactl set-source-mute @DEFAULT_SOURCE@ toggle 2>/dev/null
            ;;
        set)
            VAL="$3"
            VAL_FRAC=$(awk -v v="$VAL" 'BEGIN {printf "%.2f", v / 100}')
            wpctl set-volume @DEFAULT_AUDIO_SOURCE@ "$VAL_FRAC" 2>/dev/null || pactl set-source-volume @DEFAULT_SOURCE@ "${VAL}%" 2>/dev/null
            ;;
    esac
fi

# Signal waybar to refresh audio modules
pkill -RTMIN+10 waybar 2>/dev/null

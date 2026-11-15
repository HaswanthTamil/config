#!/usr/bin/env bash
# Media playback control helper
ACTION="$1"
case "$ACTION" in
    play-pause)
        playerctl play-pause 2>/dev/null
        ;;
    next)
        playerctl next 2>/dev/null
        ;;
    prev)
        playerctl previous 2>/dev/null
        ;;
    seek-forward)
        playerctl position 5+ 2>/dev/null
        ;;
    seek-backward)
        playerctl position 5- 2>/dev/null
        ;;
    *)
        echo "Usage: $0 {play-pause|next|prev|seek-forward|seek-backward}"
        exit 1
        ;;
esac

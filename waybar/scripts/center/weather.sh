#!/usr/bin/env bash
# Weather capsule - tries wttr.in, falls back gracefully
# Format matches Image 1: "27° 󰖙"

CACHE="$HOME/.config/waybar/state/weather_cache.json"
CACHE_MAX_AGE=1800  # 30 minutes

# Icon map for wttr.in condition codes
get_icon() {
    local desc="$1"
    case "$desc" in
        *Sunny*|*Clear*)           echo "󰖙" ;;
        *Partly*cloudy*|*Overcast*)echo "󰖐" ;;
        *Cloudy*|*cloudy*)         echo "󰖐" ;;
        *rain*|*Rain*|*Drizzle*)   echo "󰖗" ;;
        *Thunder*|*thunder*)       echo "󰙾" ;;
        *Snow*|*snow*|*Blizzard*)  echo "󰖘" ;;
        *Mist*|*Fog*|*fog*)        echo "󰖑" ;;
        *)                         echo "󰖙" ;;
    esac
}

# Check cache freshness
use_cache=0
if [ -f "$CACHE" ]; then
    cache_age=$(( $(date +%s) - $(stat -c %Y "$CACHE" 2>/dev/null || echo 0) ))
    if [ "$cache_age" -lt "$CACHE_MAX_AGE" ]; then
        use_cache=1
    fi
fi

if [ "$use_cache" -eq 1 ]; then
    cached_text=$(python3 -c "import json; d=json.load(open('$CACHE')); print(d.get('text','27° 󰖙'))" 2>/dev/null)
    cached_tip=$(python3 -c "import json; d=json.load(open('$CACHE')); print(d.get('tooltip','Weather'))" 2>/dev/null)
    printf '{"text": "%s", "class": "weather", "tooltip": "%s"}\n' "$cached_text" "$cached_tip"
    exit 0
fi

# Fetch from wttr.in (requires network)
if command -v curl >/dev/null 2>&1; then
    RAW=$(curl -sf --max-time 4 "https://wttr.in/?format=%t+%C" 2>/dev/null)
    if [ -n "$RAW" ] && [[ "$RAW" != *"Unknown"* ]] && [[ "$RAW" != *"error"* ]]; then
        TEMP=$(echo "$RAW" | awk '{print $1}' | tr -d '+')
        DESC=$(echo "$RAW" | cut -d' ' -f2-)
        ICON=$(get_icon "$DESC")
        TEXT="${TEMP} ${ICON}"
        TOOLTIP="Weather: ${TEMP} ${DESC}"

        # Save to cache
        mkdir -p "$(dirname "$CACHE")"
        python3 -c "
import json
with open('$CACHE', 'w') as f:
    json.dump({'text': '$TEXT', 'tooltip': '$TOOLTIP'}, f)
" 2>/dev/null

        printf '{"text": "%s", "class": "weather", "tooltip": "%s"}\n' "$TEXT" "$TOOLTIP"
        exit 0
    fi
fi

# Fallback: show last cache or placeholder
if [ -f "$CACHE" ]; then
    cached_text=$(python3 -c "import json; d=json.load(open('$CACHE')); print(d.get('text','--° 󰖙'))" 2>/dev/null)
    printf '{"text": "%s", "class": "weather offline", "tooltip": "Weather (offline)"}\n' "$cached_text"
else
    printf '{"text": "--°", "class": "weather offline", "tooltip": "Weather unavailable"}\n'
fi

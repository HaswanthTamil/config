#!/usr/bin/env bash
# Outputs current visualizer state for Waybar
STATE_FILE="$HOME/.config/waybar/state/visualizer.json"
OUTPUT_FILE="/tmp/waybar_visualizer.txt"

# Ensure daemon is running if enabled
ENABLED=$(python3 -c "import json; print(json.load(open('$STATE_FILE')).get('enabled', True))" 2>/dev/null || echo "True")

if [ "$ENABLED" != "True" ]; then
    echo '{"text": "", "alt": "disabled", "class": "disabled", "tooltip": "Beat Visualizer: Disabled (Click to enable)"}'
    exit 0
fi

# Ensure background daemon is running
if ! pgrep -f "visualizer_daemon.py" > /dev/null 2>&1; then
    nohup python3 "$HOME/.config/waybar/scripts/audio/visualizer_daemon.py" > /dev/null 2>&1 &
fi

if [ -f "$OUTPUT_FILE" ]; then
    TEXT=$(cat "$OUTPUT_FILE")
else
    TEXT="········"
fi

echo "{\"text\": \"$TEXT\", \"alt\": \"enabled\", \"class\": \"active\", \"tooltip\": \"Beat Visualizer: Active\\nClick: Toggle On/Off | Right: Open Panel\"}"

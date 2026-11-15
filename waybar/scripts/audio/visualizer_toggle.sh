#!/usr/bin/env bash
STATE_FILE="$HOME/.config/waybar/state/visualizer.json"
python3 -c "
import json
try:
    with open('$STATE_FILE', 'r') as f:
        data = json.load(f)
except Exception:
    data = {'enabled': True, 'style': 'bars', 'bars': 8, 'sensitivity': 1.0}

data['enabled'] = not data.get('enabled', True)
with open('$STATE_FILE', 'w') as f:
    json.dump(data, f, indent=2)
print('Visualizer enabled:', data['enabled'])
"
# Signal waybar to update visualizer module
pkill -RTMIN+8 waybar 2>/dev/null

#!/usr/bin/env bash
# Notification indicator for Waybar Middle Zone
# Emits a simple dot if there are unread notifications, empty string if none.

STATE_FILE="$HOME/.config/waybar/state/notifications.json"

UNREAD_COUNT=0

# Check dunstctl waiting if dunst is active
if command -v dunstctl >/dev/null 2>&1; then
    WAITING=$(dunstctl count waiting 2>/dev/null || echo 0)
    UNREAD_COUNT=$((UNREAD_COUNT + WAITING))
fi

# Also check persistent state file for unread notifications
if [ -f "$STATE_FILE" ]; then
    STATE_UNREAD=$(python3 -c "
import json
try:
    with open('$STATE_FILE') as f:
        data = json.load(f)
    print(sum(1 for n in data if not n.get('read', False)))
except:
    print(0)
" 2>/dev/null || echo 0)
    UNREAD_COUNT=$((UNREAD_COUNT + STATE_UNREAD))
fi

if [ "$UNREAD_COUNT" -gt 0 ]; then
    echo "{\"text\": \"●\", \"class\": \"unread\", \"tooltip\": \"Unread notifications exist ($UNREAD_COUNT)\\nClick to view notification center\"}"
else
    echo "{\"text\": \"\", \"class\": \"all-read\", \"tooltip\": \"No unread notifications\"}"
fi

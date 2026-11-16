#!/usr/bin/env bash
# Notification indicator matching Image 1 & 2: 󰂚 (badge) / 󰂜 (clean bell)

UNREAD_COUNT=0
if command -v dunstctl >/dev/null 2>&1; then
    WAITING=$(dunstctl count waiting 2>/dev/null || echo 0)
    UNREAD_COUNT=$((UNREAD_COUNT + WAITING))
fi

if [ -f "$HOME/.config/waybar/state/notifications.json" ]; then
    STATE_UNREAD=$(python3 -c "
import json
try:
    with open('$HOME/.config/waybar/state/notifications.json') as f:
        data = json.load(f)
    print(sum(1 for n in data if not n.get('read', False)))
except:
    print(0)
" 2>/dev/null || echo 0)
    UNREAD_COUNT=$((UNREAD_COUNT + STATE_UNREAD))
fi

if [ "$UNREAD_COUNT" -gt 0 ]; then
    echo "{\"text\": \"●\", \"class\": \"unread\", \"tooltip\": \"$UNREAD_COUNT unread notifications\nClick to open Notifications\"}"
else
    echo "{\"text\": \"\", \"class\": \"all-read\", \"tooltip\": \"No notifications\"}"
fi

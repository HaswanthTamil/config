#!/usr/bin/env bash
# Minimal pure-icon Bluetooth status for Waybar matching Image 1

POWERED=false
CONNECTED=false
DEV_NAME=""

if command -v bluetoothctl >/dev/null 2>&1; then
    if bluetoothctl show 2>/dev/null | grep -q "Powered: yes"; then
        POWERED=true
        DEV_NAME=$(bluetoothctl devices Connected 2>/dev/null | head -n 1 | cut -d' ' -f3-)
        [ -n "$DEV_NAME" ] && CONNECTED=true
    fi
fi

if [ "$POWERED" = false ] && command -v rfkill >/dev/null 2>&1; then
    if ! rfkill list bluetooth 2>/dev/null | grep -q "Soft blocked: yes"; then
        POWERED=true
    fi
fi

if [ "$CONNECTED" = true ]; then
    echo "{\"text\": \"󰂱\", \"class\": \"connected\", \"tooltip\": \"Bluetooth: Connected to $DEV_NAME\"}"
elif [ "$POWERED" = true ]; then
    echo "{\"text\": \"󰂯\", \"class\": \"disconnected\", \"tooltip\": \"Bluetooth: Enabled (No device connected)\"}"
else
    echo "{\"text\": \"󰂲\", \"class\": \"disabled\", \"tooltip\": \"Bluetooth: Disabled\"}"
fi

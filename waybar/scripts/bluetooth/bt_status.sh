#!/usr/bin/env bash
# Bluetooth status indicator for Waybar Right Zone
# Distinguishes disabled, enabled/disconnected, and connected states

POWERED=false
CONNECTED=false
DEVICE_NAME=""
BATTERY=""

if command -v bluetoothctl >/dev/null 2>&1; then
    SHOW_OUTPUT=$(bluetoothctl show 2>/dev/null)
    if echo "$SHOW_OUTPUT" | grep -q "Powered: yes"; then
        POWERED=true
        
        # Check connected devices
        CONNECTED_DEVICES=$(bluetoothctl devices Connected 2>/dev/null)
        if [ -n "$CONNECTED_DEVICES" ]; then
            CONNECTED=true
            DEVICE_MAC=$(echo "$CONNECTED_DEVICES" | head -n 1 | awk '{print $2}')
            DEVICE_NAME=$(echo "$CONNECTED_DEVICES" | head -n 1 | cut -d' ' -f3-)
            
            # Check battery
            if [ -n "$DEVICE_MAC" ]; then
                BATTERY=$(bluetoothctl info "$DEVICE_MAC" 2>/dev/null | grep "Battery Percentage" | awk -F '[()]' '{print $2}')
            fi
        fi
    fi
fi

# Fallback: check rfkill if bluetoothctl couldn't connect
if [ "$POWERED" = false ] && command -v rfkill >/dev/null 2>&1; then
    RF_BT=$(rfkill list bluetooth 2>/dev/null)
    if [ -n "$RF_BT" ] && ! echo "$RF_BT" | grep -q "Soft blocked: yes"; then
        POWERED=true
    fi
fi

if [ "$CONNECTED" = true ]; then
    ICON="󰂱"
    CLASS="connected"
    TOOLTIP="Bluetooth: Connected to $DEVICE_NAME"
    [ -n "$BATTERY" ] && TOOLTIP="$TOOLTIP ($BATTERY)"
    TOOLTIP="$TOOLTIP\nClick to open Bluetooth Panel"
    echo "{\"text\": \"$ICON\", \"class\": \"$CLASS\", \"tooltip\": \"$TOOLTIP\"}"
elif [ "$POWERED" = true ]; then
    ICON="󰂯"
    CLASS="disconnected"
    echo "{\"text\": \"$ICON\", \"class\": \"$CLASS\", \"tooltip\": \"Bluetooth: Enabled (No devices connected)\nClick to open Bluetooth Panel\"}"
else
    ICON="󰂲"
    CLASS="disabled"
    echo "{\"text\": \"$ICON\", \"class\": \"$CLASS\", \"tooltip\": \"Bluetooth: Disabled / Unavailable\nClick to open Bluetooth Panel\"}"
fi

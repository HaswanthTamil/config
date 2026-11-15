#!/usr/bin/env bash
# Wi-Fi status indicator for Waybar Right Zone
# Gracefully handles missing NetworkManager daemon or unavailable Wi-Fi adapter

SSID=""
SIGNAL=""
INTERFACE=""
CONNECTED=false

# Try nmcli first
if command -v nmcli >/dev/null 2>&1; then
    WIFI_LINE=$(nmcli -t -f ACTIVE,SSID,SIGNAL,DEVICE device wifi 2>/dev/null | grep '^yes' | head -n 1)
    if [ -n "$WIFI_LINE" ]; then
        SSID=$(echo "$WIFI_LINE" | cut -d: -f2)
        SIGNAL=$(echo "$WIFI_LINE" | cut -d: -f3)
        INTERFACE=$(echo "$WIFI_LINE" | cut -d: -f4)
        CONNECTED=true
    fi
fi

# Fallback: check /sys/class/net/wl* or iw
if [ "$CONNECTED" = false ]; then
    for iface in /sys/class/net/wl*; do
        [ -e "$iface" ] || continue
        OPERSTATE=$(cat "$iface/operstate" 2>/dev/null)
        if [ "$OPERSTATE" = "up" ]; then
            CONNECTED=true
            INTERFACE=$(basename "$iface")
            SSID="Connected"
            SIGNAL="100"
            break
        fi
    done
fi

if [ "$CONNECTED" = true ] && [ -n "$SSID" ]; then
    ICON="󰖩"
    CLASS="connected"
    echo "{\"text\": \"$ICON\", \"class\": \"$CLASS\", \"tooltip\": \"Wi-Fi: Connected to $SSID (${SIGNAL}%)\\nInterface: $INTERFACE\\nClick to open Network Panel\"}"
else
    ICON="󰖪"
    CLASS="disconnected"
    echo "{\"text\": \"$ICON\", \"class\": \"$CLASS\", \"tooltip\": \"Wi-Fi: Disconnected\\nClick to open Network Panel\"}"
fi

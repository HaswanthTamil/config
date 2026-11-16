#!/usr/bin/env bash
# Minimal pure-icon Wi-Fi status for Waybar matching Image 1

SSID=""
CONNECTED=false

if command -v nmcli >/dev/null 2>&1; then
    WIFI_LINE=$(nmcli -t -f ACTIVE,SSID device wifi 2>/dev/null | grep '^yes' | head -n 1)
    if [ -n "$WIFI_LINE" ]; then
        SSID=$(echo "$WIFI_LINE" | cut -d: -f2)
        CONNECTED=true
    fi
fi

if [ "$CONNECTED" = false ]; then
    for iface in /sys/class/net/wl*; do
        [ -e "$iface" ] || continue
        if [ "$(cat "$iface/operstate" 2>/dev/null)" = "up" ]; then
            CONNECTED=true
            SSID=$(basename "$iface")
            break
        fi
    done
fi

if [ "$CONNECTED" = true ]; then
    echo "{\"text\": \"󰖩\", \"class\": \"connected\", \"tooltip\": \"Wi-Fi: Connected to $SSID\"}"
else
    echo "{\"text\": \"󰖪\", \"class\": \"disconnected\", \"tooltip\": \"Wi-Fi: Disconnected\"}"
fi

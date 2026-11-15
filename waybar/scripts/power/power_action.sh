#!/usr/bin/env bash
# Power action handler with confirmations for destructive actions
ACTION="$1"
CONFIRM="$2"

lock_screen() {
    if command -v hyprlock >/dev/null 2>&1; then
        hyprlock &
    elif command -v swaylock >/dev/null 2>&1; then
        swaylock &
    else
        loginctl lock-session 2>/dev/null
    fi
}

logout_user() {
    hyprctl dispatch exit 2>/dev/null || loginctl terminate-user "$USER" 2>/dev/null
}

suspend_system() {
    systemctl suspend 2>/dev/null || loginctl suspend 2>/dev/null
}

hibernate_system() {
    systemctl hibernate 2>/dev/null || loginctl hibernate 2>/dev/null
}

reboot_system() {
    systemctl reboot 2>/dev/null || loginctl reboot 2>/dev/null
}

poweroff_system() {
    systemctl poweroff 2>/dev/null || loginctl poweroff 2>/dev/null
}

case "$ACTION" in
    lock)
        lock_screen
        ;;
    logout)
        logout_user
        ;;
    suspend)
        suspend_system
        ;;
    hibernate)
        hibernate_system
        ;;
    reboot)
        if [ "$CONFIRM" = "yes" ]; then
            reboot_system
        else
            echo "Confirmation required: pass 'yes' as second argument"
            exit 1
        fi
        ;;
    shutdown|poweroff)
        if [ "$CONFIRM" = "yes" ]; then
            poweroff_system
        else
            echo "Confirmation required: pass 'yes' as second argument"
            exit 1
        fi
        ;;
    *)
        echo "Usage: $0 {lock|logout|suspend|hibernate|reboot|shutdown} [yes]"
        exit 1
        ;;
esac

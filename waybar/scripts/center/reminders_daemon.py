#!/usr/bin/env python3
"""
Reminders background daemon.
Checks ~/.config/waybar/state/reminders.json every 30 seconds.
Triggers desktop notifications when reminders are due.
Persists state across reboots.
"""

import os
import json
import time
import subprocess
from datetime import datetime

REMINDERS_FILE = os.path.expanduser("~/.config/waybar/state/reminders.json")

def check_reminders():
    if not os.path.exists(REMINDERS_FILE):
        return

    try:
        with open(REMINDERS_FILE, "r") as f:
            reminders = json.load(f)
    except Exception:
        return

    now = datetime.now()
    changed = False

    for r in reminders:
        if r.get("completed", False):
            continue
        if r.get("notified", False):
            continue

        due_str = r.get("due", "")
        if not due_str:
            continue

        try:
            due_time = datetime.fromisoformat(due_str)
        except ValueError:
            try:
                due_time = datetime.strptime(due_str, "%Y-%m-%d %H:%M")
            except ValueError:
                continue

        if now >= due_time:
            # Fire notification
            title = r.get("title", "Reminder")
            try:
                subprocess.run(
                    ["notify-send", "-u", "critical", "-a", "Waybar Reminders", "🔔 Reminder Due", title],
                    check=False
                )
            except Exception:
                pass
            r["notified"] = True
            changed = True

    if changed:
        try:
            with open(REMINDERS_FILE, "w") as f:
                json.dump(reminders, f, indent=2)
        except Exception:
            pass

def main():
    while True:
        check_reminders()
        time.sleep(30)

if __name__ == "__main__":
    main()

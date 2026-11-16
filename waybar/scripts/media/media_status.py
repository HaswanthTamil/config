#!/usr/bin/env python3
"""
Media indicator for the collapsed left pill: play/pause glyph + track title.
"""

import json
import subprocess


def playerctl(*args):
    try:
        return subprocess.check_output(
            ["playerctl", *args],
            stderr=subprocess.DEVNULL,
            text=True,
        ).strip()
    except Exception:
        return ""


def get_media_status():
    status = playerctl("status") or "Stopped"
    title = playerctl("metadata", "title")
    artist = playerctl("metadata", "artist")

    if status == "Playing":
        icon = "󰏤"
        text = f"{icon} {title}" if title else icon
        return {
            "text": text,
            "class": "playing",
            "tooltip": f"Playing: {title} · {artist}\nClick: open Now Playing\nScroll ↑/↓: next/prev",
        }
    elif status == "Paused":
        icon = "󰐊"
        text = f"{icon} {title}" if title else icon
        return {
            "text": text,
            "class": "paused",
            "tooltip": f"Paused: {title} · {artist}\nClick: open Now Playing\nScroll ↑/↓: next/prev",
        }
    else:
        return {
            "text": "",
            "class": "stopped",
            "tooltip": "No media playing",
        }


if __name__ == "__main__":
    print(json.dumps(get_media_status(), ensure_ascii=False))

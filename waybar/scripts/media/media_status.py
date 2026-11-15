#!/usr/bin/env python3
"""
Media status script for Waybar Left Zone.
Detects active MPRIS players via playerctl.
Distinguishes Spotify, YouTube / browser media, and generic players.
Outputs Waybar JSON with compact formatting to prevent stretching.
"""

import subprocess
import json
import sys

def get_player_info():
    try:
        # Get active player name
        player = subprocess.check_output(
            ["playerctl", "status", "-f", "{{playerName}}"],
            stderr=subprocess.DEVNULL,
            text=True
        ).strip()
    except Exception:
        player = ""

    if not player:
        # Check all players
        try:
            players = subprocess.check_output(
                ["playerctl", "-l"],
                stderr=subprocess.DEVNULL,
                text=True
            ).strip().splitlines()
            if players:
                player = players[0]
        except Exception:
            player = ""

    if not player:
        return {"text": "", "alt": "none", "class": "stopped", "tooltip": "No media player active"}

    try:
        status = subprocess.check_output(
            ["playerctl", "-p", player, "status"],
            stderr=subprocess.DEVNULL,
            text=True
        ).strip()
    except Exception:
        status = "Stopped"

    if status not in ("Playing", "Paused"):
        return {"text": "", "alt": "none", "class": "stopped", "tooltip": "Playback stopped"}

    try:
        title = subprocess.check_output(
            ["playerctl", "-p", player, "metadata", "title"],
            stderr=subprocess.DEVNULL,
            text=True
        ).strip()
    except Exception:
        title = ""

    try:
        artist = subprocess.check_output(
            ["playerctl", "-p", player, "metadata", "artist"],
            stderr=subprocess.DEVNULL,
            text=True
        ).strip()
    except Exception:
        artist = ""

    # Determine icon and player type
    player_lower = player.lower()
    icon = ""  # generic music note
    player_type = "generic"

    if "spotify" in player_lower:
        icon = ""
        player_type = "spotify"
    elif any(b in player_lower for b in ["brave", "chrome", "chromium", "firefox", "vivaldi", "edge"]):
        # Check if title or url indicates youtube
        if "youtube" in title.lower() or "youtube" in player_lower:
            icon = ""
            player_type = "youtube"
        else:
            icon = ""
            player_type = "browser"

    # Truncate title / artist
    track_info = ""
    if title and artist:
        track_info = f"{title} · {artist}"
    elif title:
        track_info = title
    elif artist:
        track_info = artist
    else:
        track_info = player.capitalize()

    # Limit to 24 chars to avoid stretching the bar
    if len(track_info) > 24:
        display_text = f"{icon} {track_info[:22]}…"
    else:
        display_text = f"{icon} {track_info}"

    state_class = "playing" if status == "Playing" else "paused"

    return {
        "text": display_text,
        "alt": player_type,
        "class": f"{player_type} {state_class}",
        "tooltip": f"{player.capitalize()}: {track_info}\nStatus: {status}\nClick: Play/Pause | Middle: Next | Right: Open Panel"
    }

if __name__ == "__main__":
    info = get_player_info()
    print(json.dumps(info))

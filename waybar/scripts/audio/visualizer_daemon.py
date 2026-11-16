#!/usr/bin/env python3
"""
Thin bridge between cava and the Waybar/drawer visualizer.

All audio analysis is done by cava (https://github.com/karlstav/cava); this
script only converts cava's raw ASCII bar levels into block glyphs and keeps
the enabled/disabled state in sync.
"""

import json
import os
import signal
import subprocess
import sys
import time

STATE_FILE = os.path.expanduser("~/.config/waybar/state/visualizer.json")
OUTPUT_FILE = "/tmp/waybar_visualizer.txt"
CAVA = os.path.expanduser("~/.local/bin/cava")
CAVA_CONFIG = os.path.expanduser("~/.config/cava/config")

BAR_CHARS = [" ", "▁", "▂", "▃", "▄", "▅", "▆", "█"]

proc = None


def load_config():
    try:
        with open(STATE_FILE, "r") as f:
            return json.load(f)
    except Exception:
        return {"enabled": True, "sensitivity": 1.0}


def write_output(text):
    try:
        with open(OUTPUT_FILE + ".tmp", "w") as f:
            f.write(text)
        os.replace(OUTPUT_FILE + ".tmp", OUTPUT_FILE)
    except Exception:
        pass


def stop_cava():
    global proc
    if proc is not None:
        try:
            proc.terminate()
            proc.wait(timeout=1)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass
        proc = None


def start_cava():
    global proc
    if not os.path.exists(CAVA):
        return False
    try:
        proc = subprocess.Popen(
            [CAVA, "-p", CAVA_CONFIG],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            bufsize=1,
        )
        return True
    except Exception:
        proc = None
        return False


def shutdown(signum, frame):
    stop_cava()
    write_output("")
    sys.exit(0)


def main():
    global proc

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    idle_written = False
    last_sink = None
    last_sink_check = 0.0

    while True:
        cfg = load_config()

        if not cfg.get("enabled", True):
            stop_cava()
            if not idle_written:
                write_output("")
                idle_written = True
            time.sleep(0.8)
            continue

        idle_written = False

        # cava resolves the default sink once at start-up. Restart it when the
        # user switches output devices (e.g. speakers -> Bluetooth) so the
        # visualizer keeps following the active sink's monitor.
        now = time.monotonic()
        if now - last_sink_check > 5.0:
            last_sink_check = now
            try:
                sink = subprocess.check_output(
                    ["pactl", "get-default-sink"],
                    stderr=subprocess.DEVNULL,
                    text=True,
                ).strip()
            except Exception:
                sink = ""
            if sink and last_sink is not None and sink != last_sink:
                stop_cava()
            if sink:
                last_sink = sink

        if proc is None or proc.poll() is not None:
            stop_cava()
            if not start_cava():
                time.sleep(2.0)
                continue

        line = proc.stdout.readline()
        if not line:
            stop_cava()
            time.sleep(0.25)
            continue

        try:
            levels = [int(x) for x in line.strip().split(";") if x.strip() != ""]
        except ValueError:
            continue
        if not levels:
            continue

        sens = float(cfg.get("sensitivity", 1.0))
        glyphs = []
        for level in levels:
            idx = int(round(level * sens))
            idx = max(0, min(len(BAR_CHARS) - 1, idx))
            glyphs.append(BAR_CHARS[idx])

        text = "".join(glyphs)
        if not text.strip():
            text = "·" * len(glyphs)

        write_output(text)


if __name__ == "__main__":
    main()

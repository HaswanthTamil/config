#!/usr/bin/env python3
"""
Audio-reactive beat visualizer engine.
Consumes real audio data from PipeWire via parec.
Computes frequency bands and outputs Unicode bars.
Persists enabled/disabled state in ~/.config/waybar/state/visualizer.json.
"""

import sys
import os
import json
import time
import subprocess
import math
import struct
import signal

STATE_FILE = os.path.expanduser("~/.config/waybar/state/visualizer.json")
OUTPUT_FILE = "/tmp/waybar_visualizer.txt"

BAR_CHARS = [" ", " ", "▂", "▃", "▄", "▅", "▆", "▇", "█"]

def load_config():
    try:
        with open(STATE_FILE, "r") as f:
            return json.load(f)
    except Exception:
        return {"enabled": True, "style": "bars", "bars": 8, "sensitivity": 1.0}

def main():
    # Handle signals gracefully
    def handle_exit(signum, frame):
        try:
            with open(OUTPUT_FILE, "w") as f:
                f.write("")
        except Exception:
            pass
        sys.exit(0)

    signal.signal(signal.SIGINT, handle_exit)
    signal.signal(signal.SIGTERM, handle_exit)

    parec_proc = None

    while True:
        cfg = load_config()
        if not cfg.get("enabled", True):
            if parec_proc:
                parec_proc.terminate()
                parec_proc = None
            try:
                with open(OUTPUT_FILE, "w") as f:
                    f.write("")
            except Exception:
                pass
            time.sleep(1.0)
            continue

        num_bars = max(4, min(16, cfg.get("bars", 8)))
        sensitivity = float(cfg.get("sensitivity", 1.0))
        chunk_size = 512

        if parec_proc is None or parec_proc.poll() is not None:
            try:
                # Spawn parec to capture default monitor audio
                parec_proc = subprocess.Popen(
                    [
                        "parec",
                        "--raw",
                        "--rate=22050",
                        "--channels=1",
                        "--format=s16le"
                    ],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                    bufsize=chunk_size * 2
                )
            except Exception:
                time.sleep(2.0)
                continue

        raw_data = parec_proc.stdout.read(chunk_size * 2)
        if not raw_data or len(raw_data) < chunk_size * 2:
            time.sleep(0.05)
            continue

        samples = struct.unpack(f"<{chunk_size}h", raw_data)

        # Divide into frequency-approximated bins
        bin_size = chunk_size // num_bars
        bar_levels = []
        for i in range(num_bars):
            sub_samples = samples[i * bin_size : (i + 1) * bin_size]
            if not sub_samples:
                rms = 0.0
            else:
                sum_sq = sum(s * s for s in sub_samples)
                rms = math.sqrt(sum_sq / len(sub_samples))

            # Normalize and apply sensitivity (16-bit max is 32767)
            norm = (rms / 32768.0) * 12.0 * sensitivity
            level_idx = int(min(max(norm, 0), 1.0) * (len(BAR_CHARS) - 1))
            bar_levels.append(BAR_CHARS[level_idx])

        # Write to output file
        bar_string = "".join(bar_levels)
        # If silence, show subtle idle indicator
        if all(c == " " or c == " " for c in bar_levels):
            bar_string = "········"[:num_bars]

        try:
            with open(OUTPUT_FILE + ".tmp", "w") as f:
                f.write(bar_string)
            os.replace(OUTPUT_FILE + ".tmp", OUTPUT_FILE)
        except Exception:
            pass

        time.sleep(0.06)

if __name__ == "__main__":
    main()

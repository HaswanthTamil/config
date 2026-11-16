#!/usr/bin/env bash
ws=$(hyprctl activeworkspace -j 2>/dev/null | jq -r '.id // 1')
target=$(( (ws + 4) % 10 + 1 ))
python3 - "$target" <<'PY'
import os, socket, sys
target = int(sys.argv[1])
sig = os.environ.get('HYPRLAND_INSTANCE_SIGNATURE')
rt = os.environ.get('XDG_RUNTIME_DIR')
if not sig or not rt:
    raise SystemExit(1)
path = os.path.join(rt, 'hypr', sig, '.socket.sock')
msg = f'dispatch hl.dsp.focus({{ workspace = {target} }})'
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect(path)
s.sendall(msg.encode())
try:
    s.settimeout(0.5)
    s.recv(4096)
except Exception:
    pass
s.close()
PY

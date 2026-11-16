#!/usr/bin/env python3
import os, glob, json, sys

query = (sys.argv[1] if len(sys.argv) > 1 else '').lower()
dirs = []
xdg_data = os.environ.get('XDG_DATA_DIRS','/usr/share:/usr/local/share').split(':')
dirs += [os.path.join(d,'applications') for d in xdg_data]
dirs.append(os.path.expanduser('~/.local/share/applications'))
seen = set()
results = []

def score(name, q):
    n = name.lower()
    if not q:
        return 0
    if n == q: return 300
    if n.startswith(q): return 220
    if q in n: return 160
    # fuzzy subsequence
    i = 0
    for c in n:
        if i < len(q) and c == q[i]:
            i += 1
    if i == len(q): return 100
    # token match
    for tok in q.split():
        if tok and tok in n:
            return 120
    return 0

for d in dirs:
    if not os.path.isdir(d):
        continue
    for path in glob.glob(os.path.join(d, '*.desktop')):
        try:
            name = execmd = icon = comment = ''
            nodisplay = hidden = False
            in_main = False
            with open(path, errors='ignore') as f:
                for line in f:
                    line = line.strip()
                    if line == '[Desktop Entry]':
                        in_main = True
                        continue
                    if line.startswith('['):
                        in_main = False
                        continue
                    if not in_main or '=' not in line:
                        continue
                    k, v = line.split('=', 1)
                    if k == 'Name' and not name:
                        name = v
                    elif k == 'Exec' and not execmd:
                        execmd = v
                    elif k == 'Icon' and not icon:
                        icon = v
                    elif k == 'Comment' and not comment:
                        comment = v
                    elif k == 'NoDisplay':
                        nodisplay = v.lower() == 'true'
                    elif k == 'Hidden':
                        hidden = v.lower() == 'true'
            if not name or not execmd or nodisplay or hidden:
                continue
            key = name.lower() + '|' + execmd
            if key in seen:
                continue
            seen.add(key)
            sc = score(name, query)
            if sc <= 0:
                continue
            results.append({
                'type': 'app',
                'title': name,
                'subtitle': comment or execmd,
                'exec': execmd,
                'iconText': '󰣆',
                'path': path,
                'score': sc
            })
        except Exception:
            pass

results.sort(key=lambda x: (-x['score'], x['title'].lower()))
print(json.dumps(results[:20]))

#!/usr/bin/env python3
import os, sys, json

query = (sys.argv[1] if len(sys.argv) > 1 else '').lower().strip()
if len(query) < 2:
    print('[]')
    raise SystemExit(0)

roots = [os.path.expanduser(p) for p in ['~/Documents','~/Downloads','~/Desktop','~/Projects','~/Pictures']]
exclude = {'.git','node_modules','.cache','__pycache__','.venv','venv','build','dist','target'}
results = []
max_results = 20

def match_score(name, q):
    n = name.lower()
    if n == q: return 300
    if n.startswith(q): return 220
    if q in n: return 160
    i = 0
    for c in n:
        if i < len(q) and c == q[i]:
            i += 1
    return 90 if i == len(q) else 0

for root in roots:
    if not os.path.isdir(root):
        continue
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in exclude and not d.startswith('.')]
        for name in filenames + dirnames:
            if name.startswith('.'):
                continue
            sc = match_score(name, query)
            if sc <= 0:
                continue
            path = os.path.join(dirpath, name)
            isdir = os.path.isdir(path)
            results.append({
                'type': 'dir' if isdir else 'file',
                'title': name,
                'subtitle': dirpath,
                'path': path,
                'iconText': '󰉋' if isdir else '󰈔',
                'score': sc
            })
            if len(results) >= 60:
                break
        if len(results) >= 60:
            break
    if len(results) >= 60:
        break

results.sort(key=lambda x: (-x['score'], len(x['title']), x['title'].lower()))
print(json.dumps(results[:max_results]))

#!/usr/bin/env python3
import ast, math, operator, sys, json

expr = (sys.argv[1] if len(sys.argv) > 1 else '').strip()
if not expr:
    print('[]'); raise SystemExit(0)

if not any(c.isdigit() for c in expr):
    print('[]'); raise SystemExit(0)
if expr.isalpha():
    print('[]'); raise SystemExit(0)

expr_eval = expr.replace('^', '**')
allowed_names = {k: getattr(math, k) for k in dir(math) if not k.startswith('_')}
allowed_names.update({'abs': abs, 'round': round, 'min': min, 'max': max, 'pi': math.pi, 'e': math.e})
ops = {
    ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
    ast.Div: operator.truediv, ast.Mod: operator.mod, ast.Pow: operator.pow,
    ast.USub: operator.neg, ast.UAdd: operator.pos, ast.FloorDiv: operator.floordiv
}

def ev(node):
    if isinstance(node, ast.Expression):
        return ev(node.body)
    if isinstance(node, ast.Constant) and isinstance(node.value, (int, float)):
        return node.value
    if hasattr(ast, 'Num') and isinstance(node, ast.Num):
        return node.n
    if isinstance(node, ast.BinOp) and type(node.op) in ops:
        return ops[type(node.op)](ev(node.left), ev(node.right))
    if isinstance(node, ast.UnaryOp) and type(node.op) in ops:
        return ops[type(node.op)](ev(node.operand))
    if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
        fn = allowed_names.get(node.func.id)
        if fn is None or not callable(fn):
            raise ValueError('bad fn')
        return fn(*[ev(a) for a in node.args])
    if isinstance(node, ast.Name) and node.id in allowed_names:
        v = allowed_names[node.id]
        if isinstance(v, (int, float)):
            return v
        raise ValueError('bad name')
    raise ValueError('bad')

try:
    v = ev(ast.parse(expr_eval, mode='eval'))
    if isinstance(v, float):
        if v.is_integer():
            v = int(v)
        else:
            v = round(v, 10)
    if isinstance(v, bool):
        print('[]'); raise SystemExit(0)
    out = str(v)
    if out == expr:
        print('[]'); raise SystemExit(0)
    print(json.dumps([{ 'type': 'calc', 'title': '= ' + out, 'subtitle': expr, 'value': out, 'iconText': '󰃬', 'score': 1000 }]))
except Exception:
    print('[]')

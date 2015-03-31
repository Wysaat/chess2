inf = float('inf')

def alphabeta(node, depth, a, b, maximizing_player):
    if depth == 0 or is_terminal(node):
        return node.value
    if maximizing_player:
        v = -inf
        for child in node.children:
            v = max(v, alphabeta(child, depth-1, a, b, False))
            a = max(a, v)
            if b <= a:
                break
        return v
    else:
        v = inf
        for child in node.children:
            v = min(v, alphabeta(child, depth-1, a, b, True))
            b = min(b, v)
            if b <= a:
                break
        return v

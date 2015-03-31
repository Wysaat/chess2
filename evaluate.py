pnil = 0
wpawn = 1
wrook = 2
wknight = 3
wbishop = 4
wqueen = 5
wking = 6
bpawn = 7
brook = 8
bknight = 9
bbishop = 10
bqueen = 11
bking = 12

a = "2006043210001111011000000001000007000050000000000077:77709:;<098"


def evaluate(a):
    value = 0
    for i in a:
        p = ord(i) - ord('0')
        if   p == bpawn: value += 100
        elif p == bknight: value += 320
        elif p == bbishop: value += 330
        elif p == brook: value += 500
        elif p == bqueen: value += 900
        elif p == bking: value += 40000
        elif p == wpawn: value -= 100
        elif p == wknight: value -= 320
        elif p == wbishop: value -= 330
        elif p == wrook: value -= 500
        elif p == wqueen: value -= 900
        elif p == wking: value -= 20000
    return value

print evaluate(a)
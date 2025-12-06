data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

def handle_bank(b):
    lim = 12
    fin = ""
    lb = 0
    while lim > 0:
        ub = len(b) - lim
        mx = (-1, '0')
        for i, c in enumerate(b[lb:ub + 1]):
            if mx == -1 or mx[1] < c:
                mx = (i, c)
        lb += mx[0] + 1
        fin += mx[1]
        lim -= 1

    n = int(fin)
    return n

print(sum(map(handle_bank, data.split("\n"))))

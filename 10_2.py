from z3 import *

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

res = 0
line_cnt = 0
for line in data.split("\n"):
    print(f"i={line_cnt}")
    line_cnt += 1
    s = line.split(" ")
    target = list(map(int, s[-1][1:-1].split(",")))
    toggles = s[1:-1]
    toggles = list(map(lambda x: list(map(int, x[1:-1].split(","))), toggles))

    tl = len(target)

    matrices = []
    for toggle in toggles:
        matrix = [0] * tl
        for idx in toggle:
            matrix[idx] = 1
        matrices.append(matrix)

    init = [0] * tl
    matlen = len(matrices)

    # Z3 magic
    opt = Optimize()
    k = [Int(f"k_{i}") for i in range(matlen)]

    for i in range(matlen):
        opt.add(k[i] >= 0)

    for j in range(tl):
        opt.add(init[j] + Sum([k[i] * matrices[i][j] for i in range(matlen)]) == target[j])

    opt.minimize(Sum(k))

    if opt.check() == sat:
        model = opt.model()
        total = 0
        for i in range(matlen):
            total += model[k[i]].as_long()
        res += total
    

print(res)

import functools

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

mat = list(map(lambda x: [*x], data.split("\n")))

mi = len(mat)
mj = len(mat[0])

sloc = None
for i in range(mi):
    for j in range(mj):
        if mat[i][j] == 'S':
            sloc = (i, j)
            break

@functools.cache
def handle(i, beam):
    if i == mi:
        return 1

    if mat[i][beam] != '^':
        return handle(i + 1, beam)

    res = 0
    l = beam - 1
    r = beam + 1
    if l >= 0:
        res += handle(i + 1, l)

    if r < mj:
        res += handle(i + 1, r)

    return res

print(handle(1, sloc[1]))

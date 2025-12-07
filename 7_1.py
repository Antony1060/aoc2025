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

beams = set([sloc[1]])
splits = 0

for i in range(1, mi):
    new = set()
    for beam in beams:
        if mat[i][beam] != '^':
            new.add(beam)
            continue

        splits += 1
        l = beam - 1
        r = beam + 1
        if l >= 0 and l not in new:
            new.add(l)

        if l < mj and r not in new:
            new.add(r)
    
    beams = new

print(splits)

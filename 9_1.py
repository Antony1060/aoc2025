data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

coords = list(map(lambda x: tuple(map(int, x)), map(lambda x: x.split(","), data.split("\n"))))

mx = 0
for ix, iy in coords:
    for jx, jy in coords:
        if ix == jx and iy == jy:
            continue
        curr = (abs(ix - jx) + 1) * (abs(iy - jy) + 1)
        mx = max(mx, curr)

print(mx)

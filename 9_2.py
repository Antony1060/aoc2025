import functools
import json 
import os
import itertools

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

coords = list(map(lambda x: tuple(map(int, x)), map(lambda x: x.split(","), data.split("\n"))))
coord_set = set(coords)
assert len(coords) == len(coord_set)

gx = max(map(lambda x: x[1], coords)) + 1
gy = max(map(lambda x: x[0], coords)) + 1

x_ranges = []
y_ranges = []

def join_adjacent(idx1, idx2):
    iy, ix = coords[idx1]
    jy, jx = coords[idx2]
    if ix == jx:
        x_ranges.append((ix, range(min(iy, jy), max(iy, jy) + 1)))
    elif jy == iy:
        y_ranges.append((iy, range(min(ix, jx), max(ix, jx) + 1)))


def gen_ranges():
    for c in range(len(coords) - 1):
        join_adjacent(c, c + 1)
    join_adjacent(len(coords) - 1, 0)

def is_in_range(r, cnst, chng):
    for c, rng in r:
        if c == cnst and chng in rng:
            return True
    return False

gpdp = [[None for j in range(gy)] for i in range(gx)]

@functools.cache
def is_good_point(x, y):
    if gpdp[x][y] != None:
        return gpdp[x][y]

    visited = []
    d = 0
    h = [False] * 4
    while True:
        dx, dy = x + d, y + d
        h1, h2, h3, h4 = h
        if (not h1 and dx >= gx) or (not h2 and dy >= gy):
            break
        if not h1:
            visited.append((dx, y))
            if gpdp[dx][y] or is_in_range(x_ranges, dx, y):
                h[0] = True
        if not h2:
            visited.append((x, dy))
            if gpdp[x][dy] or is_in_range(y_ranges, dy, x):
                h[1] = True
        
        dx, dy = x - d, y - d
        if (not h3 and dx < 0) or (not h4 and dy < 0):
            break
        if not h3:
            visited.append((dx, y))
            if gpdp[dx][y] or is_in_range(x_ranges, dx, y):
                h[2] = True
        if not h4:
            visited.append((x, dy))
            if gpdp[x][dy] or is_in_range(y_ranges, dy, x):
                h[3] = True

        if all(h):
            gpdp[x][y] = True
            for vx, vy in visited:
                gpdp[vx][vy] = True
            return True
        d += 1
    gpdp[x][y] = False
    for vx, vy in visited:
        gpdp[vx][vy] = False 
    return False


@functools.cache
def is_good_square(c1, c2):
    iy, ix = c1
    jy, jx = c2
    for i in range(min(ix, jx), max(ix, jx) + 1):
        if not is_good_point(i, iy) or not is_good_point(i, jy):
            return False

    for i in range(min(iy, jy), max(iy, jy) + 1):
        if not is_good_point(ix, i) or not is_good_point(jx, i):
            return False
    return True

def precompute_good_points():
    global i
    product = itertools.product(range(gx), range(gy))
    i = 0
    for x, y in product:
        if i % 100000 == 0:
            print(f"{i=}/{gx * gy}")
        is_good_point(x, y)
        i += 1

if __name__ == "__main__":
    gen_ranges()
    #print_grid()

    precompute_good_points()

    i = 0
    mx = 0
    for ix, iy in coords:
        for jx, jy in coords:
            print(f"{i=}/{len(coords)**2}")
            i += 1
        
            if ix == jx and iy == jy:
                continue

            if not is_good_square((ix, iy), (jx, jy)):
                continue
            curr = (abs(ix - jx) + 1) * (abs(iy - jy) + 1)
            mx = max(mx, curr)

    print(mx)

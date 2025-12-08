import math
from dataclasses import dataclass

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

@dataclass(unsafe_hash = True, order = True)
class Coord:
    x: int
    y: int
    z: int
    
    def dist(self, other: Coord):
        return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2 + (self.z - other.z)**2)

to_connect = 1000

coords = list(map(lambda x: Coord(int(x[0]), int(x[1]), int(x[2])), map(lambda x: x.split(","), data.split("\n"))))

distances = dict()

for coord in coords:
    d = dict()
    for coord2 in coords:
        if coord == coord2:
            continue

        d[coord2] = coord.dist(coord2)

    distances[coord] = d

junctions = []

for i in range(to_connect):
    print(f"{i=}")
    s = None
    for c, d in distances.items():
        smc = min(d, key=d.get)
        smd = d[smc]
        if s == None or smd < s[2]:
            s = (c, smc, smd)

    if s == None:
        print("bad")
        break

    r, l, d = s

    rcurr = None
    lcurr = None
    for j in junctions:
        if r in j:
            rcurr = j
        if l in j:
            lcurr = j

        if rcurr != None and lcurr != None:
            break

    del distances[r][l]
    del distances[l][r]

    if rcurr == None and lcurr == None:
        junctions.append(set([r, l]))
    elif rcurr != None and lcurr == None:
        rcurr.add(l)
    elif lcurr != None and rcurr == None:
        lcurr.add(r)
    elif lcurr != rcurr:
        for j in lcurr:
            rcurr.add(j)
        lcurr.clear()

final = list(map(len, junctions))
filtered = list(filter(lambda x: x != 0, final))
filtered.sort()
print(final, filtered)
print(math.prod(filtered[-3:]))

import math
from dataclasses import dataclass
from sortedcontainers import SortedSet

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

@dataclass(unsafe_hash = True, order = True)
class Coord:
    x: int
    y: int
    z: int
    
    def dist(self, other):
        return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2 + (self.z - other.z)**2)

coords = list(map(lambda x: Coord(int(x[0]), int(x[1]), int(x[2])), map(lambda x: x.split(","), data.split("\n"))))

distances = dict()

for coord in coords:
    d = SortedSet()
    for coord2 in coords:
        if coord == coord2:
            continue

        d.add((coord.dist(coord2), coord2)) 

    distances[coord] = d

junctions = []

i = 0
while True:
    s = None
    for c, d in distances.items():
        smd, smc = d[0]
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

    distances[r].remove((d, l))
    distances[l].remove((d, r))

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

    filtered = list(filter(lambda x: x != 0, map(len, junctions)))
    if len(filtered) == 1 and filtered[0] == len(coords):
        print(r.x * l.x)
        break
    
    i += 1

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

ranges, ids = data.split("\n\n")
ranges = map(lambda x: x.split("-"), ranges.split("\n"))
ranges = [range(int(l), int(u) + 1) for l, u in ranges]

cnt = 0

for i in map(int, ids.split("\n")):
    for r in ranges:
        if i in r:
            cnt += 1
            break

print(cnt)

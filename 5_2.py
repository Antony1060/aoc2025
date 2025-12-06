data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

ranges, _ = data.split("\n\n")
ranges = map(lambda x: x.split("-"), ranges.split("\n"))
ranges = [(int(l), int(u)) for l, u in ranges]

def merge_ranges(ranges):
    merged = []
    ranges.sort()
    for r in ranges:
        if len(merged) == 0 or r[0] > merged[-1][1]:
            merged.append(r)
        else:
            merged[-1] = (merged[-1][0], max(merged[-1][1], r[1]))

    return merged 

ranges = merge_ranges(ranges)

total = 0
for (l, u) in ranges:
    total += (u + 1) - l

print(total)


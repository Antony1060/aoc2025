data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

def make_range(s):
    l, r = s.split("-")
    return (int(l), int(r))

ranges = list(map(make_range, data.split(",")))

invalid = 0
for l, r in ranges:
    for i in range(l, r + 1):
        s = str(i)
        if len(s) % 2 != 0:
            continue

        h = len(s) // 2
        if s[:h] == s[h:]:
            invalid += i

print(invalid)

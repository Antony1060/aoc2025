data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

def make_range(s):
    l, r = s.split("-")
    return (int(l), int(r))

ranges = list(map(make_range, data.split(",")))

def is_invalid(i):
    s = str(i)
    l = len(s)
    
    for i in range(1, l + 1):
        if l % i != 0 or (l // i) < 2:
            continue
        p = s[:i]
        if s == (p * (l // i)):
            return True
    return False

invalid = 0
for l, r in ranges:
    for i in range(l, r + 1):
        if is_invalid(i):
            invalid += i

print(invalid)

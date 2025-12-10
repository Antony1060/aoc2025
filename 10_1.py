data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

res = 0
i = 0
for line in data.split("\n"):
    print(f"{i=}")
    i += 1
    s = line.split(" ")
    target = s[0][1:-1]
    toggles = s[1:-1]
    toggles = list(map(lambda x: list(map(int, x[1:-1].split(","))), toggles))
    
    tl = len(target)
    curr = ["." * tl]
    d = 0
    visited = set()
    while True:
        done = False
        for c in curr:
            if c == target:
                res += d
                done = True
                break

        if done:
            break

        new_curr = []
        if len(curr) == 0:
            print("bad")
            exit(1)
        for c in curr:
            visited.add(c)
            for t in toggles:
                ns = [*c]
                for idx in t:
                    ns[idx] = "#" if ns[idx] == "." else "."
                val = "".join(ns)
                if val not in visited:
                    new_curr.append(val)
                    continue

        curr = new_curr

        d += 1

print(res)

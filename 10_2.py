data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

res = 0
i = 0
for line in data.split("\n"):
    print(f"{i=}")
    i += 1
    s = line.split(" ")
    target = list(map(int, s[-1][1:-1].split(",")))
    toggles = s[1:-1]
    toggles = list(map(lambda x: list(map(int, x[1:-1].split(","))), toggles))
    
    tl = len(target)
    curr = [[0] * tl]
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
        print(f"{d=}")
        for c in curr:
            visited.add(tuple(c))
            for t in toggles:
                ns = [*c]
                for idx in t:
                    ns[idx] += 1

                for idx in range(len(ns)):
                    if ns[idx] > target[idx]:
                        break
                else:
                    val = tuple(ns)
                    if val not in visited:
                        new_curr.append(ns)

        curr = new_curr

        d += 1

print(res)

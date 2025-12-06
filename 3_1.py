data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

def handle_bank(b):
    mx = (-1, '0')
    for i, c in enumerate(b):
        if mx[0] == -1 or mx[1] < c:
            mx = (i, c)

        if c == '9':
            break

    mx2 = -1
    lb = len(b)
    swp = mx[0] + 1 >= lb
    u = lb if not swp else lb - 1
    for i in range((mx[0] + 1) % lb, u):
        if mx2 == -1 or mx2 < b[i]:
            mx2 = b[i]

    n = int(f"{mx[1]}{mx2}") if not swp else int(f"{mx2}{mx[1]}")
    return n

print(sum(map(handle_bank, data.split("\n"))))

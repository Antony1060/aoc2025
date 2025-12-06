data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

mat = list(map(lambda x: [*x], data.split("\n")))

mi = len(mat)
mj = len(mat[0])

total = 0
good = 1
while good > 0:
    good = 0
    for i in range(mi):
        for j in range(mj):
            if mat[i][j] != '@':
                continue

            cnt = 0

            for k in range(max(i - 1, 0), min(mi, i + 2)):
                for l in range(max(j - 1, 0), min(mj, j + 2)):
                    if k == i and l == j:
                        continue
                    if mat[k][l] == '@':
                        cnt += 1

            if cnt < 4:
                mat[i][j] = '.'
                good += 1
    print(f"{good=}")
    total += good

print(total)


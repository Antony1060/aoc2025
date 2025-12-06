data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip("\n")

data = data.split("\n")
ops = data[-1] + " "

lens = []

cnt = 1
for c in reversed(ops):
    if c != ' ':
        lens.append((cnt - 1, c))
        cnt = 0
    cnt += 1

lens.reverse()

res = [0] * len(lens)
shift = 0
for l in range(len(lens)):
    (ll, c) = lens[l]
    for i in range(ll):
        num_text = ""

        for line in data[:-1]:
            num_text += line[shift + i]

        num = int(num_text.strip())

        if c == '+':
            res[l] += num
        elif c == '*':
            if res[l] == 0:
                res[l] = 1
            res[l] *= num
    shift += ll + 1

print(sum(res))

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

def make_node(line):
    start, outs = line.split(": ")

    return (start, outs.split(" "))

nodes = dict(map(make_node, data.split("\n")))

visited = set()
dp = dict()

def walk(curr, fft, dac):
    if curr == "out":
        return (1, fft, dac)

    if curr == "fft":
        fft = True

    if curr == "dac":
        dac = True

    val = 0
    for child in nodes[curr]:
        w_val, w_fft, w_dac = walk(child, fft, dac)
        if not fft and not w_fft:
            continue
        if not dac and not w_dac:
            continue
        val += w_val
    dp[curr] = (val, fft, dac)

    return dp[curr]

res = walk("svr", False, False)

print(res)

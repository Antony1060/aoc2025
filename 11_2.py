data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

def make_node(line):
    start, outs = line.split(": ")

    return (start, outs.split(" "))

nodes = dict(map(make_node, data.split("\n")))

dp_fft_dac = dict()
dp = dict()

def walk(curr):
    if curr == "out":
        return 1

    if curr in dp:
        return dp[curr]

    dp[curr] = 0
    for child in nodes[curr]:
        dp[curr] += walk(child)

    return dp[curr]

def walk_fft_dac(curr, fft = False, dac = False):
    if fft and dac:
        return dp[curr]

    if curr == "out":
        return 1 if fft and dac else 0

    if curr == "fft":
        fft = True

    if curr == "dac":
        dac = True

    dp_key = (curr, fft, dac)
    if dp_key in dp_fft_dac:
        return dp_fft_dac[dp_key]

    dp_fft_dac[dp_key] = 0
    for child in nodes[curr]:
        dp_fft_dac[dp_key] += walk_fft_dac(child, fft, dac)

    return dp_fft_dac[dp_key]

for key in nodes.keys():
    walk(key)

res = walk_fft_dac("svr")

print(res)

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

def make_node(line):
    start, outs = line.split(": ")

    return (start, outs.split(" "))

nodes = dict(map(make_node, data.split("\n")))

visited = set()
dp = dict()

def walk(curr):
    if curr == "out":
        return 1

    if curr in dp:
        return dp[curr]

    if curr in visited:
        return 0
    visited.add(curr)

    dp[curr] = 0
    for child in nodes[curr]:
        dp[curr] += walk(child)

    return dp[curr]

res = walk("you")

print(res)

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

parts = data.split("\n\n")
regions = parts[-1]

res = 0
for region in regions.split("\n"):
    area, shapes = region.split(": ")
    x, y = map(int, area.split("x"))
    area = x * y
    shapes = map(int, shapes.split(" "))
    n = sum(shapes)
    if n * 9 <= area:
        res += 1

print(res)

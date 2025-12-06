data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

cursor = 50
password = 0
for line in data.split("\n"):
    d = line[0]
    n = int(line[1:])
    f = 1 if d == 'R' else -1
    for _ in range(n):
        cursor += f * 1
        if cursor == 100:
            cursor = 0
        if cursor == -1:
            cursor = 99

        if cursor == 0:
            password += 1

print(password)

data = ""
with open("./input.txt", "r") as f:
    data = f.read().strip()

cursor = 50
password = 0
for line in data.split("\n"):
    d = line[0]
    n = int(line[1:])
    if d == 'L':
        cursor = (cursor - n) % 100
    else:
        cursor = (cursor + n) % 100

    if cursor == 0:
        password += 1

print(password)

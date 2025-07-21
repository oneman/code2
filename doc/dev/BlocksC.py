with open("Blocks.txt", "r") as file:
  for line in file:
    if (len(line) == 1): continue
    if (line.startswith("#")): continue
    parts = line.strip().split(";")
    name = parts[1][1:]
    range = parts[0].partition("..")
    start = int(range[0], 16)
    end = int(range[2], 16)
    size = (end + 1) - start
    offsetstr = str(start).rjust(7, ' ')
    sizestr = str(size).rjust(5, ' ')
    first = "{ .n = "
    mid1 = ", .sz = "
    mid2 = ", .a = \""
    last = "\"},"
    line = first + offsetstr + mid1 + sizestr + mid2 + name.strip() + last
    print(line)

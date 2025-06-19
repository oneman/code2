with open("Blocks.txt", "r") as file:
  for line in file:
    if (len(line) == 1): continue
    if (line.startswith("#")): continue
    parts = line.strip().split(";")
    #print(parts)
    name = parts[1][1:]
    range = parts[0].partition("..")
    start = int(range[0], 16)
    end = int(range[2], 16)
    size = (end + 1) - start
    offsetstr = str(start).rjust(7, ' ')
    sizestr = str(size).rjust(5, ' ')
    #number = int(parts[0], 16)
    #numberstr = str(number)
    #numberstr = numberstr.rjust(7, '0')
    #print(numberstr, name)
    print(offsetstr, sizestr, name)
    #print(size, start, end, name)

with open("UnicodeData.txt", "r") as file:
  for line in file:
    parts = line.strip().split(";")
    number = int(parts[0], 16)
    numberstr = str(number)
    numberstr = numberstr.rjust(7, ' ')
    name = parts[1]
    if (parts[1] == "<control>"): name = parts[10]
    print(numberstr, name)

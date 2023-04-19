# https://conwaylife.com/ref/lexicon/lex_p.htm

import os
import numpy


def cells2list(pattern: str):
    max_len = 0
    pattern_list = []
    for line in pattern.split("\n"):
        line = line.strip()
        max_len = len(line) if len(line) > max_len else max_len
        temp = []
        for x in line:
            if x == ".":
                temp.append(0)
            elif x == "O":
                temp.append(1)
        if temp:
            pattern_list.append(temp)
    for line in pattern_list:
        while len(line) < max_len:
            line.append(0)
    return pattern_list


# Rotates and saves the pattern in all directions
def new_pattern(addr, pattern):
    cwd = os.getcwd()

    pattern = cells2list(pattern)
    pattern = numpy.array(pattern)

    file = open(f"{cwd}/{addr}", "w")
    file.write(f"# {addr}: 0 degrees Rotated\n")
    for y in range(len(pattern)):
        for x in range(len(pattern[0])):
            file.write(str(pattern[y][x]) + " ")
        file.write("\n")
    file.close()

    # Rotated by 90 degrees
    pattern = numpy.rot90(pattern, 3)
    file = open(f"{cwd}/{addr}90", "w")
    file.write(f"# {addr}: 90 degrees Rotated\n")
    for y in range(len(pattern)):
        for x in range(len(pattern[0])):
            file.write(str(pattern[y][x]) + " ")
        file.write("\n")
    file.close()

    # Rotated by 180 degrees
    pattern = numpy.rot90(pattern, 3)
    file = open(f"{cwd}/{addr}180", "w")
    file.write(f"# {addr}: 180 degrees Rotated\n")
    for y in range(len(pattern)):
        for x in range(len(pattern[0])):
            file.write(str(pattern[y][x]) + " ")
        file.write("\n")
    file.close()

    # Rotated by 270 degrees
    pattern = numpy.rot90(pattern, 3)
    file = open(f"{cwd}/{addr}270", "w")
    file.write(f"# {addr}: 270 degrees Rotated\n")
    for y in range(len(pattern)):
        for x in range(len(pattern[0])):
            file.write(str(pattern[y][x]))
            file.write(" ")
        file.write("\n")
    file.close()


if __name__ == "__main__":
    A = numpy.zeros(shape=(70, 60))


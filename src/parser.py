import os
import numpy

VERTICAL = 0
HORIZONTAL = 1

STREAM = {
    ".": int(0),
    "O": int(1)
}

# Stores preloaded patterns
cache = {}


def read_from_cache(addr, angle, flip):
    pattern = numpy.array(cache[addr])
    if angle > 0:
        pattern = numpy.rot90(pattern, 3)
    if angle > 90:
        pattern = numpy.rot90(pattern, 3)
    if angle > 180:
        pattern = numpy.rot90(pattern, 3)
    temp = numpy.flip(pattern, flip) if flip != -1 else pattern
    return temp


def read_from_dir(cwd, addr, angle, flip):
    cache[ addr ] = []
    file = open(cwd + "/" + addr, "r")
    for line in file.readlines():
        line = line.strip()
        if line.startswith("#"):
            continue
        cache[addr].append([STREAM[x] for x in line])
    file.close()
    return read_from_cache(addr, angle, flip)


# Reads returns a pattern
def load(addr: str, angle: int, flip=-1):
    # Search the cache for preloaded
    if addr in cache.keys():
        return read_from_cache(addr, angle, flip)

    # Search currently directory for file
    cwd = os.getcwd()
    if addr in os.listdir(cwd):
        return read_from_dir(cwd, addr, angle, flip)

    # Search ./src directory for file
    if "src" not in os.listdir(cwd):
        return
    cwd = cwd + "/src"
    if addr in os.listdir(cwd):
        return read_from_dir(cwd, addr, angle, flip)

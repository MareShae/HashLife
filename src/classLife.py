# BIBLIOGRAPHY:
# https://en.wikipedia.org/wiki/Conway's_Game_of_Life
# https://inventwithpython.com/bigbookpython/project13.html

import numpy
from .parser import load as parser_load
from .parser import VERTICAL as P_VERTICAL
from .parser import HORIZONTAL as P_HORIZONTAL

kernel = numpy.array([
    [1, 1, 1],
    [1, 0, 1],
    [1, 1, 1]
])


class GameOfLife:
    def __init__(self, script: str):
        self.tick = 1
        self.res, self.ran = None, None
        self.Grid, self.Next = None, None

        w, h = 0, 0
        pattern = ""
        angle = 0
        flip = -1
        place = (1, 1)
        anchor = (0, 0)
        for line in script.split("\n"):
            if line.startswith("#"):            # Line is a comment
                continue
            elif line.startswith("[end]"):      # Load the pattern and append to Grid
                if pattern == "grid":
                    self.res = (h, w)
                    self.ran = (h - 1, w - 1)
                    self.Grid = numpy.zeros(shape=self.res)
                    self.Next = numpy.zeros(shape=self.res)
                else:
                    self.place(anchor, place, parser_load(pattern, angle, flip))
                pattern = ""
            elif line.startswith("W:"):
                w = int(line.lstrip("W:").strip())
            elif line.startswith("H:"):
                h = int(line.lstrip("H:").strip())
            elif line.startswith("[") and line.endswith("]"):
                if pattern != "":
                    raise Exception("Missing [end] of block")
                pattern = line.lstrip("[").rstrip("]")
            elif line.startswith("angle:"):
                angle = int(line.lstrip("angle:").strip())
                angle = int(((angle % 360) // 90) * 90)
            elif line.startswith("flip:"):
                _f = line.lstrip("flip:").strip()
                flip = P_HORIZONTAL if _f == "HORIZONTAL" else P_VERTICAL if _f == "VERTICAL" else -1
            elif line.startswith("place:"):
                _place = line.lstrip("place:").strip().split(",")
                if len(_place) != 2:
                    raise Exception("Unable to append place")
                place = (int(_place[0]), int(_place[1]))
            elif line.startswith("anchor:"):
                _anchor = line.lstrip("anchor:").strip().split(",")
                if len(_anchor) != 2:
                    raise Exception("Unable to append anchor")
                anchor = (int(_anchor[0]), int(_anchor[1]))
            pass

    # Places a pattern onto the board
    # Pattern can be (0,0) (0,1) (1,0) (1,1)
    def place(self, anchor, position, pattern):
        sy, sx = len(pattern), len(pattern[0])
        pos_0 = (position[0] - sy * anchor[0],
                 position[1] - sx * anchor[1])
        pos_1 = (pos_0[0] + sy, pos_0[1] + sx)
        for my, y in enumerate(range(pos_0[0], pos_1[0])):
            for mx, x in enumerate(range(pos_0[1], pos_1[1])):
                if pattern[my][mx] == 1:
                    self.Grid[y][x] = pattern[my][mx]

    # Applies the GoL rules to compute the next generation
    def __call__(self, *args, **kwargs):
        self.tick = self.tick + 1
        for y in range(1, self.ran[0]):
            for x in range(1, self.ran[1]):

                tally = 0
                # Count alive neighbours.!
                for dy in range(-1, 2):
                    for dx in range(-1, 2):
                        mx, my = 1 + dx, 1 + dy
                        nx, ny = x + dx, y + dy
                        tally += kernel[my][mx] * self.Grid[ny][nx]
                # Apply rules.!
                if self.Grid[y][x] == 0 and tally == 3:                      # Dead cell
                    self.Next[y][x] = 1      # Cell revives: Reproduction
                elif self.Grid[y][x] == 1 and (tally == 2 or tally == 3):      # Alive cell
                    self.Next[y][x] = 1      # Cell survives: Over-/Under-population
                else:
                    self.Next[y][x] = 0
        self.Grid = self.Next.copy()
        return self.Grid

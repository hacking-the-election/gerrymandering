"""
Animates the changes in precincts
"""


import os
import time
from tkinter import Canvas, Tk

from PIL import Image, ImageDraw

from hacking_the_election.test.funcs import polygon_to_list


def draw(shapes, state_name):
    """
    Creates tkinter drawing of shapes.
    """

    coords = [polygon_to_list(shape)[0] for shape in shapes]
    all_coords = [point for shape in coords for point in shape]

    bottom_point = [min(all_coords, key=lambda point: point[0])[0],
                    min(all_coords, key=lambda point: point[1])[1]]

    for i, (x, y) in enumerate(all_coords):
        all_coords[i][0] = (x - bottom_point[0]) + 1
        all_coords[i][1] = (y - bottom_point[1]) + 1

    shape_lengths = [len(shape) for shape in coords]
    coords = []
    index = 0
    for length in shape_lengths:
        coords.append(all_coords[index:index + length])
        index += length
    flattened_coords = [[coord for point in shape for coord in point]
                        for shape in coords]

    root = Tk()
    root.title(state_name)
    cv = Canvas(root, width=1000, height=1000, bd=0)
    cv.pack()
    for shape_coords in flattened_coords:
        modified_shape_coords = \
            [750 - int(c * 200) if i % 2 == 1 else int(c * 200)
             for i, c in enumerate(shape_coords)]
        cv.create_polygon(
            *modified_shape_coords,
            fill="white",
            outline="black"
        )
    root.mainloop()


if __name__ == "__main__":

    import pickle
    import sys

    from hacking_the_election.serialization import save_precincts
    
    sys.modules["save_precincts"] = save_precincts


    with open(sys.argv[1], "rb") as f:
        island_precinct_groups, _, _ = pickle.load(f)

    draw([p.coords for p in island_precinct_groups[0]], sys.argv[2])
"""
Animates the changes in precincts
"""


from tkinter import Canvas, Tk

from PIL import Image, ImageDraw

from hacking_the_election.test.funcs import polygon_to_list


FACTOR = 110
SUBTRACTION_VALUE = 750


def modify_coords(shapes):
    """
    Converts shapely objects into something
    tkinter and PIL can understand.
    """

    coords = [polygon_to_list(shape)[0] for shape in shapes]
    # Not grouped by shape
    all_coords = [point for shape in coords for point in shape]

    # Bottom-left corner of bounding box
    bottom_point = [min(all_coords, key=lambda point: point[0])[0],
                    min(all_coords, key=lambda point: point[1])[1]]

    # Move all points down so that bottom of image is (1, 1)
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

    # Dilate shape and subtract y values from given
    # number to turn image upside down.
    shape_coords = []
    for shape in flattened_coords:
        shape_coords.append(
            [SUBTRACTION_VALUE - int(c * FACTOR) if i % 2 == 1
                                                 else int(c * FACTOR)
             for i, c in enumerate(shape)]
        )

    # Average x and average y
    print(sum([c for shape in shape_coords for c in shape if c % 2 == 0])
        / len(all_coords))
    print(sum([c for shape in shape_coords for c in shape if c % 2 == 1])
        / len(all_coords))

    return shape_coords



def draw(coords, state_name):
    """
    Creates tkinter drawing of shapes.
    """

    root = Tk()
    root.title(state_name)
    cv = Canvas(root, width=1000, height=1000, bd=0)
    cv.pack()
    for shape_coords in coords:
        cv.create_polygon(
            *shape_coords,
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

    draw(modify_coords([p.coords for p in island_precinct_groups[0]]), sys.argv[2])
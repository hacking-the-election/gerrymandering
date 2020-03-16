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

    return shape_coords



def draw(shapes, state_name, block=True):
    """
    Creates tkinter drawing of shapes.
    """

    root = Tk()
    root.title(state_name)
    cv = Canvas(root, width=1000, height=1000, bd=0)
    cv.pack()

    shape_ids = []
    coords = modify_coords(shapes)
    for shape_coords in coords:
        shape_ids.append(
            cv.create_polygon(
                *shape_coords,
                fill="white",
                outline="black"
            )
        )
    if block:
        root.mainloop()
    else:
        root.update()
        return root, cv, shape_ids


def update_canvas(shape_ids, shapes, canvas, root):
    """
    Updates coords for `shapes` on `canvas`
    """

    coords = modify_coords(shapes)
    for shape, shape_coords in zip(shape_ids, coords):
        canvas.coords(shape, *shape_coords)
    root.update()


def save_as_image(communities, filepath):
    """
    Saves `shapes` to file at `filepath`
    """

    blue_communities = []
    red_communities = []
    for community in communities:
        if community.partisanship > 50:
            red_communities.append(community)
        else:
            blue_communities.append(community)
    community_colors = {}
    for community in blue_communities:
        changed_value = int(2.55 * community.standard_deviation)
        community_colors[community.id] = \
            (changed_value, changed_value, 255)
    for community in red_communities:
        changed_value = int(2.55 * community.standard_deviation)
        community_colors[community.id] = \
            (255, changed_value, changed_value)

    image = Image.new("RGB", (1000, 1000), (255, 255, 255))
    modified_coords = modify_coords([c.coords for c in communities])
    draw = ImageDraw.Draw(image)
    for community, shape in zip(communities, modified_coords):
        draw.polygon(
            shape,
            fill=community_colors[community.id],
            outline=(0, 0, 0)
        )
    image.save(filepath)


if __name__ == "__main__":

    import pickle
    import sys

    from hacking_the_election.serialization import save_precincts
    
    sys.modules["save_precincts"] = save_precincts


    with open(sys.argv[1], "rb") as f:
        island_precinct_groups, _, _ = pickle.load(f)

    save_as_image([p.coords for island in island_precinct_groups for p in island], sys.argv[2])
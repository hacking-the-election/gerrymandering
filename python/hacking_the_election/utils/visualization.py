"""Functions necessary for creating visuals.
"""


from copy import deepcopy

from shapely.geometry import Point


def modify_coords(coords, bounds):
    """Squishes coords into a bounding box.

    :param coords: The points to squish.
    :type coords: list of (2-list of float or shapely.geometry.Point)

    :param bounds: The box in which the inputted coords must be squished into. In format `[max_x, max_y]` (mins are 0).
    :type bounds: list of float

    :return: A list of coords that are squished into the bounds.
    :rtype: list of float
    """

    coords = deepcopy(coords)

    # Convert to float list if necessary
    if isinstance(coords[0], Point):
        for p, point in enumerate(coords):
            coords[p] = list(point.coords)[0]

    # Move to first quadrant.
    min_x = min([point[0] for point in coords])
    min_y = min([point[1] for point in coords])

    for p in range(len(coords)):
        coords[p][0] += -(min_x)
        coords[p][1] += -(min_y)

    # Get the bounding box dimensions
    X = [point[0] for point in coords]
    Y = [point[1] for point in coords]

    bounding_box_width = max(X) - min(X)
    bounding_box_length = max(Y) - min(Y)

    # Dilate to fit within canvas
    dilation_factor = max([bounding_box_width / bounds[0], bounding_box_length / bounds[1]])
    for p in range(len(coords)):
        for c in range(2):
            coords[p][c] *= (1 / dilation_factor) * 0.95

    # Reflect because y is flipped in Pillow
    for point in coords:
        point[1] = bounds[1] - point[1]

    # Center
    max_x = max([point[0] for point in coords])
    min_y = min([point[1] for point in coords])
    for point in coords:
        point[0] += (bounds[0] - max_x) / 2
        point[1] -= min_y / 2

    return coords


def add_leading_zeroes(n):
    """Adds leading zeroes to an int until it is 3 chars long.
    
    :param n: Number to add zeroes to.
    :type n: int

    :return: String with added leading zeroes.
    :rtype: str
    """

    chars = list(str(int(n)))
    while len(chars) < 3:
        chars.insert(0, "0")
    return "".join(chars)
"""Functions necessary for creating visuals.
"""

import numpy as np
from shapely.geometry import Point


def modify_coords(coords, bounds):
    """Squishes coords into a bounding box.

    :param coords: The points to squish.
    :type coords: list of list of float (coordinate pairs)

    :param bounds: The box in which the inputted coords must be squished into. In format `[max_x, max_y]` (mins are 0).
    :type bounds: list of float

    :return: A list of coords that are squished into the bounds.
    :rtype: list of float
    """

    coords = np.array(coords)
    n_points = len(coords)

    X = np.zeros(n_points)
    Y = np.zeros(n_points)
    for i, p in enumerate(coords):
        X[i] = p[0]
        Y[i] = p[1]

    # Move to first quadrant.
    min_x = min(X)
    min_y = min(Y)

    for p in range(n_points):
        X[p] += -(min_x)
        Y[p] += -(min_y)

    # Get the bounding box dimensions
    bounding_box_width = max(X) - min(X)
    bounding_box_length = max(Y) - min(Y)

    # Dilate to fit within canvas
    dilation_factor = max([bounding_box_width / bounds[0], bounding_box_length / bounds[1]])
    dilation_factor = (1 / dilation_factor) * 0.95
    for i in range(n_points):
        X[i] *= dilation_factor
        Y[i] *= dilation_factor

    # Reflect because y is flipped in Pillow
    for i in range(n_points):
        Y[i] = bounds[1] - Y[i]

    # Center
    max_x = max(X)
    min_y = min(Y)
    for i in range(n_points):
        X[i] += (bounds[0] - max_x) / 2
        Y[i] -= min_y / 2
    
    new_coords = [[X[i], Y[i]] for i in range(n_points)]

    return new_coords


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
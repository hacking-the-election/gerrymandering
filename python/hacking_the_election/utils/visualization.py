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

    X = []
    Y = []
    for point in coords:
        X.append(point[0])
        Y.append(point[1])

    # Translate
    min_x = min(X)
    min_y = min(Y)
    for p, point in enumerate(coords):
        new_x = point[0] - min_x - (0.01 * bounds[1])
        new_y = point[1] - min_y - (0.01 * bounds[1])

    # Dilate
    dilation_factor = max((0.99 * bounds[0]) / max(X),
                          (0.99 * bounds[1]) / max(Y))
    for point in coords:
        for c, coord in enumerate(point):
            point[c] *= dilation_factor

    # Reflect
    for point in coords:
        point[1] = bounds[1] - point[1]

    return coords
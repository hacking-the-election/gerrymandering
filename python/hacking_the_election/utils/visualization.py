"""Functions necessary for creating visuals.
"""


from copy import deepcopy

from shapely.geometry import Point


def modify_coords(coords, bounds):
    """Squishes coords into a bounding box.

    :param coords: The points to squish.
    :type coords: list of (2-list of float or shapely.geometry.Point)

    :param bounds: The box in which the inputted coords must be squished into. In format `[min_x, min_y, max_x, max_y]`
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

    # Dilate
    bigger_ratio = "X" if (abs(1 - (bounds[2] / max(X)))
                       > abs(1 - (bounds[3] / max(Y)))) else "Y"
    farthest_bound = bounds[2] if bigger_ratio == "X" else bounds[3]
    dilation_factor = (min(eval(bigger_ratio)) + farthest_bound) \
                    / max(eval(bigger_ratio))
    for point in coords:
        for i in range(len(point)):
            point[i] *= dilation_factor
    
    for p, point in enumerate(coords):
        X[p] = point[0]
        Y[p] = point[1]

    # Translate
    translation_vector = (bounds[0] - min(X), bounds[1] - min(Y))
    for point in coords:
        point[0] -= translation_vector[0]
        point[1] -= translation_vector[1]

    return coords
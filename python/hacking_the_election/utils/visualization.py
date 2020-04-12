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
        coords[p][0] += abs(min_x)
        coords[p][1] += abs(min_y)

    # Dilate to fit within canvas
    dilation_factor = max(max([point[0] for point in coords]) / bounds[0],
                          max([point[1] for point in coords]) / bounds[1])
    for p in range(len(coords)):
        for c in range(2):
            coords[p][c] *= dilation_factor

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
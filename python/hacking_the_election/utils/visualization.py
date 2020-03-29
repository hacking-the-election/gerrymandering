"""Functions necessary for creating visuals.
"""


def modify_coords(coords, bounds):
    """Squishes coords into a bounding box.

    :param coords: The points to squish.
    :type coords: list of (float or shapely.geometry.Point)

    :param bounds: The box in which the inputted coords must be squished into. In format `[min_x, min_y, max_x, max_y]`
    :type bounds: list of float

    :return: A list of coords that are squished into the bounds.
    :rtype: list of float
    """
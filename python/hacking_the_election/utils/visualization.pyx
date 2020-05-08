"""Functions necessary for creating visuals.
"""

cimport numpy as np
import numpy as np
from shapely.geometry import point


DTYPE = np.float64

ctypedef np.float64_t DTYPE_t


COLORS = [
    (235, 64, 52),
    (52, 122, 235),
    (194, 66, 245),
    (8, 163, 0),
    (255, 145, 0),
    (100, 192, 196),
    (92, 0, 0),
    (255, 199, 199),
    (0, 82, 22),
    (255, 255, 255)
]


cpdef list modify_coords(list coords, list bounds):
    """Squishes coords into a bounding box.

    :param coords: The points to squish.
    :type coords: list of list of float (coordinate pairs)

    :param bounds: The box in which the inputted coords must be squished into. In format `[max_x, max_y]` (mins are 0).
    :type bounds: list of float

    :return: A list of coords that are squished into the bounds.
    :rtype: list of list of float
    """

    cdef int n_points = len(coords)

    cdef np.ndarray X = np.zeros(n_points)
    cdef np.ndarray Y = np.zeros(n_points)
    cdef int i
    cdef list point
    for i, point in enumerate(coords):
        X[i] = point[0]
        Y[i] = point[1]

    # Move to first quadrant.
    cdef DTYPE_t min_x = min(X)
    cdef int min_y = min(Y)

    cdef int p
    for p in range(n_points):
        X[p] += -(min_x)
        Y[p] += -(min_y)

    # Get the bounding box dimensions
    cdef DTYPE_t bounding_box_width = max(X) - min(X)
    cdef DTYPE_t bounding_box_length = max(Y) - min(Y)

    # Dilate to fit within canvas
    cdef DTYPE_t dilation_factor = max([bounding_box_width / bounds[0], bounding_box_length / bounds[1]])
    dilation_factor = (1 / dilation_factor) * 0.95
    for i in range(n_points):
        X[i] *= dilation_factor
        Y[i] *= dilation_factor

    # Reflect because y is flipped in Pillow
    for i in range(n_points):
        Y[i] = bounds[1] - Y[i]

    # Center
    cdef DTYPE_t max_x = max(X)
    min_y = min(Y)
    for i in range(n_points):
        X[i] += (bounds[0] - max_x) / 2
        Y[i] -= min_y / 2

    # Translate down a bit (quick fix)
    # TODO: Actually solve the problem of coords being too high up.
    while min(Y) < 0:
        for i in range(n_points):
            Y[i] += 10
    
    new_coords = [[float(X[i]), float(Y[i])] for i in range(n_points)]

    return new_coords


cpdef str add_leading_zeroes(int n):
    """Adds leading zeroes to an int until it is 3 chars long.
    
    :param n: Number to add zeroes to.
    :type n: int

    :return: String with added leading zeroes.
    :rtype: str
    """

    cdef list chars = list(str(int(n)))
    while len(chars) < 3:
        chars.insert(0, "0")
    return "".join(chars)


cpdef dict get_partisanship_colors(list objects,  get_partisanship):
    """Maps objects to a color based on partisanship.

    :param objects: A list of objects.
    :type objects: list, most commonly containing `hacking_the_election.utils.precinct.Precinct` or `hacking_the_election.utils.community.Community`

    :param get_partisanship: A function that gets the partisanship of an item in `objects`. -1 means democrat and 1 means republican.
    :type get_partisanship: `types.FunctionType` that takes type of item in `objects` and returns float.

    :return: A dict mapping the objects in `objects` to rgb codes.
    :rtype: dict mapping type of item in `objects` to 3-tuple of int.
    """

    cpdef dict colors = {}

    cdef float partisanship
    for obj in objects:
        partisanship = get_partisanship(obj)
        if partisanship > 0:
            # Object is republican - purple to red.
            colors[obj] = (207, 27, int(-180 * partisanship) + 207)
        elif partisanship <= 0:
            # Object is democratic - blue to purple.
            colors[obj] = (int(-180 * abs(partisanship)) + 207, 27, 207)
    
    return colors
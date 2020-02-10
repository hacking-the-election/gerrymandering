"""
Useful computational geometry functions and classes
for communities algorithm
"""


import itertools
import math


# ===================================================
# general geometry functions


def get_equation(segment):
    """
    Returns the function representing the equation of a line segment
    containing 2 or more points
    """
    m = ( (segment[0][1] - segment[1][1])
        / (segment[0][0] - segment[1][0]))
    b = m * (- segment[1][0]) + segment[1][1]
    return lambda x: m * x + b


def get_segments(shape):
    """
    Returns set of segments from list of vertices
    """
    segments = set([shape[i:i + 2] if i + 2 <= len(shape) else
                   [shape[-1], shape[0]] for i in range(0, len(shape), 2)])
    return segments


def get_segments_collinear(segment_1, segment_2):
    """
    Returns whether or not two segments are collinear
    """

    f = equation(segment_1)
    g = equation(segment_2)
    # if two lines have more than one shared point,
    # they are the same line
    return (f(0) == g(0)) and (f(1) == g(1))


def get_if_bordering(shape_1, shape_2):
    """
    Returns whether or not two shapes are bordering

    Both args are sets of segs
    (set of list of list of floats)
    """

    bordering = False

    # check the equation for every segment in `shape_1` to see if it's
    # equal to any segment in `shape_2`
    for segment_1 in shape_1:
        for segment_2 in shape_2:
            if get_segments_collinear(segment_1, segment_2):
                bordering = True

    return bordering


def get_border(shapes):
    """
    Returns the vertices of the larger polygon that the smaller ones
    make up
    
    `shapes`: a list of lists of coords representing the vertices of a
              polygon. These shapes must be clustered together to make
              one larger polygon.
    """

    segments = set([get_segments(shape) for shape in shapes])

    inner_segments = {}

    for segment_1 in segments:
        if segment not in inner_segments:
            for segment_2 in segments - {segment_1}:
                if get_segments_collinear(segment_1, segment_2):
                    # segments are collinear
                    xs_1 = [p[0] for p in segment_1]
                    xs_2 = [p[0] for p in segment_2]
                    if max(xs_1) > min(xs_2) and max(xs_2) > min(xs_1):
                        # segments have overlapping bounding boxes
                        inner_segments.add(segment_1)
                        inner_segments.add(segment_2)
                        break

    outer_segments = segments - inner_segments

    # points are in order as groups of two (enough to draw the shape)
    outer_vertices = [vertex for vertex in segment
                      for segment in outer_segments]

    return outer_vertices


def get_point_in_polygon(point, shape):
    """
    Returns True if point is inside polygon, False if outside

    Args:
    `shape`: set of segments
    `point`: point to find whether or not it is in `shape`
    """

    crossings = 0

    for segment in shape:
        if (
                # both endpoints are to the left
                (segment[0][0] < point[0] and segment[1][0] < point[0]) or
                # both endpoints are to the right
                (segment[0][0] > point[0] and segment[1][0] > point[0]) or
                # both endpoints are below
                (segment[0][1] < point[1] and segment[1][1] < point[1])):
            continue

        # point is between endpoints
        if segment[0][1] > point[1] and segment[1][1] > point[1]:
            # both endpoints are above point
            crossings += 1
            continue
        else:
            # one endpoint is above point, the other is below

            # y-value of segment at point 
            y_c = equation(segment)(point[0])
            if y_c > point[1]:  # point is below segment
                crossings += 1
    
    return crossings % 2 == 1


def get_distance(p1, p2):
    """
    Distance formula
    """
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)


def get_permeter(shape):
    """
    Returns the permeter of set of segments `shape`
    """
    return sum([get_distance(*seg) for seg in shape])


def get_area(shape):
    """
    Returns the area of list of vertices `shape`
    """
    area = 0
    v2 = shape[-1]

    for v1 in shape:
        area += (v1[0] + v2[0]) * (v1[1] - v2[1])
        v2 = v1

    return abs(area / 2)


def get_schwartsberg_compactness(shape):
    """
    Returns the schwartsberg compactness value of set of segments
    `shape`
    """
    
    compactness = get_perimeter(shape)
                  / (2 * math.pi * math.sqrt(get_area(shape) / math.pi))
    if compactness < 1:
        return 1 / compactness
    else:
        return compactness


# ===================================================
# community algorithm-specifc functions and classes:


class Community:
    """
    A collection of precincts
    """
    
    def __init__(self, precincts):
        self.precincts = precincts

    @property
    def border(self):
        """
        The outside edge of the community (in segments)
        """
        precinct_coords = [p.coords for p in self.precincts]
        # unpack coords from unnecessary higher dimesions
        for i, precinct in enumerate(p.coords):
            coords = precinct
            while type(coords[0][0]) != type(1.0):
                coords = coords[0]
            precinct_coords[i] = coords
        return get_segments(get_border([]))

    @property
    def partisanship(self):
        """
        The percent of this community that is republican
        """
        rep_sum = 0
        total_sum = 0
        for precinct in self.precincts
            if (r_sum := precinct.r_election_sum) != -1:
                rep_sum += r_sum
                total_sum += r_sum + precinct.d_election_data
        return rep_sum / total_sum


def get_if_addable(precinct, community, boundary):
    """
    Returns whether or not it will create an island if `precinct` is
    added to `community` with `boundary` already taken up.

    Args:
    `precinct`: coords of segments of `precinct`
    `community`: community object of community that `precinct` may be
                 added to
    `boundary`: coords of segments of area in state not yet taken up by
                communities
    """

    # check if precinct is bordering boundary
    if not get_if_bordering(precinct, boundary):
        return False

    bordering_segments = {}  # for all precincts in `community`
    precinct_bordering_segments = {}  # just for `precinct`
    for community_segment in community.segments:
        for boundary_segment in boundary:
            if get_segments_collinear(community_segment, boundary_segment):
                bordering_segments.add(community_segment)
    for precinct_segment in precinct:
        for boundary_segment in boundary:
            if get_segments_collinear(precinct_segment, boundary_segment):
                bordering_segments.add(precinct_segment)
                precinct_bordering_segments.add(precinct_segment)

    if len(precinct_bordering_segments) > 1:
        # check every segment from `precinct` that is collinear with a
        # segment on the boundary to see whether or not it is touching
        # any other segment from `community` or `precinct` that is also
        # collinear with a segment from the boundary
        segs_touching_others = {seg: False for seg in
                                precinct_bordering_segments}
        for precinct_seg in precinct_bordering_segments:
            for community_seg in bordering_segments:
                if set(precinct_seg) & set(community_seg) != set():
                    segs_touching_others[precinct_seg] = True
                    break
        for val in segs_touching_others:
            if not val:
                return False

    return True
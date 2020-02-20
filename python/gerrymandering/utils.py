"""
Useful computational geometry functions and classes
for communities algorithm

For interpreting docstrings:
"list" refers to either `list` or `numpy.array`.
"point" refers to a list of two floats that represent a
point in 2-d space.
"segment" refers to a list of two points.
"polygon" refers to a list of segments of which the first represents
the hull of the polygon, and the rest represent any holes in the
polygon.
"shape" refers to an object that can be either a polygon or a
multipolygon (list of polygons).
"""


__all__ = ["clip", "UNION", "DIFFERENCE", "get_schwartzberg_compactness",
           "get_if_bordering", "get_point_in_polygon"]


import itertools
import math
import types

import numpy as np
from shapely.ops import unary_union
from shapely.geometry import MultiPolygon, Polygon


UNION = 1
DIFFERENCE = 2


# ===================================================
# general geometry functions


def _get_equation(segment):
    """
    Returns the function representing the equation of segment.

    If the line segment is vertical
    (y cannot be expressed in terms of x),
    returns single float representing x = `value`
    """
    if segment[0][0] == segment[1][0] and segment[0][1] == segment[1][1]:
        print(segment)
        raise ValueError("points of segment must be unique")

    try:
        m = ( (segment[0][1] - segment[1][1])
            / (segment[0][0] - segment[1][0]))
        b = m * (- segment[1][0]) + segment[1][1]
        def f(x): return m * x + b
        return f
    except ZeroDivisionError:
        # Lines is vertical.
        return float(segment[0][0])


def _get_segments(polygon):
    """
    Returns array of segments from polygon
    (with or without holes)

    ex.
    [
        [[0, 0], [4, 0], [4, 4], [0, 4]],
        [[1, 1], [1, 3], [3, 3], [3, 1]]
    ]
               |
               v 
    [
        [[0, 0], [4, 0]],
        [[4, 0], [4, 4]],
        [[4, 4], [0, 4]],
        [[1, 1], [1, 3]],
        [[1, 3], [3, 3]],
        [[3, 3], [3, 1]]
    ]

    If there are more than 2 points on a single segment,
    this function groups only the first and last into the segment list.
    """
    segments = []
    for shape in polygon:
        if shape[-1] == shape[0]:
            shape.pop(-1)
        shape_segments = [[shape[-1], shape[0]]]
        equation = _get_equation([shape[-1], shape[0]])
        for p in range(1, (len(shape))):
            if isinstance(equation, float):
                # Line is vertical.
                point_on_line = shape[p][0] == equation
            elif isinstance(equation, types.FunctionType):
                point_on_line = equation(shape[p][0]) == shape[p][1]
            if point_on_line:
                # shape[p] is on the same line as
                # shape[p - 1] and shape[p - 2]
                shape_segments[-1].append(shape[p])
            else:
                # Equations are different - time to start a new segment
                shape_segments.append([shape[p - 1], shape[p]])
                equation = _get_equation(shape_segments[-1])
        segments += shape_segments

    # Remove redundant points in the middle of a segment.
    for segment in segments:
        while len(segment) > 2:
            segment.pop(1)
    
    return np.array(segments)


def _get_segments_collinear(segment_1, segment_2):
    """
    Returns whether or not two segments are collinear.
    """
    f = _get_equation(segment_1)
    g = _get_equation(segment_2)
    # If two lines have more than one shared point,
    # they are the same line.
    return (f(0) == g(0)) and (f(1) == g(1))


def get_if_bordering(polygon_1, polygon_2):
    """
    Returns whether or not two polygons are bordering

    Checks all combinations of segments between the two polygons to see
    if they are both collinear and their bounding boxes (only x
    dimension) overlap. If this is the case, those segments overlap and
    that means the polygons border each other.

    Returns bool
    """
    segments_1 = _get_segments(polygon_1)
    segments_2 = _get_segments(polygon_2)

    for segment_1 in segments_1:
        for segment_2 in segments_2:
            # check collinearity
            if _get_segments_collinear(segment_1, segment_2):
                # check bounding boxes
                xs_1 = [p[0] for p in segment_1]
                xs_2 = [p[0] for p in segment_2]
                if min(xs_1) < max(xs_2) and min(xs_2) < max(xs_1):
                    return True

    return False


def get_point_in_polygon(polygon, point):
    """
    Returns whether or not point is in polygon
    (with or without holes)

    Draws vertical ray from point and checks numbder of intersections
    with segments. If odd, returns True; if even, returns False.

    Returns bool
    """
    crossings = 0
    segments = _get_segments(polygon)

    for segment in segments:
        # Continue if point is not between endpoints.
        # (ray cannot intersect)
        if (
                # Both endpoints are to the left.
                (segment[0][0] < point[0] and segment[1][0] < point[0])
                # Both endpoints are to the right.
                or (segment[0][0] > point[0] and segment[1][0] > point[0])):
            continue

        # Continue if both endpoints are below point.
        # (ray cannot intersect)
        if segment[0][1] < point[1] and segment[1][1] < point[1]: continue
        if segment[0][1] >= point[1] and segment[1][1] >= point[1]:
            # Both endpoints are above or at same y as point.
            # (ray must intersect)
            crossings += 1
            continue
        else:
            # One endpoint is above point, the other is below.

            # y-value of segment at point.
            y_c = _get_equation(segment)(point[0])
            if y_c >= point[1]:
                # Point is below or at same height as segment.
                crossings += 1

    return crossings % 2 == 1


def _get_distance(p1, p2):
    """
    Returns distance between `p1` and `p2`
    """
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)


def _get_perimeter(polygon):
    """
    Returns the perimeter of polygon (with or without holes)

    Adds perimeter of holes to total perimeter.
    """
    distances = np.array(
        [_get_distance(p1, p2) for p1, p2 in _get_segments(polygon)])
    return sum(distances)


def _get_simple_area(shape):
    """
    Returns the area of polygon (with no holes)

    Implementation of this algorithm:
    https://www.mathopenref.com/coordpolygonarea2.html
    """
    area = 0
    v2 = shape[-1]

    for v1 in shape:
        area += (v1[0] + v2[0]) * (v1[1] - v2[1])
        v2 = v1

    area = abs(area / 2)

    return area

def get_area(polygon):
    """
    Returns the area of polygon (with or without holes)
    """

    outer_area = _get_simple_area(polygon[0])
    for shape in polygon[1:]:
        outer_area -= _get_simple_area(shape)

    return outer_area


def get_schwartzberg_compactness(shape):
    """
    Returns the schwartzberg compactness value of array of segments
    `shape`

    Implemenatation of this algorithm (schwartzberg):
    https://fisherzachary.github.io/public/r-output.html
    """
    
    # perimeter / circumference
    compactness = (_get_perimeter(shape)
                   / (2 * math.pi * math.sqrt(get_area(shape) / math.pi)))
    return 1 - abs(1 - compactness)  # ensures less than one


def get_centroid(polygon):
    """
    Returns centriod of polygon 
    """

    centroid = [0, 0]
    area = 0

    for i in range(-1, len(polygon[0]) - 1):
        x0 = polygon[0][i][0]
        y0 = polygon[0][i][1]
        x1 = polygon[0][i + 1][0]
        y1 = polygon[0][i + 1][1]
        a = x0*y1 - x1*y0
        area += a
        centroid[0] += (x0 + x1) * a
        centroid[1] += (y0 + y1) * a

    area /= 2
    centroid[0] /= (6 * area)
    centroid[1] /= (6 * area)

    return centroid


def convert_to_shapely(shape):
    """
    Converts list-type polygon or multi-polygon `shape` to
    shapely.geometry.MultiPolygon or shapely.geometry.Polygon
    """
    polygons = []
    for polygon in shape:
        try:
            polygons.append(
                Polygon([tuple(coord) for coord in polygon]))
        except (ValueError, AssertionError):
            # if there is a multipolygon precinct (which there
            # shouldn't be, because those will be broken into
            # separate precincts in the serialization process)
            for p in polygon:
                polygons.append(
                    Polygon([tuple(coord) for coord in p]))
        
    return MultiPolygon(polygons)


def clip(shapes, clip_type):
    """
    Finds external border of a group of shapes

    Args:
    `shapes`: array of array of array of vertices
    `clip_type`: either 1 (union) or 2 (difference)

    if `clip_type` is difference, then there should only be 2 shapes in
    `shapes`

    Returns array of vertices
    """

    # Find union of shapes.

    multipolygons = [convert_to_shapely(shape) for shape in shapes]

    if clip_type == 1:
        solution = unary_union(multipolygons)
    elif clip_type == 2:
        if len(multipolygons) != 2:
            raise ValueError(
                "Polygon clip of type DIFFERENCE takes exactly 2 input shapes"
                )
        # Buffer is used below because in certain cases (such as using
        # both precinct-level data and district-level data in the input
        # to this function), the data will be slightly different.
        solution = multipolygons[0].buffer(0.0000001).difference(
            multipolygons[1].buffer(0.0000001))
    else:
        raise ValueError(
            "Invalid clip type. Use utils.UNION or utils.DIFFERENCE")

    real_border = []
    if isinstance(solution, Polygon):
        real_border.append(
            np.concatenate([np.concatenate(
                [np.asarray(t.exterior)[:, :2]] + [np.asarray(r)[:, :2] 
                for r in t.interiors]) for t in [solution]]).tolist())
    elif isinstance(solution, MultiPolygon):
        for polygon in solution.geoms:
            real_border.append(
            np.concatenate([np.concatenate(
                [np.asarray(t.exterior)[:, :2]] + [np.asarray(r)[:, :2] 
                for r in t.interiors]) for t in [polygon]]).tolist())

    return [real_border]


# ===================================================
# Community algorithm-specifc functions and classes:


class Community:
    """
    A collection of precincts
    """

    @staticmethod
    def get_standard_deviation(precincts):
        """
        Returns standard deviation of republican percentage in
        `precincts`
        """

        rep_percentages = [
            p.r_election_sum / (p.r_election_sum + p.d_election_sum) * 100
            for p in precincts]

        mean = sum(rep_percentages) / len(rep_percentages)
        
        return math.sqrt(sum([(p - mean) ** 2 for p in rep_percentages]))

    @staticmethod
    def get_partisanship(precincts):
        """
        Returns average percentage of republicans in `precincts`
        """

        rep_sum = 0
        total_sum = 0
        for precinct in precincts:
            if (r_sum := precinct.r_election_sum) != -1:
                rep_sum += r_sum
                total_sum += r_sum + precinct.d_election_data
        return rep_sum / total_sum
    
    def __init__(self, precincts, identifier):
        self.precincts = {precinct.id: precinct for precinct in precincts}
        self.id = identifier
        self.border = clip([p.coords for p in self.precincts.values()], UNION)
        self.partisanship = Community.get_partisanship(
                                list(self.precincts.values()))
        self.standard_deviation = Community.get_standard_deviation(
                                      self.precincts.values())
        self.population = sum([precinct.population for precinct
                               in self.precincts.values()])
        self.compactness = get_schwartzberg_compactness(self.border)

    def give_precinct(self, other, precinct_id, border=True,
                      partisanship=True, standard_deviation=True,
                      population=True, compactness=True):
        """
        Gives precinct from self to other community.

        All parameters with default value of `True` indicate whether or
        not to update that attribute after the community is given.
        """

        if not isinstance(other, Community):
            raise TypeError("Can only give precinct to community.")
        try:
            precinct = self.precincts[precinct_id]
        except KeyError:
            raise ValueError(
                f"No precinct in community with id '{precinct_id}.'")

        del self.precincts[precinct_id]
        other.precincts.append(precinct)
        # Update borders
        if border:
            self.border = clip([self.border, precinct.coords], DIFFERENCE)
            other.border = clip([other.border, precinct.coords], UNION)

        # Update other attributes that are dependent on precincts attribute
        for community in [self, other]:
            if partisanship:
                community.partisanship = get_partisanship(
                    community.precincts.values())
            if standard_deviation:
                community.standard_deviation = \
                    Community.get_standard_deviation(
                        community.precincts.values)
            if population:
                community.population = sum(
                    [precinct.population for precinct in
                     community.precincts.values()])
            if compactness:
                community.compactness = get_schwartzberg_compactness(
                    community.border)


# Below functions likely to be removed because they are specific to
# scenario in algorithm and will be implemented there.


def get_if_addable(precinct, community, boundary):
    """
    Returns whether or not it will create an island if `precinct` is
    added to `community` with `boundary` already taken up.

    Checks if the number of polygons in the output of boundary is the same after
    precinct is added to community

    Args:
    `precinct`: segments of `precinct`
    `community`: segements of community object that `precinct` may be
                 added to
    `boundary`: segments of area in state not yet taken up by
                communities
    """

    return True


def get_exchangeable_precincts(community, communities):
    """
    Finds exchangeable precincts between a community and its neighbors.

    Args:
    `community`: id for community to find precincts of
    `communities`: list of all communities in state

    Returns:
    Dict with keys as precinct ids and values as lists of ids of
    neighboring communities.
    """

    outside_precincts = {}

    borders = {c.id: c.border for c in communities}
    for precinct in community.precincts:
        if get_if_bordering(precinct.coords, borders[community]):
            # precinct is on outside border of community
            bordering_communities = [
                c.id for c in communities
                if get_if_bordering(precinct.coords, borders[c.id])
                ]
            if bordering_communities != []:
                outside_precincts[precinct.vote_id] = bordering_communities

    return outside_precincts

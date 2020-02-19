"""
Useful computational geometry functions and classes
for communities algorithm

Note for docstrings:
array refers to numpy.array or numpy.ndarray
segment or "seg" refers to array of points
point refers to array of two floats
"""


import itertools
import math

import numpy as np
from shapely.ops import unary_union
from shapely.geometry import MultiPolygon, Polygon


UNION = 1
DIFFERENCE = 2


# ===================================================
# general geometry functions


def get_equation(segment):
    """
    Returns the function representing the equation of a line segment
    containing 2 points
    """

    if segment[0][0] == segment[1][0] and segment[0][1] == segment[1][1]:
        raise ValueError("points of segment must be unique")

    m = ( (segment[0][1] - segment[1][1])
        / (segment[0][0] - segment[1][0]))
    b = m * (- segment[1][0]) + segment[1][1]
    return lambda x: m * x + b


def get_segments(polygon):
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
    """
    segments = np.array(
        [[shape[i], shape[i+1]] for shape in polygon
         for i in range(len(shape) - 1)]
        )
    return segments


def get_segments_collinear(segment_1, segment_2):
    """
    Returns whether or not two segments are collinear
    """

    f = get_equation(segment_1)
    g = get_equation(segment_2)
    # if two lines have more than one shared point,
    # they are the same line
    return (f(0) == g(0)) and (f(1) == g(1))


def get_if_bordering(polygon_1, polygon_2):
    """
    Returns whether or not two polygons are bordering

    Returns bool
    """

    # based on fact that if two segments are collinear, and their
    # bounding boxes overlap, they are overlapping

    segments_1 = get_segments(polygon_1)
    segments_2 = get_segments(polygon_2)

    for segment_1 in segments_1:
        for segment_2 in segments_2:
            # check collinearity
            if get_segments_collinear(segment_1, segment_2):
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

    Returns bool
    """

    crossings = 0
    segments = get_segments(polygon)

    for segment in segments:
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
            y_c = get_equation(segment)(point[0])
            if y_c >= point[1]:
                # point is below or at same height as segment
                crossings += 1

    return crossings % 2 == 1


def get_distance(p1, p2):
    """
    Distance formula
    """
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)


def get_perimeter(polygon):
    """
    Returns the perimeter of polygon (with or without holes)
    """
    distances = np.array(
        [get_distance(p1, p2) for p1, p2 in get_segments(polygon)])
    return sum(distances)


def get_simple_area(shape):
    """
    Returns the area of polygon (with no holes)
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

    outer_area = get_simple_area(polygon[0])
    for shape in polygon[1:]:
        outer_area -= get_simple_area(shape)

    return outer_area


def get_schwartsberg_compactness(shape):
    """
    Returns the schwartsberg compactness value of array of segments
    `shape`
    """
    
    # perimeter / circumference
    compactness = (get_perimeter(shape)
                   / (2 * math.pi * math.sqrt(get_area(shape) / math.pi)))
    return 1 - abs(1 - compactness)  # ensures less than one


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

    # find union of shapes

    polygons = []
    for multipolygon in shapes:
        polygon_list = []
        for polygon in multipolygon:
            try:
                polygon_list.append(
                    Polygon([tuple(coord) for coord in polygon]))
            except (ValueError, AssertionError):
                # if there is a multipolygon precinct (which there
                # shouldn't be, because those will be broken into
                # separate precincts in the serialization process)
                for p in polygon:
                    polygon_list.append(
                        Polygon([tuple(coord) for coord in p]))
        polygons.append(polygon_list)
        
    multipolygons = []
    for shape in polygons:
        multipolygons.append(MultiPolygon(shape))

    if clip_type == 1:
        solution = unary_union(multipolygons)
    elif clip_type == 2:
        if len(multipolygons) != 2:
            raise ValueError(
                "polygon clip of type DIFFERENCE takes exactly 2 input shapes"
                )
        solution = multipolygons[0].buffer(0.0000001).difference(multipolygons[1].buffer(0.0000001))
    else:
        raise ValueError(
            "invalid clip type. use utils.UNION or utils.DIFFERENCE")

    # display with matplotlib

    # from matplotlib import pyplot
    # from descartes import PolygonPatch

    # SIZE = (8.0, 8.0*(math.sqrt(5)-1.0/2.0))

    # fig = pyplot.figure(1, figsize=SIZE, dpi=90)
    # ax = fig.add_subplot(121)
    # for ob in multipolygons:
    #     p = PolygonPatch(ob, alpha=0.5, zorder=1)
    #     ax.add_patch(p)

    # og_xs = []
    # og_ys = []
    # for multipolygon in multipolygons:
    #     for polygon in multipolygon.geoms:
    #         for coord in list(polygon.exterior.coords):
    #             og_xs.append(coord[0])
    #             og_ys.append(coord[1])

    # ax1 = fig.add_subplot(122)
    # patch2b = PolygonPatch(union, alpha=0.5, zorder=2)
    # ax1.add_patch(patch2b)

    # border = [[list(coord) for coord in solution.exterior.coords]]
    # border_xs = [coord[0] for coord in border[0]]
    # border_ys = [coord[1] for coord in border[0]]

    # ax.set_autoscaley_on(False)
    # ax.set_autoscalex_on(False)
    # ax.set_ylim([min(og_ys), max(og_ys)])
    # ax.set_xlim([min(og_xs), max(og_xs)])

    # ax1.set_autoscaley_on(False)
    # ax1.set_autoscalex_on(False)
    # ax1.set_ylim([min(border_ys), max(border_ys)])
    # ax1.set_xlim([min(border_xs), max(border_xs)])

    # pyplot.show()

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
# community algorithm-specifc functions and classes:


class Community:
    """
    A collection of precincts
    """

    @staticmethod
    def get_standard_deviation(precincts):
        """
        Standard deviation of republican percentage in precincts
        """

        rep_percentages = [
            p.r_election_sum / (p.r_election_sum + p.d_election_sum) * 100
            for p in precincts]

        mean = sum(rep_percentages) / len(rep_percentages)
        
        return math.sqrt(sum([(p - mean) ** 2 for p in rep_percentages]))

    @staticmethod
    def get_partisanship(precincts):
        """
        Returns average percentabge of
        republicans in a list of precincts
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
        self.standard_deviation = Community.get_standard_deviation(self.precincts.values())
        self.population = sum([precinct.population for precinct in self.precincts.values()])
        self.compactness = get_schwartsberg_compactness(self.border)

    def give_precinct(self, other, precinct_id, border=True,
                      partisanship=True, standard_deviation=True,
                      population=True, compactness=True):
        """
        Gives precinct from self to other community.

        All parameters set as `True` for default indicate whether or
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
                community.compactness = get_schwartsberg_compactness(
                    community.border)


def get_if_addable(precinct, community, boundary):
    """
    Returns whether or not it will create an island if `precinct` is
    added to `community` with `boundary` already taken up.

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

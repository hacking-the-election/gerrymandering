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
           "get_if_bordering", "get_point_in_polygon", "Community"]


import itertools
import math
import types

import numpy as np
from shapely.ops import unary_union
from shapely.geometry import MultiPolygon, Polygon, Point
from shapely.errors import TopologicalError
from .test.utils import print_time


import logging

logging.basicConfig(filename="precincts.log", level=logging.DEBUG)


UNION = 1
DIFFERENCE = 2


# ===================================================
# general geometry functions


def get_if_bordering(shape1, shape2):
    """
    Returns whether or not two shapes are bordering
    """
    return shape1.touches(shape2)


def get_point_in_polygon(polygon, point):
    """
    Returns whether or not point is in polygon
    (with or without holes)

    Polygon can either be list or shapely.geometry.Polygon object
    Point can either be list or shapely.geomtry.Point object

    Returns bool
    """
    if isinstance(polygon, list):
        tuple_polygon = [[tuple(coord) for coord in ring] for ring in polygon]
        shapely_polygon = Polygon(tuple_polygon[0], tuple_polygon[1:])
    elif isinstance(polygon, Polygon):
        shapely_polygon = polygon
    if isinstance(point, list):
        coord = Point(*point)
    elif isinstance(point, Point):
        coord = point
    return shapely_polygon.contains(coord) or shapely_polygon.touches(coord)


def get_area(polygon):
    """
    Returns the area of polygon (with or without holes)
    """
    return polygon.area


def get_schwartzberg_compactness(polygon):
    """
    Returns the schwartzberg compactness value of `polygon`

    Implemenatation of this algorithm (schwartzberg):
    https://fisherzachary.github.io/public/r-output.html
    """
    
    # perimeter / circumference
    compactness = (polygon.length
                   / (2 * math.pi * math.sqrt(polygon.area / math.pi)))
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
    if clip_type == 1:
        solution = unary_union(shapes)
    elif clip_type == 2:
        if len(shapes) != 2:
            raise ValueError(
                "Polygon clip of type DIFFERENCE takes exactly 2 input shapes"
                )
        solution = shapes[0].difference(shapes[1])
    else:
        raise ValueError(
            "Invalid clip type. Use utils.UNION or utils.DIFFERENCE")
    return solution


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

        try:
            rep_percentages = [
                p.r_election_sum / (p.r_election_sum + p.d_election_sum) * 100
                for p in precincts]
            mean = sum(rep_percentages) / len(rep_percentages)
            
            return math.sqrt(sum([(p - mean) ** 2 for p in rep_percentages]))
        except ZeroDivisionError:
            return 0.0

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
                total_sum += r_sum + precinct.d_election_sum
        return rep_sum / total_sum
    
    def __init__(self, precincts, identifier):
        self.precincts = {precinct.vote_id: precinct for precinct in precincts}
        self.id = identifier
        if precincts != []:
            self.coords = clip([p.coords for p in self.precincts.values()], UNION)
        else:
            self.coords = []
        self.partisanship = None
        self.standard_deviation = None
        self.population = None
        self.compactness = None

    @print_time
    def give_precinct(self, other, precinct_id, border=True,
                      partisanship=True, standard_deviation=True,
                      population=True, compactness=True):
        """
        Gives precinct from self to other community.

        All parameters with default value of `True` indicate whether or
        not to update that attribute after the community is given.
        """

        if not isinstance(other, Community):
            raise TypeError(f"Invalid type {type(other)}.\n"
                            "Can only give precinct to community.")
        try:
            precinct = self.precincts[precinct_id]
        except KeyError:
            raise ValueError(
                f"No precinct in community with id '{precinct_id}.'")

        del self.precincts[precinct_id]
        other.precincts[precinct_id] = precinct
        # Update borders
        if border:
            self.coords = clip([p.coords for p in self.precincts.values()],
                               UNION)
            other.coords = clip([p.coords for p in other.precincts.values()],
                                UNION)

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

    @print_time
    def get_bordering_precincts(self, unchosen_precincts):
        """
        Returns list of precincts bordering `self`

        `unchosen_precincts` is a Community object that contains all
        the precincts in the state that haven't already been added to
        a community
        """
        bordering_precincts = []
        if self.precincts != {}:
            for vote_id, precinct in unchosen_precincts.precincts.items():
                if get_if_bordering(precinct.coords, self.coords):
                    bordering_precincts.append(bordering_precincts)
        else:
            bordering_precincts = list(unchosen_precincts.precincts.keys())
        return bordering_precincts
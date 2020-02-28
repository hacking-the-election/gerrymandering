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


__all__ = ["clip", "UNION", "DIFFERENCE", "get_if_bordering",
           "get_point_in_polygon", "Community", "group_by_islands",
           "get_precinct_link_pair", "LoopBreakException",
           "LoopContinueException", "get_area_intersection", 
           "shapely_to_polygon", "polygon_to_shapely", "average", "stdev"]


import math
import random
import logging

from shapely.ops import unary_union
from shapely.geometry import MultiLineString, MultiPolygon, Polygon, Point

from test.utils import print_time


logging.basicConfig(filename="precincts.log", level=logging.DEBUG)


UNION = 1
DIFFERENCE = 2
INTERSECTION = 3


# ===================================================
# custom exceptions


class LoopBreakException(Exception):
    """
    Used to break outer loops from within nested loops.
    """


class LoopContinueException(Exception):
    """
    Used to continue outer loops from within nested loops.
    """


# ===================================================
# general geometry functions


def get_if_bordering(shape1, shape2):
    """
    Returns whether or not two shapes are bordering
    """
    return isinstance(clip([shape1, shape2], INTERSECTION), MultiLineString)


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


def get_area_intersection(polygon1, polygon2):
    """
    Finds the area intersecting between the two polygons passed as arguments
    Both polygons should be shapely polygons
    Returns float (area of intersection.)
    """
    # find area of both polygons
    area1 = polygon1.area
    area2 = polygon2.area
    # find area of union
    union_coords = clip([polygon1, polygon2], 1)
    union_area = union_coords.area
    # The area of the intersection is the sum of the two areas minus the area of the union
    intersect_area = area1 + area2 - union_area
    # if the intersection area is negative, raise exception
    if abs(intersect_area) != intersect_area:
        raise Exception("Negative area found, check inputs")
    return intersect_area

def average(number_list):
    '''
    Finds the average of the integers in number_list.
    Returns float.
    '''
    sum1 = sum(number_list)
    return sum1/len(number_list)

def stdev(number_list, weight_list=None):
    '''
    Finds the standard deviation of the integers in integers list.
    If weight_list applies, multiply each of the numbers in number_list by the weight
    (decimal) corresponding in weight_list. weight_list should have a list of decimals.
    Does not use sample, but all elements in list
    Returns float.
    '''
    average_int = average(number_list)
    squared_sum = 0
    for num, integer in enumerate(number_list):
        difference = average_int - integer
        if weight_list:
            squared_sum += (difference * difference) * weight_list[num]
        else:
            squared_sum += (difference * difference)
    return math.sqrt(squared_sum / len(number_list))

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
    else:
        if len(shapes) != 2:
            raise ValueError(
                "Polygon clip of type DIFFERENCE takes exactly 2 input shapes"
                )
        if clip_type == 2:    
            solution = shapes[0].difference(shapes[1])
        elif clip_type == 3:
            solution = shapes[0].intersection(shapes[1])
        else:
            raise ValueError(
                "Invalid clip type. Use utils.UNION or utils.DIFFERENCE")
    return solution

def polygon_to_shapely(polygon):
    """
    Converts list-type polygon `shape` to
    `shapely.geometry.Polygon`
    """
    tuple_polygon = [[tuple(coord) for coord in linear_ring]
                     for linear_ring in polygon]
    return Polygon(tuple_polygon[0], tuple_polygon[1:])

def shapely_to_polygon(polygon):
    """
    Creates json polygon from shapely.geometry.Polygon
    NOTE: Assumes shapely polygon only has exterior and no holes.
    """
    coordinates = list(polygon.exterior.coords)
    linear_ring = []
    for tuple1 in coordinates:
        x = tuple1[0]
        y = tuple1[1]
        point_list = [x, y]
        linear_ring.append(point_list)
    print(linear_ring[0], linear_ring[-1])
    return linear_ring
# ===================================================
# Community algorithm-specifc functions and classes:


class Community:
    """
    A collection of precincts
    """
    
    def __init__(self, precincts, identifier, islands):
        self.precincts = {precinct.vote_id: precinct for precinct in precincts}
        self.id = identifier
        if precincts != []:
            self.coords = clip([p.coords for p in self.precincts.values()], UNION)
        else:
            self.coords = Polygon()
        self.partisanship = None
        self.standard_deviation = None
        self.population = None
        self.compactness = None

        # Attributes useful for states with islands:

        # dict with keys as index and values as number of precincts.
        self.islands = islands

    def update_compactness(self):
        """
        Updates the `compactness` attribute.

        Implemenatation of this algorithm (schwartzberg):
        https://fisherzachary.github.io/public/r-output.html
        """
        area = self.coords.area
        circumference = 2 * math.pi * math.sqrt(area / math.pi)
        perimeter = self.coords.length
        self.compactness = circumference / perimeter

    def update_standard_deviation(self):
        """
        Updates the `standard_deviation` attribute.
        """

        try:
            rep_percentages = [
                p.r_election_sum / (p.r_election_sum + p.d_election_sum) * 100
                for p in self.precincts.values()]
            mean = sum(rep_percentages) / len(rep_percentages)
            
            self.standard_deviation = math.sqrt(sum([(p - mean) ** 2 for p in rep_percentages]))
        except ZeroDivisionError:
            self.standard_deviation = 0.0

    def update_partisanship(self):
        """
        Updates the `partisanship` attribute
        """

        rep_sum = 0
        total_sum = 0
        for precinct in self.precincts.values():
            if (r_sum := precinct.r_election_sum) != -1:
                rep_sum += r_sum
                total_sum += r_sum + precinct.d_election_sum
        self.partisanship = rep_sum / total_sum

    def give_precinct(self, other, precinct_id, coords=True,
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
        if coords:
            self.coords = clip([self.coords, precinct.coords],
                               DIFFERENCE)
            other.coords = clip([p.coords for p in other.precincts.values()],
                                UNION)

        # Update other attributes that are dependent on precincts attribute
        for community in [self, other]:
            if partisanship:
                community.update_partisanship
            if standard_deviation:
                community.update_standard_deviation
            if population:
                community.population = sum(
                    [precinct.population for precinct in
                     community.precincts.values()])
            if compactness:
                community.update_compactness

    def fill(self, precincts, linked_precincts, island_index):
        """
        Fills a community up with precincts.

        Args:
            `precincts`:        List of Precinct objects in the island the
                                community is on.
            `linked_precincts`: Set of precincts that are meant to be part
                                of communities that span islands, therefore
                                making them untouchable during this step.
            `island_index`    : Index of island that corresponds to
                                a key in `self.islands`.
        """

        kwargs = {
            "partisanship": False,
            "standard_deviation": False,
            "population": False, 
            "compactness": False, 
            "coords": True
        }
        unchosen_precincts = Community(precincts[:], 0)

        for _ in range(self.islands[island_index]):
            # Set of precincts that have been tried
            tried_precincts = set()

            # Give random precinct to `community`
            random_precinct = random.sample(
                self.get_bordering_precincts(unchosen_precincts) \
                - tried_precincts - linked_precincts, 1)[0]

            unchosen_precincts.give_precinct(
                self, random_precinct, **kwargs)
            tried_precincts.add(random_precinct)

            # Keep trying other precincts until one of them
            # doesn't make an island.
            while isinstance(unchosen_precincts.coords, MultiPolygon):
                # Give it back
                self.give_precinct(
                    unchosen_precincts, random_precinct, **kwargs)
                print(f"precinct {random_precinct} added to and removed "
                    f"from community {self.id} because it created an "
                    "island")
                # Random precinct that hasn't already been
                # tried and also borders community.
                random_precinct = random.sample(
                    self.get_bordering_precincts(unchosen_precincts) \
                    - tried_precincts - linked_precincts, 1)[0]
                unchosen_precincts.give_precinct(
                    self, random_precinct, **kwargs)
                tried_precincts.add(random_precinct)

            print(f"precinct {random_precinct} added to community {self.id}")

    def get_bordering_precincts(self, unchosen_precincts):
        """
        Returns list of precincts bordering `self`

        `unchosen_precincts` is a Community object that contains all
        the precincts in the state that haven't already been added to
        a community
        """
        bordering_precincts = set()
        if self.precincts != {}:
            for vote_id, precinct in unchosen_precincts.precincts.items():
                if get_if_bordering(precinct.coords, self.coords):
                    bordering_precincts.add(precinct.vote_id)
        else:
            bordering_precincts = set(unchosen_precincts.precincts.keys())
        return bordering_precincts


def group_by_islands(precincts):
    """
    Returns list of precincts sorted into
    lists by what island they are on.
    """
    state_border = clip([p.coords for p in precincts], UNION)

    if isinstance(state_border, MultiPolygon):
        # There are islands in the state.

        island_polygons = state_border.geoms
        islands = [[] for _ in range(len(island_polygons))]

        for precinct in precincts:
            for i, island in enumerate(island_polygons):
                # If the first point of the precinct is in this island,
                # all of them must be.
                if get_point_in_polygon(
                        island, list(precinct.coords.exterior.coords[1])):
                    islands[i].append(precinct)
        return islands
    else:
        return [[p for p in precincts]]


def get_closest_precinct_on_island(island_centroid,
                                   other_island_border_precincts,
                                   distance=False):
    """
    Finds precinct on `other_island` that is closest to `island`

    Args:
        `island_centroid`:               Centroid of island.
        `other_island_border_precincts`: Precincts on border of other
                                         island.
        `distance`:                      Whether or not to return distance after
                                         precinct

    Returns `save_precincts.Precinct` object.
    """
    closest_precinct = None
    closest_precinct_distance = 0
    for p in other_island_border_precincts:
        distance = _get_distance(island_centroid,
                                 p.coords.centroid.coords[0])
        if (closest_precinct is None
                or distance < closest_precinct_distance):
            closest_precinct = p
            closest_precinct_distance = distance

    if distance:
        return closest_precinct, closest_precinct_distance
    else:
        return closest_precinct


def get_closest_precinct(island, island_precinct_groups, island_borders):
    """
    Finds precinct on border of any island in `island_precinct_groups`
    that is closest to `island`. Meant to be used as second precinct in
    a "linked pair" of precincts.
    
    Args:
        `island`:                 List of `save_precincts.Precinct`
                                  objects that is create the island
                                  you want to find the precinct that
                                  is closest to.
        `island_precinct_groups`: List of lists of
                                  `save_precincts.Precinct` objects
                                  grouped by islands in the whole state.
        `island_borders`:         List of Polygons that are borders of
                                  all the islands in the state.

    Returns vote_id attribute of closest precinct and index of island
    that contains that precinct.
    """
    island_index = island_precinct_groups.index(island)

    # List of precincts that border the "coastline" of the each island.
    # Grouped by island.
    border_precincts = \
        [[precinct for precinct in il
          if get_if_bordering(precinct.coords, i_border)]
         for i_border, il in zip(island_borders, island_precinct_groups)]

    island_centroid = island_borders[island_index].centroid.coords[0]
    
    closest_precinct = None
    closest_precinct_distance = 0
    closest_precinct_island_index = 0

    for i, island_border_precincts in enumerate(border_precincts):
        if i != island_index:
            closest_precinct_on_island, distance = \
                get_closest_precinct_on_island(
                    island_centroid, island_border_precincts, True)
            if (
                    closest_precinct is None
                    or closest_precinct_distance > distance):
                closest_precinct_distance = distance
                closest_precinct = closest_precinct_on_island
                closest_precinct_island_index = i

    return closest_precinct.vote_id, closest_precinct_island_index


def get_precinct_link_pair(island, island_precinct_groups,
                           island_border, island_borders):
    """
    Finds pair of two closest precincts on borders of `island` and any
    island in `island_precinct_groups`.

    Args:
        `island`:                 List of `save_precincts.Precinct`
                                  objects that is create the island
                                  you want to find the precinct that
                                  is closest to.
        `island_precinct_groups`: List of lists of
                                  `save_precincts.Precinct` objects
                                  grouped by islands in the whole state.
        `island_border`:          Polygon that is the border of `island`
        `island_borders`:         List of Polygons that are borders of
                                  all the islands in the state.

    Returns two strings (vote_ids of precinct on `island` and other
    precinct) and one integer (index of the island the other precinct
    is on).
    """

    precinct1, closest_precinct_island_index = get_closest_precinct(
        island, island_precinct_groups, island_borders)
    
    island_border_precincts = \
        [p for p in island if get_if_bordering(island_border, p.coords)]

    precinct2 = get_closest_precinct_on_island(
            island_borders[closest_precinct_island_index].centroid.coords[0],
            island_border_precincts
        )

    return precinct1, precinct2, closest_precinct_island_index

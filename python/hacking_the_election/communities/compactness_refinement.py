"""
Refines a state broken into communities
so that they are compact within a threshold.
"""

import math
import random
import sys

from shapely.geometry import Point
import matplotlib.pyplot as plt

from hacking_the_election.utils.compactness import (
    format_float,
    get_average_compactness,
    GIVE_PRECINCTS_COMPACTNESS_KWARGS
)
from hacking_the_election.utils.exceptions import LoopBreakException
from hacking_the_election.utils.geometry import (
    clip,
    DIFFERENCE,
    get_if_bordering,
    INTERSECTION
)


def refine_for_compactness(communities, minimum_compactness):
    """
    Returns communities that are all below the minimum compactness.
    """

    precincts = {p for c in communities for p in c.precincts.values()}
    for community in communities:
        community.update_compactness()

    try:
        while True:
            random.shuffle(communities)
            for community in communities:
                # Find circle with same area as district.
                radius = math.sqrt(community.coords.area) / math.pi
                center = community.coords.centroid.coords[0]
                circle = Point(*center).buffer(radius)

                # Find precincts that need to be added to this community.
                inside_circle = set()
                bordering_communities = \
                    [c for c in communities
                     if get_if_bordering(c.coords, community.coords)]
                for precinct in [p for c in bordering_communities
                                 for p in c.precincts.values()]:
                    # Section of precinct that is inside of circle.
                    circle_intersection = \
                        clip([circle, precinct.coords], INTERSECTION)
                    # If precinct and circle are intersecting
                    if len(circle_intersection.exterior.coords) != 0:
                        intersection_area = circle_intersection.area
                        precinct_area = precinct.coords.area
                        if intersection_area > (precinct_area / 2):
                            inside_circle.add(precinct)
                
                # Find precincts that need to be removed from this community.
                outside_circle = set()
                for precinct in community.precincts.values():
                    # Section of precinct that is outside of circle.
                    circle_difference = clip([precinct.coords, circle], DIFFERENCE)
                    # If precinct is not entirely in circle
                    if len(circle_difference.exterior.coords) != 0:
                        difference_area = circle_difference.area
                        precinct_area = precinct.coords.area
                        if difference_area > (precinct_area / 2):
                            outside_circle.add(precinct)

                while True:
                    # Add precincts one by one
                    precinct = random.sample(outside_circle)
                    for other_community in communities:
                        if precinct in other_community.precincts.values():
                            other_community.give_precinct(
                                community,
                                precinct.vote_id,
                                **GIVE_PRECINCTS_COMPACTNESS_KWARGS
                            )
                            community.update_compactness()
                            other_community.update_compactness()
                    if get_average_compactness(communities) > minimum_compactness:
                        raise LoopBreakException

    except LoopBreakException:
        return communities

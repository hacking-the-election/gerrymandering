"""
Refines a state broken into communities
so that they are compact within a threshold.
"""

import math
import random

from shapely.geometry import Point
import matplotlib.pyplot as plt
import numpy as np

from hacking_the_election.utils.compactness import (
    add_precinct,
    format_float,
    get_average_compactness
)
from hacking_the_election.utils.exceptions import (
    CreatesMultiPolygonException,
    LoopBreakException,
    ZeroPrecinctCommunityException
)
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
        X = np.array()
        Y = np.array()
        i = 0
        while True:
            print(f"Average community compactness: {get_average_compactness(communities)}")

            for community in communities:
                try:
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

                    while outside_circle != set() and inside_circle != set():
                        # Add precincts one by one
                        try:
                            precinct = random.sample(outside_circle, 1)[0]
                            add_precinct(communities, community, precinct)
                            print(f"Added {precinct.vote_id} to community {community.id}. Compactness: {community.compactness}")
                        except ValueError:
                            # No precincts left in `outside_circle`
                            precinct = random.sample(inside_circle, 1)[0]
                            add_precinct(communities, community, precinct)
                            print(f"Added {precinct.vote_id} to community {community.id}. Compactness: {community.compactness}")
                        if get_average_compactness(communities) > minimum_compactness:
                            raise LoopBreakException
                        if community.compactness > minimum_compactness:
                            print(f"Community {community.id} has compactness above threshold.")
                            raise LoopBreakException(1)

                    print(f"Community {community.id} failed to get below threshold after adding and removing all precincts in and out of circle respectively.")
                    
                except LoopBreakException as e:
                    if e.level == 1:
                        continue
                    else:
                        raise e

            i += 1
            np.append(X, [i])
            np.append(Y, [get_average_compactness(communities)])

        plt.scatter(X, Y)
        plt.show()

    except LoopBreakException:
        return communities

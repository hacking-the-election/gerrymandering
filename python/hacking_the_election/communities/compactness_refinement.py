"""
Refines a state broken into communities
so that they are compact within a threshold.
"""

import math
import pickle
import random
import warnings

from shapely.geometry import Point
import matplotlib.pyplot as plt
import numpy as np

from hacking_the_election.test.funcs import (
    polygon_to_list,
    convert_to_json
)
from hacking_the_election.utils.compactness import (
    add_precinct,
    format_float,
    get_average_compactness
)
from hacking_the_election.utils.exceptions import (
    CreatesMultiPolygonException,
    ExitException,
    LoopBreakException,
    ZeroPrecinctCommunityException
)
from hacking_the_election.utils.geometry import (
    clip,
    DIFFERENCE,
    get_if_bordering,
    INTERSECTION
)


def signal_handler(sig, frame):
    raise ExitException


def refine_for_compactness(communities, minimum_compactness, output_file):
    """
    Returns communities that are all below the minimum compactness.
    """

    precincts = {p for c in communities for p in c.precincts.values()}
    for community in communities:
        community.update_compactness()

    try:
        X = np.array([])
        Y = np.array([])
        i = 0

        community = random.choice(communities)
        while True:
            try:
                print("Average community compactness: "
                        f"{get_average_compactness(communities)}")

                # Find circle with same area as district.
                radius = math.sqrt(community.coords.area / math.pi)
                center = community.coords.centroid.coords[0]
                circle = Point(*center).buffer(radius)

                # Find precincts that need to be added to this community.
                inside_circle = set()
                bordering_communities = \
                    [c for c in communities
                    if get_if_bordering(c.coords, community.coords)
                        and c != community]
                for precinct in [p for c in bordering_communities
                                    for p in c.precincts.values()]:
                    if get_if_bordering(precinct.coords,
                                        community.coords):
                        # Section of precinct that is inside of circle.
                        circle_intersection = \
                            clip([circle, precinct.coords], INTERSECTION)
                        # If precinct and circle are intersecting
                        if not circle_intersection.is_empty:
                            intersection_area = circle_intersection.area
                            precinct_area = precinct.coords.area
                            if intersection_area > (precinct_area / 2):
                                inside_circle.add(precinct)
                
                # Find precincts that need to be removed from this community.
                outside_circle = set()

                outside_bordering_precincts = community.get_outside_precincts()
                for precinct in outside_bordering_precincts:
                    # Section of precinct that is outside of circle.
                    circle_difference = clip([precinct.coords, circle],
                                                DIFFERENCE)
                    # If precinct is not entirely in circle
                    if not circle_difference.is_empty:
                        difference_area = circle_difference.area
                        precinct_area = precinct.coords.area
                        if difference_area > (precinct_area / 2):
                            outside_circle.add(precinct)

                while outside_circle != set() or inside_circle != set():
                    # Add precincts one by one
                    try:
                        precinct = random.sample(outside_circle, 1)[0]
                        try:
                            add_precinct(
                                communities, community, precinct, True)
                            print(f"Removed {precinct.vote_id} from "
                                    f"community {community.id}. Compactness: "
                                    f"{round(community.compactness, 3)}")
                        except IndexError:
                            # Precinct was outside of circle but not
                            # bordering any other community.
                            pass
                        outside_circle.discard(precinct)
                    except ValueError:
                        # No precincts left in `outside_circle`
                        precinct = random.sample(inside_circle, 1)[0]
                        try:
                            add_precinct(
                                communities, community, precinct, False)
                            print(f"Added {precinct.vote_id} to community "
                                    f"{community.id}. Compactness: "
                                    f"{round(community.compactness, 3)}")
                        except ValueError as e:
                            warnings.warn(str(e))
                        inside_circle.discard(precinct)

                    if all([c.compactness > minimum_compactness
                            for c in communities]):
                        i += 1
                        np.append(X, [i])
                        np.append(Y, [get_average_compactness(communities)])
                        raise LoopBreakException
                    if community.compactness > minimum_compactness:
                        i += 1
                        np.append(X, [i])
                        np.append(Y, [get_average_compactness(communities)])
                        print(f"Community {community.id} has "
                                "compactness above threshold.")
                        break

                if community.compactness <= minimum_compactness:
                    i += 1
                    np.append(X, [i])
                    np.append(Y, [get_average_compactness(communities)])
                    print(f"Community {community.id} failed to get above "
                            "threshold after adding and removing all "
                            "precincts in and out of circle.")
                
            except LoopBreakException:
                break

            # To stop endless recursion.
            community = \
                random.choice([c for c in communities if c != community])

        plt.scatter(X, Y)
        plt.show()

        with open("test_compactness_graph.pickle", "wb+") as f:
            pickle.dump([X, Y], f)

        convert_to_json(
            [polygon_to_list(c.coords) for c in communities],
            output_file,
            [{"ID": c.id} for c in communities]
        )
    
    except Exception as e:
        with open("test_compactness.pickle", "wb+") as f:
            pickle.dump(communities, f)
        convert_to_json(
            [polygon_to_list(c.coords) for c in communities],
            "test_compactness.json",
            [{"ID": c.id} for c in communities]
        )
        plt.scatter(X, Y)
        plt.show()
        with open("test_compactness_graph.pickle", "wb+") as f:
            pickle.dump([X, Y], f)
        raise e


if __name__ == "__main__":

    import signal
    import sys

    from hacking_the_election.utils.initial_configuration import Community
    from hacking_the_election.serialization import save_precincts


    sys.modules["save_precincts"] = save_precincts

    with open("test_vermont_communities.pickle", "rb") as f:
        communities, linked_precinct_chains = pickle.load(f)
    
    signal.signal(signal.SIGINT, signal_handler)

    refine_for_compactness(communities, 0.75, "test_compactness_output.json")
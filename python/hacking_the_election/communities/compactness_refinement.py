"""
Refines a state broken into communities
so that they are compact within a threshold.

Usage:
python3 compactness_refinement.py [initial_configuration] [compactness_threshold] [json_path] [pickle_path] [animation_dir] [state_name]
"""

import math
import os
import pickle
import random
import warnings

from shapely.geometry import Point
import matplotlib.pyplot as plt

from hacking_the_election.test.funcs import (
    polygon_to_list,
    convert_to_json
)
from hacking_the_election.utils.animation import (
    save_as_image
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
    INTERSECTION,
    UNION
)
from hacking_the_election.utils.initial_configuration import (
    add_leading_zeroes
)


def signal_handler(sig, frame):
    raise ExitException


def refine_for_compactness(communities, minimum_compactness,
                           linked_precincts, output_json,
                           output_pickle, animation_dir,
                           state_name, max_iterations=100):
    """
    Returns communities that are all below the minimum compactness.
    """

    precincts = {p for c in communities for p in c.precincts.values()}
    for community in communities:
        community.update_compactness()

    try:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

        X = [[] for _ in communities]
        Y = [[] for _ in communities]
        i = 0
        f = 1

        worst_communities = []

        while True:
            try:
                if i > max_iterations:
                    print("max iterations for compactness reached for compactness")
                    raise ExitException

                print(f"avg compactness: {get_average_compactness(communities)}")

                community = min(communities, key=lambda c: c.compactness)
                worst_communities.append(community.id)
                if len(worst_communities) >= 5:
                    if len(set(worst_communities[-5:])) == 1:
                        # Last 5 communities were the same.
                        community = random.choice(
                            [c for c in communities if c != community]
                        )

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
                    if precinct not in linked_precincts:
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
                    if precinct not in linked_precincts:
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
                            try:
                                add_precinct(
                                    communities, community, precinct, True)
                                save_as_image(
                                    communities,
                                    os.path.join(
                                        animation_dir,
                                        f"{add_leading_zeroes(f)}.png"
                                    )
                                )
                                f += 1
                            except (CreatesMultiPolygonException,
                                    ZeroPrecinctCommunityException):
                                pass
                        except IndexError:
                            # Precinct was outside of circle but not
                            # bordering any other community.
                            pass
                        outside_circle.discard(precinct)
                    except ValueError:
                        # No precincts left in `outside_circle`
                        precinct = random.sample(inside_circle, 1)[0]
                        try:
                            try:
                                add_precinct(
                                    communities, community, precinct, False)
                                save_as_image(
                                    communities,
                                    os.path.join(
                                        animation_dir,
                                        f"{add_leading_zeroes(f)}.png"
                                    )
                                )
                                f += 1
                            except (CreatesMultiPolygonException,
                                    ZeroPrecinctCommunityException):
                                pass
                        except ValueError as e:
                            warnings.warn(str(e))
                        inside_circle.discard(precinct)
                        

                    if all([c.compactness > minimum_compactness
                            for c in communities]):

                        print([c.compactness for c in communities])
                        i += 1
                        for x, c in enumerate(communities):
                            Y[x].append(c.compactness)
                            X[x].append(i)
                        raise LoopBreakException
                    if community.compactness > minimum_compactness:
                        i += 1
                        for x, c in enumerate(communities):
                            Y[x].append(c.compactness)
                            X[x].append(i)
                        break

                if community.compactness <= minimum_compactness:
                    i += 1
                    for x, c in enumerate(communities):
                        Y[x].append(c.compactness)
                        X[x].append(i)
                
            except LoopBreakException:
                break
    except Exception as e:
        print(str(e))
        raise e
    finally:
        with open(output_pickle, "wb+") as f:
            pickle.dump(communities, f)
        convert_to_json(
            [polygon_to_list(c.coords) for c in communities],
            output_json,
            [{"ID": c.id} for c in communities]
        )
        # for x, y in zip(X, Y):
        #     plt.plot(x, y)
        # plt.show()
        print(f"finished compactness. communities: {[c.compactness for c in communities]}")
        return communities


if __name__ == "__main__":

    import signal
    import sys

    from hacking_the_election.utils.community import Community
    from hacking_the_election.serialization import save_precincts


    sys.modules["save_precincts"] = save_precincts

    signal.signal(signal.SIGINT, signal_handler)

    with open(sys.argv[1], "rb") as f:
        communities, linked_precinct_chains = pickle.load(f)
    linked_precincts = {p for chain in linked_precinct_chains for p in chain}

    refine_for_compactness(communities, float(sys.argv[2]),
                           linked_precincts, *sys.argv[3:])
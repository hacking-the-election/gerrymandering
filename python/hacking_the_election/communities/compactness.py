"""Refines a configuration of political communities in a state so that their Reock compactness score is as low as possible.

Usage:
python3 -m hacking_the_election.communities.compactness <serialized_state> <n_communities> (<animation_dir> | "none") <output_path>
"""

import copy
import math
import os
import pickle
import sys
import time

from shapely.geometry import MultiPolygon, Point

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.utils.exceptions import LoopBreakException
from hacking_the_election.utils.geometry import get_distance
from hacking_the_election.utils.graph import (
    get_components,
    get_giveable_precincts,
    get_takeable_precincts
)
from hacking_the_election.utils.stats import average
from hacking_the_election.visualization.misc import draw_state


M = 10
N = 20


def optimize_compactness(communities, graph, animation_dir=None):
    """Takes a set of communities and exchanges precincts in a way that maximizes compactness.

    :param communities: The current state of the communities within a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param graph: A graph containing precinct data within a state.
    :type graph: `pygraph.classes.graph.graph`

    :param animation_dir: Path to the directory where animation files should be saved, defaults to None
    :type animation_dir: str or NoneType
    """

    for community in communities:
        community.update_imprecise_compactness()

    if animation_dir is not None:
        draw_state(graph, animation_dir)

    best_communities = [copy.copy(c) for c in communities]
    last_communities = []
    iterations_since_best = 0

    n = 0

    while True:

        # Stop if number of iterations since the best 
        # communities so far is more than N.
        if min([c.imprecise_compactness for c in communities]) \
                > min([c.imprecise_compactness for c in best_communities]):
            # Current communities are new best.
            best_communities = [copy.copy(c) for c in communities]
            iterations_since_best = 0
        else:
            iterations_since_best += 1
            if iterations_since_best > N:
                # Revert to best and exit function.
                for c in communities:
                    for bc in best_communities:
                        if c.id == bc.id:
                            c = bc
                
                rounded_compactnesses = [round(c.imprecise_compactness, 3) for c in communities]
                print(rounded_compactnesses, min(rounded_compactnesses))
                return

        # Stop if there has been no change in communities this iteration.
        if last_communities != []:
            try:
                for c in communities:
                    for lc in last_communities:
                        if c.id == lc.id:
                            if c.precincts != lc.precincts:
                                changed = True
                                raise LoopBreakException
                    # Revert to best and exit function.
                    # (LoopBreakException was not raised,
                    # so communities did not change).
                    for c in communities:
                        for bc in best_communities:
                            if c.id == bc.id:
                                c = bc
                
                    rounded_compactnesses = [round(c.imprecise_compactness, 3) for c in communities]
                    print(rounded_compactnesses, min(rounded_compactnesses))
                    return
            except LoopBreakException:
                pass


        community = communities[n]

        # Precinct centroid coords
        X = []
        Y = []
        precinct_areas = []
        for p in community.precincts.values():
            X.append(p.centroid[0])
            Y.append(p.centroid[1])
            precinct_areas.append(p.coords.area)
        center = [average(X), average(Y)]
        radius = math.sqrt(sum(precinct_areas) / math.pi)

        for _ in range(M):

            # Communities that have exchanged precincts with `community`
            other_communities = set()

            giveable_precincts = get_giveable_precincts(
                graph, communities, community.id)
            
            for precinct, other_community in giveable_precincts:
                if get_distance(precinct.centroid, center) > radius:
                    community.give_precinct(
                        other_community, precinct.id)
                    if len(get_components(community.induced_subgraph)) > 1:
                        # Giving precinct made `community` non contiguous.
                        other_community.give_precinct(
                            community, precinct.id)
                    else:
                        other_communities.add(other_community)
            
            takeable_precincts = get_takeable_precincts(
                graph, communities, community.id)
            
            for precinct, other_community in takeable_precincts:
                if get_distance(precinct.centroid, center) <= radius:
                    other_community.give_precinct(
                        community, precinct.id)
                    if len(get_components(other_community.induced_subgraph)) > 1:
                        # Giving precicnt made `other_community` non contiguous.
                        community.give_precinct(
                            other_community, precinct.id)
                    else:
                        other_communities.add(other_community)
                
            for other_community in other_communities:
                other_community.update_imprecise_compactness()
            other_community.update_imprecise_compactness()

        rounded_compactnesses = [round(c.imprecise_compactness, 3) for c in communities]
        print(rounded_compactnesses, min(rounded_compactnesses))

        n += 1
        if n == len(communities):
            n = 0


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)
    communities = create_initial_configuration(graph, int(sys.argv[2]))
    print("Initial Configuration Generated")

    # TO LOAD INIT CONFIG FROM TEXT FILE:

    # with open("test_vermont_init_config.txt", "r") as f:
    #     precinct_list = eval(f.read())
    # communities = []

    # from hacking_the_election.utils.community import Community

    # for i, community in enumerate(precinct_list):
    #     c = Community(i, graph)
    #     for precinct_id in community:
    #         for node in graph.nodes():
    #             precinct = graph.node_attributes(node)[0]
    #             if precinct.id == precinct_id:
    #                 c.take_precinct(precinct)
    #     communities.append(c)

    animation_dir = None if sys.argv[3] == "none" else sys.argv[3]
    if animation_dir is not None:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

    start_time = time.time()
    optimize_compactness(communities, graph, animation_dir)
    print(f"Compactness optimization took {time.time() - start_time} seconds.")
    draw_state(graph, None, fpath="test_thing3.png")

    with open(sys.argv[4], "wb+") as f:
        pickle.dump(communities, f)
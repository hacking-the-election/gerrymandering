"""
Refines a configuration of political communities in a state so that
they all have Schwartzberg compactness scores above a certain threshold.

Usage:
python3 -m hacking_the_election.communities.compactness_refinement <serialized_state> <n_communities> (<animation_dir> | "none") <output_path>
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
from hacking_the_election.utils.geometry import get_compactness, get_distance
from hacking_the_election.utils.graph import (
    get_giveable_precincts,
    get_takeable_precincts
)
from hacking_the_election.visualization.misc import draw_state

# TMP
from hacking_the_election.visualization.map_visualization import visualize_map
from hacking_the_election.utils.visualization import COLORS, get_next_file_path


N = 40


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
        community.update_compactness()

    if animation_dir is not None:
        draw_state(graph, animation_dir)

    # The compactness of the least compact community after each iteration.
    compactnesses = []
    community_states = []
    while True:

        # Find least compact community.
        community = min(communities, key=lambda c: c.compactness)
        iteration_compactnesses = [c.compactness for c in communities]
        rounded_compactnesses = [round(c, 3) for c in iteration_compactnesses]
        min_compactness = min(iteration_compactnesses)
        print(rounded_compactnesses, min(rounded_compactnesses))

        # Exit function if solution is worse than all of previous N solutions.
        if len(compactnesses) > N:
            return_ = True
            for compactness in compactnesses[-N:]:
                if min_compactness > compactness:
                    return_ = False
            if return_:
                # Revert to best community state.
                best_communities = \
                    community_states[compactnesses.index(max(compactnesses))]
                for c in best_communities:
                    for c2 in communities:
                        if c.id == c2.id:
                            c2.precincts = c.precincts
                            c2.update_compactness()
                for c in communities:
                    for precinct in c.precincts.values():
                        precinct.community = c.id

                rounded_compactnesses = [round(c.compactness, 3) for c in communities]
                print(rounded_compactnesses, min(rounded_compactnesses))
                if animation_dir is not None:
                    draw_state(graph, animation_dir)
                return
        
        if compactnesses != []:
            # Check if anything changed. If not, exit the function.
            current_communities = set(tuple(p.id for p in c.precincts.values())
                for c in communities)
            last_communities = set(tuple(p.id for p in c.precincts.values())
                for c in community_states[-1])
            if current_communities == last_communities:
                print(rounded_compactnesses, min(rounded_compactnesses))
                if animation_dir is not None:
                    draw_state(graph, animation_dir)
                return

        compactnesses.append(min_compactness)
        # Not deepcopy so that precinct objects are not copied (saves memory).
        communities_copy = [copy.copy(c) for c in communities]
        community_states.append(communities_copy)
        
        # Get area of district.
        community_precinct_coords = \
            [p.coords for p in community.precincts.values()]
        community_coords = MultiPolygon(community_precinct_coords)
        center = list(community_coords.centroid.coords[0])
        radius = math.sqrt(community_coords.area / math.pi)

        # Communities that exchanged precincts with `community`.
        other_communities = set()
        
        # Give away precincts that are outside circle.
        giveable_precincts = get_giveable_precincts(graph, communities, community.id)
        # TMP
        # visualize_map(graph.nodes(), get_next_file_path("../images/test_vermont_2"), lambda n: graph.node_attributes(n)[0].coords, lambda n: COLORS[0] if graph.node_attributes(n)[0] in list(giveable_precincts.keys()) else COLORS[-1])
        print(len(giveable_precincts))
        for precinct, other_community in giveable_precincts.items():
            if get_distance(precinct.centroid, center) > radius:
                community.give_precinct(
                    other_community, precinct.id, update={"compactness"})
                other_communities.add(other_community)
        
        # Take precincts inside that are inside circle.
        takeable_precincts = get_takeable_precincts(graph, communities, community.id)
        for precinct, other_community in takeable_precincts.items():
            if get_distance(precinct.centroid, center) <= radius:
                other_community.give_precinct(
                    community, precinct.id, update={"compactness"})
                other_communities.add(other_community)
        
        if animation_dir is not None:
            # draw_state(graph, animation_dir, [Point(*center).buffer(radius)])
            draw_state(graph, animation_dir)

        for c in other_communities:
            c.update_compactness()
        community.update_compactness()


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)
    # communities = create_initial_configuration(graph, int(sys.argv[2]))
    with open("test_vermont_init_config.txt", "r") as f:
        precinct_list = eval(f.read())
    communities = []

    from hacking_the_election.utils.community import Community

    for i, community in enumerate(precinct_list):
        c = Community(i, graph)
        for precinct_id in community:
            for node in graph.nodes():
                precinct = graph.node_attributes(node)[0]
                if precinct.id == precinct_id:
                    c.take_precinct(precinct)
        communities.append(c)

    animation_dir = None if sys.argv[3] == "none" else sys.argv[3]
    if animation_dir is not None:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

    start_time = time.time()
    optimize_compactness(communities, graph, animation_dir)
    print(f"Compactness optimization took {time.time() - start_time} seconds.")

    with open(sys.argv[4], "wb+") as f:
        pickle.dump(communities, f)
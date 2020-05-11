"""
Refines a configuration of political communities in a state so that
the standard deviation of the partisanships of the precincts inside
of each of the communities is below a threshold.

Usage:
python3 -m hacking_the_election.communities.partisanship_refinement <serialized_state> <n_communities> (<animation_dir> | "none") <output_path>
"""

import copy
import os
import pickle
import sys

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.utils.graph import (
    get_components,
    get_giveable_precincts,
    get_takeable_precincts
)
from hacking_the_election.visualization.misc import draw_state


N = 40


def optimize_partisanship_stdev(communities, graph, animation_dir=None):
    """Takes a set of communities and exchanges precincts such that the standard deviation of the partisanships of their precincts are as low as possible.

    :param communities: The current state of the communities within a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param graph: A graph containing precinct data within a state.
    :type graph: `pygraph.classes.graph.graph`

    :param animation_dir: Path to the directory where animation files should be saved, defaults to None
    :type animation_dir: str or NoneType
    """

    for community in communities:
        community.update_partisanship_stdev()
    
    if animation_dir is not None:
        draw_state(graph, animation_dir)

    partisanship_stdevs = []  # Worst partisanship stdev after each iteration.
    community_states = []  # Community objects after each iteration.

    while True:
        
        # Find most diverse community.
        community = max(communities, key=lambda c: c.partisanship_stdev)
        iteration_stdevs = [c.partisanship_stdev for c in communities]
        min_stdev = min(iteration_stdevs)
        rounded_stdevs = [round(stdev, 3) for stdev in iteration_stdevs]
        print(rounded_stdevs, min(rounded_stdevs))
        # print(sum(rounded_stdevs) / len(rounded_stdevs))

        # Exit function if solution is worse than all of previous N solutions.
        if len(partisanship_stdevs) > N:
            return_ = True
            for stdev in partisanship_stdevs[-N:]:
                if min_stdev > stdev:
                    return_ = False
            if return_:
                # Revert to best community state.
                best_communities = \
                    community_states[partisanship_stdevs.index(max(partisanship_stdevs))]
                for c in best_communities:
                    for c2 in communities:
                        if c.id == c2.id:
                            c2.precincts = c.precincts
                            c2.update_partisanship_stdev()
                for c in communities:
                    for precinct in c.precincts.values():
                        precinct.community = c.id

                rounded_stdevs = [round(c.partisanship_stdev, 3) for c in communities]
                print(rounded_stdevs, min(rounded_stdevs))
                if animation_dir is not None:
                    draw_state(graph, animation_dir)
                return
        
        if partisanship_stdevs != []:
            # Check if anything changed. If not, exit the function.
            current_communities = set(tuple(p.id for p in c.precincts.values())
                for c in communities)
            last_communities = set(tuple(p.id for p in c.precincts.values())
                for c in community_states[-1])
            if current_communities == last_communities:

                # Revert to best community state.
                best_communities = \
                    community_states[partisanship_stdevs.index(
                        min(partisanship_stdevs))]
                for c in best_communities:
                    for c2 in communities:
                        if c.id == c2.id:
                            c2.precincts = c.precincts
                            c2.update_partisanship_stdev
                for c in communities:
                    for precinct in c.precincts.values():
                        precinct.community = c.id

                rounded_stdevs = [round(c.partisanship_stdev, 3) for c in communities]
                print(rounded_stdevs, min(rounded_stdevs))
                if animation_dir is not None:
                    draw_state(graph, animation_dir)
                return

        partisanship_stdevs.append(min_stdev)
        # Not deepcopy so that precinct objects are not copied (saves memory).
        communities_copy = [copy.copy(c) for c in communities]
        community_states.append(communities_copy)

        giveable_precincts = get_giveable_precincts(
            graph, communities, community.id)
        for precinct, other_community in giveable_precincts:

            starting_stdev = community.partisanship_stdev

            community.give_precinct(
                other_community, precinct.id, update={"partisanship_stdev"})
            # Check if exchange made `community` non-contiguous.
            # Or if it hurt the previous partisanship stdev.
            if (len(get_components(community.induced_subgraph)) > 1
                    or community.partisanship_stdev > starting_stdev):
                other_community.give_precinct(
                    community, precinct.id, update={"partisanship_stdev"})
            else:
                if animation_dir is not None:
                    draw_state(graph, animation_dir)
        
        takeable_precincts = get_takeable_precincts(
            graph, communities, community.id)
        for precinct, other_community in takeable_precincts:
            
            starting_stdev = community.partisanship_stdev

            other_community.give_precinct(
                community, precinct.id, update={"partisanship_stdev"})
            # Check if exchange made `community` non-contiguous.
            # Or if it hurt the previous partisanship stdev.
            if (len(get_components(other_community.induced_subgraph)) > 1
                    or community.partisanship_stdev > starting_stdev):
                community.give_precinct(
                    other_community, precinct.id, update={"partisanship_stdev"})
            else:
                if animation_dir is not None:
                    draw_state(graph, animation_dir)
    
if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)

    communities = create_initial_configuration(graph, int(sys.argv[2]))

    animation_dir = None if sys.argv[3] == "none" else sys.argv[3]
    if animation_dir is not None:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

    optimize_partisanship_stdev(communities, graph, animation_dir)

    with open(sys.argv[4], "wb+") as f:
        pickle.dump(communities, f)
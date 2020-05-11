"""
Refines a configuration of political communities in a state so that
they all have populations within a certain percent of each other.

Usage:
python3 -m hacking_the_election.communities.population_refinement <serialized_state> <n_communities> <max_pop_percentage> (<animation_dir> | "none") <output_path>
"""

from itertools import combinations
import os
import pickle
import random
import sys
import time

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.utils.graph import (
    get_giveable_precincts,
    get_takeable_precincts,
    get_components
)
from hacking_the_election.utils.stats import average
from hacking_the_election.visualization.misc import draw_state


def _check_communities_complete(communities, percentage):
    """Checks if communities have satifsfactory populations.

    :param communities: A list of communities.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param percentage: The maximum tolerated percentage difference between two communities.
    :type percentage: float

    :return: Whether or not the communities fulfil population requirements.
    :rtype: bool
    """

    for c1, c2 in combinations(communities, 2):
        if (abs(c1.population - c2.population)
                / average([c1.population, c2.population])) > (percentage / 200):
            return False

    return True


def optimize_population(communities, graph, percentage, animation_dir=None):
    """Takes a set of communities and exchanges precincts so that the population is as evenly distributed as possible.

    :param communities: The current state of the communities within a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param graph: A graph containing precinct data within a state.
    :type graph: `pygraph.classes.graph.graph`

    :param percentage: By how much the populations are able to deviate from one another. A value of 1% means that all the communities must be within 0.5% of the ideal population, so that they are guaranteed to be within 1% of each other.
    :type percentage: float between 0 and 1

    :param animation_dir: Path to the directory where animation files should be saved, defaults to None
    :type animation_dir: str or NoneType
    """

    for community in communities:
        community.update_population()

    if animation_dir is not None:
        draw_state(graph, animation_dir)

    ideal_population = average([c.population for c in communities])
    print(f"{ideal_population=}")

    while not _check_communities_complete(communities, percentage):

        # community = max(communities, key=lambda c: abs(c.population - ideal_population))
        community = random.choice(communities)

        if community.population > ideal_population:

            giveable_precincts = get_giveable_precincts(
                graph, communities, community.id)

            while community.population > ideal_population:
                
                if giveable_precincts == {}:
                    giveable_precincts = get_giveable_precincts(
                        graph, communities, community.id)

                precinct = random.choice(list(giveable_precincts.keys()))
                other_community = giveable_precincts[precinct]
                del giveable_precincts[precinct]
                
                community.give_precinct(
                    other_community, precinct.id, update={"population"})
                if len(get_components(community.induced_subgraph)) > 1:
                    # Give back precinct.
                    other_community.give_precinct(
                        community, precinct.id, update={"population"})

        elif community.population < ideal_population:

            takeable_precincts = get_takeable_precincts(
                graph, communities, community.id)

            while community.population < ideal_population:

                if takeable_precincts == {}:
                    takeable_precincts = get_takeable_precincts(
                        graph, communities, community.id)

                precinct = random.choice(list(takeable_precincts.keys()))
                other_community = takeable_precincts[precinct]
                del takeable_precincts[precinct]

                other_community.give_precinct(
                    community, precinct.id, update={"population"})
                if len(get_components(other_community.induced_subgraph)) > 1:
                    # Give back precinct.
                    community.give_precinct(
                        other_community, precinct.id, update={"population"})

        if animation_dir is not None:
            draw_state(graph, animation_dir)

        print([c.population for c in communities], community.population)


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)
    communities = create_initial_configuration(graph, int(sys.argv[2]))

    animation_dir = None if sys.argv[4] == "none" else sys.argv[4]
    if animation_dir is not None:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

    start_time = time.time()
    optimize_population(communities, graph, float(sys.argv[3]), animation_dir)
    print(f"Population optimization took {time.time() - start_time} seconds")

    with open(sys.argv[5], "wb+") as f:
        pickle.dump(communities, f)
"""
Implementation of the iterative method using refinement
processes for creating political communities.

Usage:
python3 -m hacking_the_election.communities <serialized_state_graph_path> <n_communities> <output_path> [animation_dir]
"""

import os
import pickle
import sys

from hacking_the_election.communities.compactness import \
    optimize_compactness
from hacking_the_election.communities.partisanship_stdev import \
    optimize_partisanship_stdev
from hacking_the_election.communities.population import \
    optimize_population
from hacking_the_election.communities.population_stdev import \
    optimize_population_stdev
from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.visualization.map_visualization import visualize_map


POPULATION_THRESHOLD = 1


def generate_communities(precinct_graph, n_communities, animation_dir=None):
    """Generates a set of political communities.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `pygraph.classes.graph.graph`

    :param n_communities: The number of communities to break the state into.
    :type n_communities: int

    :param animation_dir: Path to directory which will contain files for animation, defaults to None
    :type animation_dir: str or NoneType

    :return: A list of communities that represent the state.
    :rtype: list of `hacking_the_election.utils.community.Community`
    """

    if animation_dir is not None:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

    communities = create_initial_configuration(precinct_graph, n_communities)

    # while True:
    #     optimize_compactness(communities, precinct_graph, animation_dir)
    #     optimize_partisanship_stdev(communities, precinct_graph, animation_dir)
    #     optimize_population(communities, precinct_graph,
    #         POPULATION_THRESHOLD, animation_dir)
    #     optimize_population_stdev(communities, precinct_graph, animation_dir)
    for i in range(30):
        optimize_compactness(communities, precinct_graph, animation_dir)
        optimize_population(communities, precinct_graph,
            POPULATION_THRESHOLD, animation_dir)

    return communities


if __name__ == "__main__":
    
    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)

    animation_dir = None
    if len(sys.argv) > 4:
        animation_dir = sys.argv[4]

    communities = generate_communities(precinct_graph, int(sys.argv[2]), animation_dir)

    with open(sys.argv[3], "wb+") as f:
        pickle.dump(communities, f)
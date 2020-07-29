"""
Implementation of the simulated annealing and gradient ascent for
generating political communities.

Usage:
python3 -m hacking_the_election.communities <serialized_state_graph_path> <n_communities> <output_path> <algorithm> [animation_dir]

<algorithm> can either be "SA" or "GA"
"""

import pickle
import random
import sys

from pygraph.classes.graph import graph as Graph

from hacking_the_election.utils.community import Community
from hacking_the_election.utils.graph import (
    contract,
    light_copy
)
from hacking_the_election.visualization.misc import draw_state


def _get_score(communities):
    """Returns the goodness score of a list of communities.
    
    :param communities: A list of communities.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :return: A score between 0 and 1 representing the goodness of the inputted communities.
    :rtype: float
    """
    
    return 0.0

def _get_all_exchanges(precinct_graph, communities):
    """Finds all the possible ways that a community map can be changed via a single precinct exchanges between two communities.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `pygraph.classes.graph.graph`

    :param communities: A list of the communities that divy up the precincts in precinct_graph
    :type communities: list of `hacking_the_election.utils.community.Community`

    :return: A dictionary mapping precincts to lists of communities it can be given to.
    :rtype: dict mapping `hacking_the_election.utils.precinct.Precinct` to list of `hacking_the_election.utils.community.Community`
    """

    return {}


def _create_initial_configuration(precinct_graph, n_communities):
    """Creates a list of communities based off of a state precinct-map represented by a graph.

    Implementation of Karger-Stein algorithm, except modified a bit to
    make the partitions of similar sizes.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `pygraph.classes.graph.graph`
    """

    # Create copy of `precinct_graph` without precinct data.
    G = light_copy(precinct_graph)

    while len(G.nodes()) > n_communities:
        attr_lengths = {}  # Links edges to the number of nodes they contain.
        edges = set(G.edges())
        for i in range(min(100, len(edges))):
            e = edges.pop()
            attr_lengths[e] = (len(G.node_attributes(e[0]))
                             + len(G.node_attributes(e[1])))
        contract(G, min(attr_lengths))

    # Create community objects from nodes.
    communities = [Community(i, precinct_graph) for i in range(n_communities)]
    for i, node in enumerate(G.nodes()):
        for precinct_node in G.node_attributes(node):
            communities[i].take_precinct(
                precinct_graph.node_attributes(precinct_node)[0])

    return communities


def generate_communities_gradient_ascent(precinct_graph, n_communities, animation_dir=None):
    """Generates a set of political communities using gradient ascent.

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

    return communities


def generate_communities_simulated_annealing(precinct_graph, n_communities, animation_dir=None):
    """Generates a set of political communities using simulated annealing.

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

    return communities


if __name__ == "__main__":
    
    # Load input file and generate communities.
    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)

    args = [precinct_graph, int(sys.argv[2])]
    if len(sys.argv) > 5:
        args.append(sys.argv[5])

    if sys.argv[4] == "SA":
        communities = generate_communities_simulated_annealing(*args)
    elif sys.argv[4] == "GA":
        communities = generate_communities_gradient_ascent(*args)

    # Write to file.
    with open(sys.argv[3], "wb+") as f:
        pickle.dump(communities, f)
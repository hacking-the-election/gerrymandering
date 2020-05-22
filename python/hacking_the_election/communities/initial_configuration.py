"""Randomly groups precincts in a state to communities of equal (or as close as possible) size.

Usage:
python3 initial_configuration.py <serialized_state> <number_of_communities> <pickle_output> (<image_output> | "none")
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


def create_initial_configuration(precinct_graph, n_communities):
    """Creates a list of communities based off of a state precinct-map represented by a graph.

    Implementation of modified Karger-Stein algorithm.
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

    communities = [Community(i, precinct_graph) for i in range(n_communities)]
    for i, node in enumerate(G.nodes()):
        for precinct_node in G.node_attributes(node):
            communities[i].take_precinct(
                precinct_graph.node_attributes(precinct_node)[0])

    return communities


if __name__ == "__main__":
    
    # Load input file and calculate initial configuration.
    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)
    communities = create_initial_configuration(precinct_graph, int(sys.argv[2]))

    # Write to file.
    with open(sys.argv[3], "wb+") as f:
        pickle.dump(communities, f)

    # Display and/or save image.
    image_output = sys.argv[4] if sys.argv[4] != "none" else None
    if image_output is not None:
        draw_state(precinct_graph, None, fpath=image_output)
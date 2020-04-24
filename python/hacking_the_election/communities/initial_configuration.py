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
    get_node_number,
    light_copy
)
from hacking_the_election.utils.visualization import COLORS
from hacking_the_election.visualization.map_visualization import visualize_map


def create_initial_configuration(precinct_graph, n_communities):
    """Creates a list of communities based off of a state precinct-map represented by a graph.

    Implementation of modified Karger-Stein algorithm.
    """

    # Create copy of `precinct_graph` without precinct data.
    G = light_copy(precinct_graph)

    while len(G.nodes()) > n_communities:
        attr_lengths = {}  # Links edges to the number of nodes they contain.
        edges = G.edges()
        for i in range(min(100, len(edges))):
            e = random.choice(edges)
            attr_lengths[e] = (len(G.node_attributes(e[0]))
                             + len(G.node_attributes(e[1])))
        contract(G, min(attr_lengths))

    communities = [Community(i) for i in range(n_communities)]
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
    precinct_colors = {}
    for c, community in enumerate(communities):
        for precinct in community.precincts.values():
            precinct_colors[precinct] = COLORS[c]

    precincts = [precinct_graph.node_attributes(p)[0]
                 for p in precinct_graph.nodes()]
    image_output = None if sys.argv[4] == "none" else sys.argv[4]
    visualize_map(precincts, image_output, coords=lambda x: x.coords,
                  color=lambda x: precinct_colors[x], show=True)
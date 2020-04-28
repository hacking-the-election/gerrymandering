"""Unit test for initial configuration.

Usage:
python3 test_initial_configuration.py [state <serialized_state_path>] [-s]

-s -- Show the initial configurations once generated.
"""


import pickle
import sys
import unittest

from shapely.geometry import Point
from pygraph.classes.graph import graph
from pygraph.classes.exceptions import AdditionError

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.utils.precinct import Precinct
from hacking_the_election.visualization.graph_visualization import \
    visualize_graph
from hacking_the_election.visualization.map_visualization import \
    visualize_map
from hacking_the_election.utils.visualization import COLORS


STATE_PATH = "../data/bin/python/vermont.pickle"
SHOW = False  # Whether or not to show the communities generated when the algorithm is run.


class TestInitialConfiguration(unittest.TestCase):
    
    def test_grid_graph(self):
        """Tests random community generation with a graph of 100 precincts in a grid.
        """

        G1 = graph()  # A graph whose nodes are in a order of the grid.
        
        # Add nodes with Precinct object attributes.
        for x in range(10):
            for y in range(10):
                coords = Point(x * 10, y * 10).buffer(2)
                precinct = Precinct(0, coords, "North Montana",
                                    str(10*x + y), 1, 2)
                G1.add_node(int(precinct.id), attrs=[precinct])

        # Add edges so that degree can be calculated.
        for node in G1.nodes():
            if node % 10 != 9:
                G1.add_edge((node, node + 1))
            if node // 10 != 9:
                G1.add_edge((node, node + 10))

        ordered_nodes = sorted(G1.nodes(), key=lambda n: len(G1.neighbors(n)))

        G2 = graph()  # Graph whose node numbers correspond to their rank by degree.
        for node in G1.nodes():
            G2.add_node(ordered_nodes.index(node), attrs=G1.node_attributes(node))
        for node in G1.nodes():
            G2_node = ordered_nodes.index(node)
            for neighbor in G1.neighbors(node):
                try:
                    G2.add_edge((G2_node, ordered_nodes.index(neighbor)))
                except AdditionError:
                    pass

        communities = create_initial_configuration(G2, 2)

        def get_color(node):
            precinct = G2.node_attributes(node)[0]
            for c, community in enumerate(communities):
                if precinct in community.precincts.values():
                    return COLORS[c]

        if SHOW:
            visualize_graph(G2, None, lambda v: G2.node_attributes(v)[0].centroid,
                colors=get_color, sizes=lambda x: 20, show=True)


    def test_state(self):
        """Tests random community generation with an inputted state.
        """

        global STATE_PATH

        with open(STATE_PATH, "rb") as f:
            state_graph = pickle.load(f)
        
        communities = create_initial_configuration(state_graph, 2)
        for community in communities:
            community.update_coords()

        if SHOW:
            visualize_map(communities, None, lambda c: c.coords,
                color=lambda c: COLORS[communities.index(c)], show=True)


if __name__ == "__main__":

    SHOW = "-s" in sys.argv[1:]

    TestInitialConfiguration("test_grid_graph")()

    if len(sys.argv) > 1 and sys.argv[1] == "state":
        if len(sys.argv) > 2:
            STATE_PATH = sys.argv[2]
        TestInitialConfiguration("test_state")()
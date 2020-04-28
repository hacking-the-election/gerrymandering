import copy
import os
import pickle
import unittest

from shapely.geometry import Point
from pygraph.classes.graph import graph as Graph

from hacking_the_election.utils import graph
from hacking_the_election.utils.precinct import Precinct


SOURCE_DIR = os.path.dirname(__file__)


class TestGraph(unittest.TestCase):

    def setUp(self):
        """Loads data files and saves as instance attributes.
        """
        with open(f"{SOURCE_DIR}/data/graph/vermont_graph.pickle", "rb") as f:
            self.vermont_graph = pickle.load(f)

    def test_light_copy(self):
        """Tests `hacking_the_election.utils.graph.light_copy`
        """
        light_copy = graph.light_copy(self.vermont_graph)
        
        self.assertEqual(set(light_copy.nodes()), set(self.vermont_graph.nodes()))
        self.assertEqual(set(light_copy.edges()), set(self.vermont_graph.edges()))
        for v in light_copy.nodes():
            self.assertEqual(light_copy.node_attributes(v), [])

    def test_contract(self):
        """Tests `hacking_the_election.utils.graph.contract`
        """

        G = Graph()
        G.add_node(0); G.add_node(1)
        G.add_edge((0, 1))
        graph.contract(G, (0, 1))

        G2 = Graph()
        G2.add_node(0, attrs=[0, 1])

        self.assertEqual(G, G2)

    def test_get_components(self):
        """Tests `hacking_the_election.utils.graph.get_components`
        """

        vermont_components = len(graph.get_components(self.vermont_graph))
        self.assertEqual(vermont_components, 1)

        G = Graph()
        G.add_node(0); G.add_node(1)
        self.assertEqual(len(graph.get_components(G)), 2)
        

    def test_get_node_number(self):
        """Tests `hacking_the_election.utils.graph.get_node_number`
        """

        precinct_graph = Graph()
        precinct = Precinct(0, Point(0, 0).buffer(1), "North Montana", "0", 0, 0)
        precinct_graph.add_node(0, attrs=[precinct])
        graph.get_node_number(precinct, precinct_graph)


if __name__ == "__main__":
    unittest.main()
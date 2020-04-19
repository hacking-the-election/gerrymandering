from os.path import dirname
import unittest
import time

from shapely.geometry import Point
from pygraph.classes.graph import graph as Graph
from pygraph.readwrite.markup import read

from hacking_the_election.utils import graph
from hacking_the_election.utils.precinct import Precinct


SOURCE_DIR = dirname(__file__)


class TestGraph(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        
        super().__init__(*args, **kwargs)
        
        with open(f"{SOURCE_DIR}/data/random_graph.xml", "r") as f:
            self.random_graph = read(f.read())
        
        with open(f"{SOURCE_DIR}/data/vermont_graph.xml", "r") as f:
            self.vermont_graph = read(f.read())

        with open(f"{SOURCE_DIR}/data/vermont_graph_2.xml", "r") as f:
            self.vermont_graph_2 = read(f.read())  # Has all edges removed from node 27

    def test_get_discontinuous_components(self):

        self.assertEqual(len(graph.get_components(self.random_graph)), 1)
        start = time.time()
        vermont_components = len(graph.get_components(self.vermont_graph))
        print(f"vermont get_components: {time.time() - start}")
        self.assertEqual(vermont_components, 1)
        self.assertEqual(len(graph.get_components(self.vermont_graph_2)), 2)

    def test_remove_edges_to(self):
        
        start = time.time()
        vermont_removed_edges = graph.remove_edges_to('27', self.vermont_graph)
        print(f"vermont remove_edges_to: {time.time() - start}")
        self.assertEqual(vermont_removed_edges, self.vermont_graph_2)

    def test_get_node_number(self):

        precinct_graph = Graph()
        precinct = Precinct(0, Point(0, 0).buffer(1), "North Montana", "0", 0, 0)
        precinct_graph.add_node(0, attrs=[precinct])
        graph.get_node_number(precinct, precinct_graph)


if __name__ == "__main__":
    unittest.main()
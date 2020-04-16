from os.path import dirname
import unittest
import timeit

from pygraph.classes.graph import graph as Graph
from pygraph.readwrite.markup import read

from hacking_the_election.utils import graph


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
        
        self.assertEqual(graph.get_discontinuous_components(self.random_graph), 1)
        self.assertEqual(graph.get_discontinuous_components(self.vermont_graph), 1)
        self.assertEqual(graph.get_discontinuous_components(self.vermont_graph_2), 2)

    def test_remove_edges_to(self):
        
        self.assertEqual(graph.remove_edges_to('27', self.vermont_graph), self.vermont_graph_2)


if __name__ == "__main__":
    unittest.main()
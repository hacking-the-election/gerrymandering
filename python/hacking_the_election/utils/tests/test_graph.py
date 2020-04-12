from os.path import dirname
import unittest

from pygraph.classes.graph import graph as Graph
from pygraph.readwrite.markup import read

from hacking_the_election.utils import graph


SOURCE_DIR = dirname(__file__)


class TestGraph(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        
        super().__init__(*args, **kwargs)
        
        with open(f"{SOURCE_DIR}/data/random_graph.xml", "r") as f:
            self.random_graph = read(f.read())
        
        self.disconnected_graph = Graph()
        self.disconnected_graph.add_node(1)
        self.disconnected_graph.add_node(2)

    def test_get_discontinuous_components(self):
        
        self.assertEqual(graph.get_discontinuous_components(self.random_graph), 1)
        self.assertEqual(graph.get_discontinuous_components(self.disconnected_graph), 2)

    def test_remove_edges_to(self):
        pass


if __name__ == "__main__":
    unittest.main()
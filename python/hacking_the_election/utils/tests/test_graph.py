from os.path import dirname
import pickle
import time
import unittest

from shapely.geometry import Point
from pygraph.classes.graph import graph as Graph

from hacking_the_election.utils import graph
from hacking_the_election.utils.precinct import Precinct


SOURCE_DIR = dirname(__file__)


class TestGraph(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        
        super().__init__(*args, **kwargs)
        
        with open(f"{SOURCE_DIR}/data/random_graph.pickle", "rb") as f:
            self.random_graph = pickle.load(f)
        
        with open(f"{SOURCE_DIR}/data/vermont_graph.pickle", "rb") as f:
            self.vermont_graph = pickle.load(f)

        with open(f"{SOURCE_DIR}/data/vermont_graph_2.pickle", "rb") as f:
            self.vermont_graph_2 = pickle.load(f)  # Has all edges removed from node 27

    def test_get_components(self):

        self.assertEqual(len(graph.get_components(self.random_graph)), 1)
        start = time.time()
        vermont_components = len(graph.get_components(self.vermont_graph))
        print(f"vermont get_components: {time.time() - start}")
        self.assertEqual(vermont_components, 1)
        self.assertEqual(len(graph.get_components(self.vermont_graph_2)), 2)

    def test_get_node_number(self):

        precinct_graph = Graph()
        precinct = Precinct(0, Point(0, 0).buffer(1), "North Montana", "0", 0, 0)
        precinct_graph.add_node(0, attrs=[precinct])
        graph.get_node_number(precinct, precinct_graph)


if __name__ == "__main__":
    unittest.main()
"""Unit tests for `hacking_the_election.utils.graph`
"""


import copy
import os
import pickle
import unittest

from shapely.geometry import Point, Polygon
from pygraph.classes.graph import graph as Graph
from pygraph.classes.digraph import digraph as DirectedGraph

from hacking_the_election.utils import graph
from hacking_the_election.utils.community import Community
from hacking_the_election.utils.precinct import Precinct


SOURCE_DIR = os.path.dirname(__file__)


def _get_precinct_from_id(graph, id_):
    """Gets a precinct object from a precinct id.

    :param graph: A graph with precincts as node attributes.
    :type graph: `pygraph.classes.graph.graph`

    :param id_: The id of a precinct in `graph`.
    :type id_: str

    :return: The precinct with id `id_`
    :rtype: `hacking_the_election.utils.precinct.Precinct`
    """

    for node in graph.nodes():
        precinct = graph.node_attributes(node)[0]
        if precinct.id == id_:
            return precinct


with open(f"{SOURCE_DIR}/data/vermont_graph.pickle", "rb") as f:
    VERMONT_GRAPH = pickle.load(f)
COMMUNITIES = graph.create_initial_configuration(VERMONT_GRAPH, 2)


class TestGraph(unittest.TestCase):

    def test_light_copy(self):
        """Tests `hacking_the_election.utils.graph.light_copy`
        """
        light_copy = graph.light_copy(VERMONT_GRAPH)
        
        self.assertEqual(set(light_copy.nodes()), set(VERMONT_GRAPH.nodes()))
        self.assertEqual(set(light_copy.edges()), set(VERMONT_GRAPH.edges()))
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

        vermont_components = len(graph.get_components(VERMONT_GRAPH))
        self.assertEqual(vermont_components, 1)

        G = Graph()
        G.add_node(0); G.add_node(1)
        self.assertEqual(len(graph.get_components(G)), 2)

    def test_get_induced_subgraph(self):
        """Tests `hacking_the_election.utils.graph.get_induced_subgraph`
        """

        precinct_ids = ["50013VD102", "50011VD101", "50011VD95", "50011VD93"]
        precincts = [_get_precinct_from_id(VERMONT_GRAPH, id_)
                     for id_ in precinct_ids]
        
        precincts_graph = graph.get_induced_subgraph(VERMONT_GRAPH, precincts)
        
        self.assertEqual(len(precincts_graph.nodes()), 4)
        self.assertEqual(len(precincts_graph.edges()), 6)

    def test_get_giveable_precincts(self):
        """Tests `hacking_the_election.utils.graph.get_giveable_precincts`
        """
        
        giveable_precincts = graph.get_giveable_precincts(VERMONT_GRAPH, COMMUNITIES, 0)

        # Check that a community that is equal to the state has no giveable precincts.
        at_large = Community(0, VERMONT_GRAPH)
        self.assertEqual(
            graph.get_giveable_precincts(VERMONT_GRAPH,
                [at_large], 0),
            []
        )

    def test_get_takeable_precincts(self):
        """Tests `hacking_the_election.utils.graph.get_takeable_precincts`
        """

        takeable_precincts = graph.get_takeable_precincts(VERMONT_GRAPH, COMMUNITIES, 1)

        # Check that a community that is equal to the state has no giveable precincts.
        at_large = Community(0, VERMONT_GRAPH)
        self.assertEqual(
            graph.get_takeable_precincts(VERMONT_GRAPH,
                [at_large], 0),
            []
        )
    
    def test_get_articulation_points(self):
        """Tests `hacking_the_election.utils.graph.get_articulation_points`
        """

        g = Graph()
        for v in range(1, 7):
            g.add_node(v)
        edges = [(1, 2), (2, 3), (1, 4), (4, 5), (5, 6), (6, 4)]
        for edge in edges:
            g.add_edge(edge)

        self.assertEqual(graph.get_articulation_points(g), {1, 2, 4})

    def test_get_initial_configuration(self):
        """Tests `hacking_the_election.utils.graph.create_initial_configuration`
        """

        communities = graph.create_initial_configuration(VERMONT_GRAPH, 3)
        self.assertEqual(len(communities), 3)
        for community in communities:
            self.assertIsInstance(community, Community)
            self.assertEqual(len(graph.get_components(community.induced_subgraph)), 1)


if __name__ == "__main__":
    unittest.main()
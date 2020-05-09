"""Unit tests for `hacking_the_election.utils.graph`
"""


import copy
import os
import pickle
import unittest

from shapely.geometry import Point, Polygon
from pygraph.classes.graph import graph as Graph

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
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


class TestGraph(unittest.TestCase):

    def setUp(self):
        """Loads data files and saves as instance attributes.
        """
        with open(f"{SOURCE_DIR}/data/graph/vermont_graph.pickle", "rb") as f:
            self.vermont_graph = pickle.load(f)
        self.communities = create_initial_configuration(self.vermont_graph, 2)

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

    def test_get_induced_subgraph(self):
        """Tests `hacking_the_election.utils.graph.get_induced_subgraph`
        """

        precinct_ids = ["50013VD102", "50011VD101", "50011VD95", "50011VD93"]
        precincts = [_get_precinct_from_id(self.vermont_graph, id_)
                     for id_ in precinct_ids]
        
        precincts_graph = graph.get_induced_subgraph(self.vermont_graph, precincts)
        
        self.assertEqual(len(precincts_graph.nodes()), 4)
        self.assertEqual(len(precincts_graph.edges()) / 2, 3)

    def test_get_giveable_precincts(self):
        """Tests `hacking_the_election.utils.graph.get_giveable_precincts`
        """
        
        giveable_precincts = graph.get_giveable_precincts(
            self.vermont_graph, self.communities[0].induced_subgraph)

        for c in self.communities:
            c.update_coords()

        # Check contiguity of both communities as precincts are exchanged.
        for id_ in giveable_precincts:
            self.communities[0].give_precinct(self.communities[1],
                id_, update={"coords"})
            self.assertIsInstance(self.communities[0].coords, Polygon)
            self.assertIsInstance(self.communities[1].coords, Polygon)
            self.communities[1].give_precinct(self.communities[0],
                id_, update={"coords"})

        # Check that a community that is equal to the state has no giveable precincts.
        at_large = Community(0, self.vermont_graph)
        self.assertEqual(
            graph.get_giveable_precincts(self.vermont_graph,
                at_large.induced_subgraph),
            []
        )

    def test_get_takeable_precincts(self):
        """Tests `hacking_the_election.utils.graph.get_takeable_precincts`
        """

        # TODO: Add more elaborate test.
        # takeable_precincts = \
        #     [p.id for p in graph.get_takeable_precincts(self.vermont_graph, self.communities, 0).keys()]

        # Check that a community that is equal to the state has no giveable precincts.
        at_large = Community(0, self.vermont_graph)
        self.assertEqual(
            graph.get_takeable_precincts(self.vermont_graph,
                [at_large], 0),
            {}
        )


if __name__ == "__main__":
    unittest.main()
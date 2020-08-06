"""Unit tests for `hacking_the_election.utils.community`
"""

import os
import pickle
import sys
import unittest

import networkx as nx
from shapely.ops import unary_union

from hacking_the_election.utils import community
from hacking_the_election.visualization.map_visualization import visualize_map
from hacking_the_election.visualization.misc import draw_communities


source_dir = os.path.dirname(__file__)
with open(f"{source_dir}/data/vermont_graph.pickle", "rb") as f:
    VERMONT = pickle.load(f)

ALL_ATTRS = ["coords", "partisanship", "partisanship_stdev", "population"]
# Find 3 specific precincts to use for testing.
PRECINCTS = set()
for node in VERMONT.nodes:
    precinct = VERMONT.nodes[node]['precinct']
    if precinct.id in {'50003VD30', '50003VD25-1', '50003VD25-2'}:
        PRECINCTS.add(precinct)

# Global variable as to whether or not windows
# should pop up with geometric figures.
SHOW = False


class TestCommunity(unittest.TestCase):
    
    # -------- Tests for community class -------- #

    def test_take_precinct(self):
        """Tests `hacking_the_election.utils.community.Community.take_precinct`

        Also tests various 'update_<attribute>' methods.
        """
        test_community = community.Community(0, VERMONT)

        # Have the community take those precincts, only udpating some
        # attributes (including coords), so we can check that the rest
        # aren't changed.
        for precinct in PRECINCTS:
            test_community.take_precinct(precinct, update={"coords", "partisanship"})
        # Coords.
        correct_coords = unary_union([p.coords for p in PRECINCTS])
        try:
            self.assertEqual(
                test_community.coords,
                correct_coords
            )
        except AssertionError:
            # This is so that you can make a visual comparison if there
            # are some floating point discrepancies.
            if SHOW:
                visualize_map([test_community.coords],
                    output_path="test_community1.png", show=True)
        # Partisanships.
        community_partisanship = [round(part, 7) for part in test_community.partisanship]
        correct_partisanship = {
            "rep": round(0.3000054408521879, 7),
            "dem": round(0.6744106031874885, 7),
            "other": round(0.025583955960323628, 7)
        }
        for party in correct_partisanship:
            with self.subTest(party=party):
                self.assertIn(correct_partisanship[party], community_partisanship)
        # Unchanged attributes.
        self.assertEqual(test_community.partisanship_stdev, 0)
        self.assertEqual(test_community.population, 0)

        # Update the other ones manually, and see if they work.
        # Hold off on compactness measures and population stdev for now
        # (save them for after we implement Brian Olson compactness).
        test_community.update_partisanship_stdev()
        test_community.update_population()
        self.assertAlmostEqual(test_community.partisanship_stdev, 0.017394752448622267)
        self.assertEqual(test_community.population, 19291.0)

        # Check that the parties are correct.
        self.assertEqual(
            set(test_community.parties),
            {'percent_dem', 'percent_rep', 'percent_other'}
        )
        # Check induced subgraph.
        correct_induced_subgraph = nx.Graph()
        correct_induced_subgraph.add_edges_from([(44, 25), (44, 28), (25, 28)])
        self.assertEqual(
            set(test_community.induced_subgraph.nodes),
            set(correct_induced_subgraph.nodes)
        )
        self.assertEqual(
            {frozenset(edge) for edge in test_community.induced_subgraph.edges},
            {frozenset(edge) for edge in correct_induced_subgraph.edges}
        )

    def test_give_precinct(self):
        """Tests `hacking_the_election.utils.community.Community.give_precinct`
        """
        test_community = community.Community(0, VERMONT)
        for precinct in PRECINCTS:
            if precinct not in test_community.precincts:
                test_community.take_precinct(precinct, update=set(ALL_ATTRS))

        # Create another community.
        com2 = community.Community(1, VERMONT)
        # Give one precinct to that community, updating all variables
        # except compactnesses and pop stdev.
        community_coords = test_community.coords
        test_community.give_precinct(com2, "50003VD25-1", update=set(ALL_ATTRS))
        # Coords.
        correct_coords = community_coords.difference(
            com2.precincts["50003VD25-1"].coords)
        self.assertEqual(
            test_community.coords,
            correct_coords
        )
        # Population.
        self.assertEqual(test_community.population, 11294.0)
        # Partisanship.
        community_partisanship = [round(part, 7) for part in test_community.partisanship]
        correct_partisanship = {
            "rep": round(0.3157258014309087, 7),
            "dem": round(0.6586660955649135, 7),
            "other": round(0.025608103004177842, 7)
        }
        for party in correct_partisanship:
            with self.subTest(party=party):
                self.assertIn(correct_partisanship[party], community_partisanship)
        # Partisanship standard deviation.
        self.assertAlmostEqual(test_community.partisanship_stdev, 0.010620057680170132)

        # Try to give a precinct to itself.
        with self.assertRaises(ValueError):
            test_community.give_precinct(test_community, "50003VD30")
        # Try to give a precinct it doesn't have.
        with self.assertRaises(ValueError):
            test_community.give_precinct(com2, "50007VD63")
        # Try to give the only precinct of a community.
        # Make sure it returns False and doesn't do anything.
        com1_attrs = [
            test_community.coords, test_community.partisanship,
            test_community.partisanship_stdev, test_community.population
        ]
        com2_attrs = [
            com2.coords, com2.partisanship,
            com2.partisanship_stdev, com2.population
        ]
        self.assertFalse(
            com2.give_precinct(test_community, "50003VD30", update=set(ALL_ATTRS)))
        for i, attr in enumerate(ALL_ATTRS):
            with self.subTest(attr=attr + "-com1"):
                self.assertEqual(com1_attrs[i], eval(f"test_community.{attr}"))
            with self.subTest(attr=attr + "-com2"):
                self.assertEqual(com2_attrs[i], eval(f"com2.{attr}"))
    
    def test_dem_rep_partisanship(self):
        # TODO: Implement this method.
        pass

    def test_from_text_file(self):
        # TODO: Implement this method.
        pass
    
    # -------- Tests for functions -------- #

    def test_create_initial_configuration(self):
        """Tests `hacking_the_election.utils.community.create_initial_configuration`
        """
        communities = community.create_initial_configuration(VERMONT, 3)
        nodes = set()
        for c in communities:
            self.assertGreater(len(c.precincts), 0)
            for precinct in c.precincts.values():
                nodes.add(precinct.node)
        self.assertEqual(nodes, set(VERMONT.nodes))
        if SHOW:
            draw_communities(
                communities, output_path="test_initconfig.png", show=True)


if __name__ == "__main__":
    if "-s" in sys.argv[1:]:
        SHOW = True
        sys.argv.remove("-s")

    unittest.main()
"""
Unit tests for communities.py
"""

import unittest
import pickle

from shapely.geometry import Polygon, MultiPolygon

from gerrymandering.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.utils.initial_configuration import Community
from hacking_the_election.serialization.load_precincts import load
from hacking_the_election.serialization.save_precincts import Precinct
from hacking_the_election.test.funcs import (
    multipolygon_to_list,
    polygon_to_list,
    convert_to_json,
    print_time
)


VERMONT = load("../data/test/python/vermont.pickle")
ALASKA = load("../data/test/python/alaska.pickle")


class TestInitialConfiguration(unittest.TestCase):
    """
    Tests for first step in communities algorithm
    """
    
    def test_initial_configuration(self):

        @print_time
        def test_initial_configuration_speed(precincts, n_districts,
                                            state_border):
            return create_initial_configuration(
                precincts, n_districts, state_border)

        vermont_output = test_initial_configuration_speed(
            VERMONT[0], 2, VERMONT[2])
        with open("test_communities.pickle", "wb") as f:
            pickle.dump(vermont_output, f)
        vermont_output_coords = []
        for community in vermont_output:
            if isinstance(community.coords, MultiPolygon):
                vermont_output_coords.append(multipolygon_to_list(community.coords))
            elif isinstance(community.coords, Polygon):
                vermont_output_coords.append(polygon_to_list(community.coords))

        convert_to_json(vermont_output_coords, "test_communities.json")

if __name__ == "__main__":
    unittest.main()

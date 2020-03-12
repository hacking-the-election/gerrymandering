"""
Unit tests for communities.py
"""

import unittest
import pickle

from shapely.geometry import Polygon, MultiPolygon

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.utils.community import Community
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

    @staticmethod
    @print_time
    def test_initial_configuration_speed(precincts, n_districts, state_border):
        return create_initial_configuration(precincts, n_districts, state_border)

    @staticmethod
    def test_state(state):
        state_output = \
            TestInitialConfiguration.test_initial_configuration_speed(
                state[0], 3, state[2])
        with open("../data/test/python/vermont_initial_configuration.pickle", "wb") as f:
            pickle.dump(state_output, f)
        state_output_coords = []
        for community in state_output[0]:
            if isinstance(community.coords, MultiPolygon):
                state_output_coords.append(multipolygon_to_list(community.coords))
            elif isinstance(community.coords, Polygon):
                state_output_coords.append(polygon_to_list(community.coords))

        convert_to_json(state_output_coords, "../data/test/python/vermont_initial_configuration.json")
    
    def test_vermont(self):
        TestInitialConfiguration.test_state(VERMONT)

    def test_alaska(self):
        TestInitialConfiguration.test_state(ALASKA)


if __name__ == "__main__":
    unittest.main()

"""
Unit tests for communities.py
"""

import sys
from os.path import abspath, dirname
import unittest
import pickle

from shapely.geometry import Polygon, MultiPolygon

sys.path.append(abspath(dirname(dirname(dirname(__file__)))))

from gerrymandering import communities
from gerrymandering.utils import Community
from utils import *
from serialization.load_precincts import load
from serialization.save_precincts import Precinct


VERMONT = load("../data/test/python/alaska.pickle")
ALASKA = load("../data/test/python/alaska.pickle")


class TestInitialConfiguration(unittest.TestCase):
    """
    Tests for first step in communities algorithm
    """
    
    def test_initial_configuration(self):

        @print_time
        def test_initial_configuration_speed(precincts, n_districts,
                                            state_border):
            return communities.create_initial_configuration(
                precincts, n_districts, state_border)

        alaska_output = test_initial_configuration_speed(
            ALASKA[0], 2, ALASKA[2])
        with open("test_communities.pickle", "wb") as f:
            pickle.dump(alaska_output, f)
        alaska_output_coords = []
        for community in alaska_output:
            if isinstance(community.coords, MultiPolygon):
                alaska_output_coords.append(multipolygon_to_list(community.coords))
            elif isinstance(community.coords, Polygon):
                alaska_output_coords.append(polygon_to_list(community.coords))

        convert_to_json(alaska_output_coords, "test_communities.json")

if __name__ == "__main__":
    unittest.main()

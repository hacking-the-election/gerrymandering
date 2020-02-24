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


VERMONT = load("../data/test/python/vermont.pickle")


class TestInitialConfiguration(unittest.TestCase):
    """
    Tests for first step in communities algorithm
    """
    
    def test_initial_configuration(self):

        @print_time
        def test_initial_configuration_speed(precincts, n_districts):
            return communities.create_initial_configuration(precincts, n_districts)

        vermont_output = test_initial_configuration_speed(VERMONT[0], 2)
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

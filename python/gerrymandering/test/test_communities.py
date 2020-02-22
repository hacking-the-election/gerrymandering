"""
Unit tests for communities.py
"""

import sys
from os.path import abspath, dirname
import unittest
import unittest

sys.path.append(abspath(dirname(dirname(dirname(__file__)))))

from gerrymandering import communities
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

        convert_to_json([community.border for community in vermont_output], "test_communities.json")

if __name__ == "__main__":
    unittest.main()
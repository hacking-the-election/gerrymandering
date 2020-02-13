"""
Unit tests for utils.py
"""

import sys
from os.path import abspath, dirname
import unittest

sys.path.append(abspath(dirname(dirname(dirname(__file__)))))

from utils import convert_to_json
from serialization.load_precincts import load
from serialization.save_precincts import Precinct
from gerrymandering.utils import *


class TestUtils(unittest.TestCase):

    def __init__(self, *args, **kwargs):

        unittest.TestCase.__init__(self, *args, **kwargs)

        self.vermont = load(dirname(__file__) + "/vermont.pickle")
        for precinct in self.vermont[0]:
            coords = precinct.coords[:]
            while type(coords[0][0]) != type(1.0):
                coords = coords[0]
            precinct.coords = coords


    def test_get_border(self):
        pass

    def test_get_schwartsberg_compactness(self):
        pass

    def test_get_if_addable(self):
        pass

    def test_community(self):
        """
        To test Community class
        """
        pass


if __name__ == "__main__":
    unittest.main()


"""
Unit tests for utils.py
"""

import json
from sys import path
from os.path import abspath, dirname
import unittest

from pyclipper import scale_to_clipper

path.append(abspath(dirname(dirname(dirname(__file__)))) + "/serialization")
path.append(abspath(dirname(dirname(__file__))))

from load_precincts import load
from save_precincts import Precinct
from utils import *


def convert_to_json(coords, output_file):
    """
    Writes `coords` to `output_file` as geojson

    Args:
    `coords`: list of features (each a list of coords)
    """

    features = []
    for feature in coords:
        features.append({
            "type": "Feature",
            "geometry": {
                "type": "Polygon",
                "coordinates": feature
            }
        })
        features.append(feature)

    with open(output_file, 'w+') as f:
        json.dump({"type":"FeatureCollection", "features":features}, f)


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


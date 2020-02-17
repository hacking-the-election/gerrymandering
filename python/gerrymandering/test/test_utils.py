"""
Unit tests for utils.py
"""

import sys
from os.path import abspath, dirname
import unittest

import numpy as np

sys.path.append(abspath(dirname(dirname(dirname(__file__)))))

from utils import convert_to_json
from serialization.load_precincts import load
from serialization.save_precincts import Precinct
from gerrymandering.utils import get_equation, get_segments, get_border


class TestUtils(unittest.TestCase):

    def __init__(self, *args, **kwargs):

        unittest.TestCase.__init__(self, *args, **kwargs)

        self.vermont = load(dirname(__file__) + "/vermont.pickle")
        # self.vermont = load("/Users/Mukeshkhare/Desktop/GCRSEF2020/data/bin/python/alabama.pickle")

    def test_get_equation(self):
        
        seg1 = np.array([[3, 1], [7, 0]])
        seg2 = np.array([[0, 0], [0, 0]])
        seg3 = np.array([])

        self.assertEqual(-748.25, get_equation(seg1)(3000))
        with self.assertRaises(ValueError):
            get_equation(seg2)
        with self.assertRaises(ValueError):
            get_equation(seg3)


    def test_get_segments(self):

        # normal polygon
        shape_1 = np.array([[0, 0], [2, 0], [2, 2], [0, 2]])
        # vermont polygon
        shape_2 = np.array(self.vermont[0][0].coords)

        # self.assertEqual(get_segments(shape_1))
        
        # with self.assertRaises(ValueError):
        #     pass


    def test_get_border(self):
        # convert_to_json(get_border([p.coords for p in self.vermont[0]]), "test_vermont.json")
        # shapes_1 = [[[0, 0], [0, 5], [2, 4], [3, 2]], [[2.5, 3], [3.5, 1], [6, 6], [6, 0]]]

        convert_to_json(get_border([p.coords for p in self.vermont[0]]), "test_vermont_border.json")
        # print(get_border([p.coords for p in self.vermont[0][:2]]))

        # convert_to_json([p.coords for p in self.vermont[0][:2]], "test_get_border.json")

        # print(get_border(shapes_1))

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


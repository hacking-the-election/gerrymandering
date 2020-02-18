"""
Unit tests for utils.py
"""

import sys
from os.path import abspath, dirname
import unittest
import time
import pickle

import numpy as np
import matplotlib.pyplot as plt

sys.path.append(abspath(dirname(dirname(dirname(__file__)))))

from utils import convert_to_json
from serialization.load_precincts import load
from serialization.save_precincts import Precinct
from gerrymandering.utils import (get_equation, get_segments, clip, UNION,
                                  DIFFERENCE, get_schwartsberg_compactness,
                                  get_if_bordering, get_point_in_polygon)


DATA_DIR = "../data/test/python"


def print_time(func):
    def timed_func(*args, **kwargs):
        start_time = time.time()
        output = func(*args, **kwargs)
        print(f"{func.__name__} took {time.time() - start_time} seconds")
        return output
    return timed_func


def return_time(func):
    def timed_func(*args, **kwargs):
        start_time = time.time()
        func(*args, **kwargs)
        return time.time() - start_time
    return timed_func


class TestClipping(unittest.TestCase):
    """
    Tests for polygon clipping functions
    """

    # complete pickle state data files
    # ================================

    # for small tests that shouldnt take long
    vermont = load(DATA_DIR + "/vermont.pickle")
    # for small tests that need more than one district
    connecticut = load(DATA_DIR + "/connecticut.pickle")

    # function-specific test data
    # ================================
    
    difference_test_data = load(DATA_DIR \
                                + "/difference_test_data.pickle")
    border_test_data = load(DATA_DIR \
                                + "/border_test_data.pickle")


    @classmethod
    def plot_get_border(cls):
        """
        Creates scatter plot of all relationship between number of
        precincts and time
        """

        timed_get_border = lambda shapes: return_time(clip)(shapes, UNION)

        X = np.arange(1, len(cls.vermont[0]) + 1)
        Y = np.array([])
        for i in range(1, len(cls.vermont[0]) + 1):
            t = timed_get_border([p.coords for p in cls.vermont[0][:i]])
            Y = np.append(Y, [t])
        
        plt.scatter(X, Y)
        plt.show()

    def __init__(self, *args, **kwargs):

        unittest.TestCase.__init__(self, *args, **kwargs)

    def test_get_border(self):
        """
        Border of precincts of vermont
        """

        @print_time
        def test_get_border_speed():
            return clip([p.coords for p in self.vermont[0]], UNION)

        self.assertEqual(test_get_border_speed(), self.border_test_data)

    def test_get_difference(self):
        """
        Difference between outer border of connecticut and
        its 5th congressional district
        """

        @print_time
        def test_get_difference_speed():
            connecticut_district_coords = [
                d["geometry"]["coordinates"]
                for d in self.connecticut[1]["features"]]

            outer_border = clip(connecticut_district_coords, UNION)
            difference = clip([outer_border, connecticut_district_coords[4]],
                              DIFFERENCE)

            return difference

        self.assertEqual(test_get_difference_speed(), self.difference_test_data)


class TestGeometry(unittest.TestCase):
    """
    Tests for geometric calculations
    """

    def __init__(self, *args, **kwargs):

        unittest.TestCase.__init__(self, *args, **kwargs)

        # poly1 has a hole, poly2 does not
        self.poly1, self.poly2 = load(
            DATA_DIR + "/geometry_test_data.pickle")
        # both polygons are precincts from arkansas
        # poly1 has geoid 05073008 and poly2 has geoid 05027020

    def test_get_compactness(self):

        @print_time
        def test_get_compactness_speed(polygon):
            return get_schwartsberg_compactness(polygon)

        # later to be tested with cpp outputs
        print("compactness:")
        print(f"poly1: {test_get_compactness_speed(self.poly1)}")
        print(f"poly2: {test_get_compactness_speed(self.poly2)}")

    def test_get_point_in_polygon(self):

        @print_time
        def test_get_point_in_polygon_speed(polygon, point):
            return get_point_in_polygon(polygon, point)

        # point is inside hole in polygon
        self.assertEquals(
            test_get_point_in_polygon_speed(
                    self.poly1,
                    (456625, 3662331)
                ),
            False
            )
        # point is inside polygon
        self.assertEquals(
            test_get_point_in_polygon_speed(
                    self.poly2,
                    (456582, 3669030)
                ),
                True
            )

    def test_get_if_bordering(self):
        
        @print_time
        def test_get_if_bordering(polygon1, polygon2):
            return get_if_bordering(polygon1, polygon2)

        self.assertEqual(test_get_if_bordering(self.poly1, self.poly2), True)


class TestCommunities(unittest.TestCase):
    """
    Tests for community algorithm-specfic functions
    """

    def test_get_addable_precincts(self):
        pass

    def test_get_exchangeable_precincts(self):
        pass


if __name__ == "__main__":

    if "border_graph" in sys.argv:
        TestClipping.plot_get_border()
    else:
        clipping = TestClipping
        communities = TestCommunities
        geometry = TestGeometry

        unittest.main()

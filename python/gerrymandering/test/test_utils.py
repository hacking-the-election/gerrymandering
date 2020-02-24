"""
Unit tests for utils.py
"""

import sys
from os.path import abspath, dirname
import unittest
import time

import numpy as np
import matplotlib.pyplot as plt
from shapely.geometry import Polygon, MultiPolygon

sys.path.append(abspath(dirname(dirname(dirname(__file__)))))

from utils import *
from serialization.load_precincts import load
from serialization.save_precincts import Precinct
from gerrymandering.utils import *


DATA_DIR = "../data/test/python"


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
        def test_get_border_speed(shapes):
            return clip(shapes, UNION)

        shapely_vermont = []
        for p in self.vermont[0]:
            try:
                shapely_vermont.append(
                    polygon_to_shapely(p.coords)
                )
            except ValueError:
                shapely_vermont.append(
                    multipolygon_to_shapely(p.coords)
                )
        union = test_get_border_speed(shapely_vermont)
        
        convert_to_json([polygon_to_list(union)], "test_polys.json")

    def test_get_difference(self):
        """
        Difference between outer border of connecticut and
        its 5th congressional district
        """

        connecticut_district_coords = [
            d["geometry"]["coordinates"]
            for d in self.connecticut[1]["features"]]
        district_multipolygons = []
        for district in connecticut_district_coords:
            polygons = []
            for polygon in district:
                polygons.append(polygon_to_shapely(polygon))
            district_multipolygons.append(MultiPolygon(polygons))
        outer_border = clip(district_multipolygons, UNION)

        @print_time
        def test_get_difference_speed(shapes):
            return clip(shapes, DIFFERENCE)

        difference = test_get_difference_speed([outer_border, district_multipolygons[4]])

        # print(multipolygon_to_list(difference))
        convert_to_json(multipolygon_to_list(difference), "test_poly.json")


class TestGeometry(unittest.TestCase):
    """
    Tests for geometric calculations
    """

    def __init__(self, *args, **kwargs):

        unittest.TestCase.__init__(self, *args, **kwargs)

        # poly1 has a hole, poly2 does not
        self.poly1, self.poly2 = [polygon_to_shapely(i) for i in load(
            DATA_DIR + "/geometry_test_data.pickle")]
        # both polygons are precincts from arkansas
        # poly1 has geoid 05073008 and poly2 has geoid 05027020

        self.north_dakota = load(DATA_DIR + "/nd_test.pickle")
        self.north_dakota[0] = multipolygon_to_shapely(self.north_dakota[0])
        for i in range(1, 3):
            self.north_dakota[i] = polygon_to_shapely(self.north_dakota[i])
        # precincts:
        # 1 | 3810138-05 | non-contiguous
        # 2 | 3801139-02 | hole
        # 3 | 3800139-01 | normal

        self.vermont = load(DATA_DIR + "/vermont.pickle")

    def test_get_compactness(self):

        @print_time
        def test_get_compactness_speed(polygon):
            return get_schwartzberg_compactness(polygon)

        # later to be tested with cpp outputs
        self.assertEqual(
            test_get_compactness_speed(self.north_dakota[0]),
            0.6140866384724538
        )
        self.assertEqual(
            test_get_compactness_speed(self.north_dakota[1]),
            0.7997295546826071
        )
        self.assertEqual(
            test_get_compactness_speed(self.north_dakota[2]),
            0.8197172000202778
        )

    def test_get_point_in_polygon(self):

        @print_time
        def test_get_point_in_polygon_speed(polygon, point):
            return get_point_in_polygon(polygon, point)

        # point is inside polygon
        self.assertEqual(
            test_get_point_in_polygon_speed(
                    self.poly2,
                    [451937, 3666268]
            ),
            True
        )

        # edge case: point is on edge of polygon
        self.assertEqual(
            test_get_point_in_polygon_speed(
                [[[0, 0], [2, 0], [2, 2], [0, 2]]],
                [1, 2]
            ),
            True
        )


class TestCommunities(unittest.TestCase):
    """
    Tests for community algorithm-specfic functions
    """

    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)

        self.vermont = load(DATA_DIR + "/vermont.pickle")

    def test_get_bordering_precincts(self):
        
        @print_time
        def test_get_bordering_precincts_speed(community, unchosen_precincts):
            return Community.get_bordering_precincts(community,
                       unchosen_precincts)

        big_community = Community(self.vermont[0], 0)
        small_community = Community([], 1)
        # Precinct with id 50005VD42 touches another precinct at one vertex. Checks to make sure that precicnt is not included in bordering precincts
        big_community.give_precinct(small_community, "50005VD42",
            compactness=False)

        self.assertEqual(
            test_get_bordering_precincts_speed(
                small_community,
                big_community
            ),
            {"50005VD56", "50005VD52", "50005VD54", "50023VD184", "50005VD48",
             "50005VD40", "50005VD50"}
        )



if __name__ == "__main__":

    if "border_graph" in sys.argv:
        TestClipping.plot_get_border()
    else:
        clipping = TestClipping
        communities = TestCommunities
        geometry = TestGeometry

        unittest.main()

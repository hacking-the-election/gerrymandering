"""Unit tests for `hacking_the_election.utils.geometry`
"""


import json
import math
import os
import sys
import unittest

from shapely.geometry import MultiPolygon, Point

from hacking_the_election.utils import geometry
from hacking_the_election.visualization.map_visualization import visualize_map


SOURCE_DIR = os.path.dirname(__file__)
SHOW = False


class TestGeometry(unittest.TestCase):

    def setUp(self):
        """Loads data files and saves as instance attributes.
        """

        with open(f"{SOURCE_DIR}/data/geometry/original_gerrymander.json", "r") as f:
            # Stored as GeoJson, not shapely object.
            self.original_gerrymander = \
                json.load(f)["features"][0]["geometry"]["coordinates"]

        with open(f"{SOURCE_DIR}/data/geometry/vermont_precincts.json", "r") as f:
            vermont_precincts_json = json.load(f)

        # Dict linking geoid10 to shapely object containing coordinates.
        self.vermont_precincts = {}
        for precinct in vermont_precincts_json["features"]:
            self.vermont_precincts[precinct["properties"]["GEOID10"]] = \
                geometry.geojson_to_shapely(precinct["geometry"]["coordinates"])

        with open(f"{SOURCE_DIR}/data/geometry/vermont.json", "r") as f:
            # Total state boundary of Vermont.
            self.vermont = geometry.geojson_to_shapely(
                json.load(f)["features"][0]["geometry"]["coordinates"])

    def test_geojson_to_shapely(self):
        """Tests `hacking_the_election.utils.geometry.geojson_to_shapely`
        
        If SHOW is True, then:
        Working under assumption that visualization functions are working.
        Purely for human observation.

        Otherwise simply checks that the function does not throw an error.
        """

        original_gerrymander_polygon = geometry.geojson_to_shapely(self.original_gerrymander)

        if SHOW:
            visualize_map([original_gerrymander_polygon], None, show=True)

    def test_shapely_to_geojson(self):
        """Tests `hacking_the_election.utils.geometry.geojson_to_shapely`

        Checks that the function does not throw an error.
        """

        polygons = [
            Point(0, 0).buffer(50),
            Point(100, 100).buffer(10)
        ]

        geometry.shapely_to_geojson(polygons[0])
        geometry.shapely_to_geojson(MultiPolygon(polygons))

    def test_get_if_bordering(self):
        """Tests `hacking_the_election.utils.geometry.get_if_bordering`

        Tests various precinct borders that were previously not working properly with function.
        All within state of vermont.
        """
        self.assertEqual(
            geometry.get_if_bordering(
                self.vermont_precincts["50009VD85"],
                self.vermont_precincts["50009VD77"]
            ),
            True
        )
        self.assertEqual(
            geometry.get_if_bordering(
                self.vermont_precincts["50003VD37"],
                self.vermont_precincts["50009VD77"]
            ),
            False
        )
        self.assertEqual(
            geometry.get_if_bordering(
                self.vermont,
                self.vermont_precincts["50009VD85"],
                inside=True
            ),
            True
        )

    def test_get_compactness(self):
        """Tests `hacking_the_election.utils.geometry.get_compactness`
        """

        original_gerrymander_polygon = \
            geometry.geojson_to_shapely(self.original_gerrymander)

        original_reock_score = round(geometry.get_compactness(original_gerrymander_polygon), 3)
        self.assertEqual(original_reock_score, 0.32)

        vermont_reock_score = round(geometry.get_compactness(self.vermont), 3)
        self.assertEqual(vermont_reock_score, 0.423)

    def test_area(self):
        """Tests `hacking_the_election.utils.geometry.area`
        """

        precinct = self.vermont_precincts["50009VD85"]

        precinct_area = round(
            geometry.area(geometry.shapely_to_geojson(precinct)[0]), 3)

        self.assertEqual(
            precinct_area,
            round(self.vermont_precincts["50009VD85"].area, 3)
        )

    def test_get_distance(self):
        """Tests `hacking_the_election.utils.geometry.get_distance`
        """
        self.assertEqual(
            geometry.get_distance([0, 0], [4, 4]),
            4 * math.sqrt(2)
        )


if __name__ == "__main__":
    SHOW = "-s" in sys.argv[1:]

    unittest.main()
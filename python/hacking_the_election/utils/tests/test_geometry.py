import json
from os.path import dirname
import time
import unittest

from hacking_the_election.utils import geometry


SOURCE_DIR = dirname(__file__)


class TestGeometry(unittest.TestCase):

    def __init__(self, *args, **kwargs):

        super().__init__(*args, **kwargs)

        with open(f"{SOURCE_DIR}/data/original_gerrymander.json", "r") as f:
            self.original_gerrymander = geometry.geojson_to_shapely(
                json.load(f)["features"][0]["geometry"]["coordinates"])

    def test_get_if_bordering(self):
        pass

    def test_get_compactness(self):
        
        start_time = time.time()
        reock_score = round(geometry.get_compactness(self.original_gerrymander), 3)
        print(f"Reock compactness time: {time.time() - start_time}")
        self.assertEqual(reock_score, 0.32)


if __name__ == "__main__":
    unittest.main()
"""
Useful functions for unit testing
"""


import json
import time


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

    with open(output_file, 'w+') as f:
        json.dump({"type":"FeatureCollection", "features":features}, f)


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
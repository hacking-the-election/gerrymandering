"""
Useful functions for unit testing
"""


import json
import time

from shapely.geometry import Polygon, MultiPolygon


def polygon_to_shapely(polygon):
    """
    Converts list-type polygon `shape` to
    `shapely.geometry.Polygon`
    """
    tuple_polygon = [[tuple(coord) for coord in linear_ring]
                     for linear_ring in polygon]
    return Polygon(tuple_polygon[0], tuple_polygon[1:])


def convert_to_json(coords, output_file, properties=None):
    """
    Writes `coords` to `output_file` as geojson

    Args:
    `coords`: list of features (each a list of coords)
    """
    if properties is None:
        properties = [{} for _ in range(len(coords))]

    features = []
    for i, feature in enumerate(coords):
        features.append({
            "type": "Feature",
            "geometry": {
                "type": ("Polygon" if not isinstance(feature[0][0][0], list)
                         else "MultiPolygon"),
                "coordinates": feature,
            },
            "properties": properties[i]
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


def _tuples_to_lists(lst):
    return [list(coord) for coord in lst]


def polygon_to_list(polygon):
    polygon_list = [_tuples_to_lists(list(polygon.exterior.coords))]
    for ring in list(polygon.interiors):
        polygon_list.append(
            _tuples_to_lists(list(ring.coords)))
    return polygon_list


def multipolygon_to_list(multipolygon):
    # Turns shapely multipolygon into a json one.
    multipolygon_list = []
    for polygon in multipolygon.geoms:
        multipolygon_list.append(polygon_to_list(polygon))
    return multipolygon_list


def multipolygon_to_shapely(multipolygon):
    # Takes in coordinates of a json multipolygon and returns a shapley one.
    polygons = [polygon_to_shapely(p) for p in multipolygon]
    return MultiPolygon(polygons)
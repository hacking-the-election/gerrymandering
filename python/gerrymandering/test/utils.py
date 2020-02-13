"""
Useful functions for unit testing
"""


import json


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
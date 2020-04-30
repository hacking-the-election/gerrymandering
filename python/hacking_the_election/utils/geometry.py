"""
Various useful geometric functions.
"""


import math

import miniball
from shapely.geometry import (
    LinearRing,
    MultiPolygon,
    MultiLineString,
    LineString,
    Polygon
)

def _float_to_int(float_arg):
    """
    Converts float integer for geojson_to_shapely, if int_coords is True.
    """
    return int(float_arg * (2 ** 18))


def geojson_to_shapely(geojson, int_coords=False):
    """Takes shape in geojson format and returns shapely object.

    :param geojson: List of coords in geojson format.
    :type geojson: 3d or 4d list

    :param int_coords: Boolean that signals whether coords in geojson should be converted to integers
    :type int_coords: boolean
    :return: A shapely object containing the same information as `geojson`
    :rtype: `shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`
    """
    # If linear ring
    if isinstance(geojson[0][0], float):
        if int_coords:
            point_list = [(_float_to_int(point[0]), _float_to_int(point[1])) for point in geojson]
        else:
            point_list = [tuple(point) for point in geojson]

        return LinearRing(point_list)
    elif isinstance(geojson[0][0][0], list):
        if int_coords:
            polygons = [[[(_float_to_int(coord[0]), _float_to_int(coord[1])) for coord in linear_ring]
                        for linear_ring in polygon] for polygon in geojson]
        else:
            polygon_coords = [([[tuple(coord) for coord in linear_ring]
                        for linear_ring in polygon]) for polygon in geojson]
            polygons = [Polygon(polygon[0], polygon[1:]) for polygon in polygon_coords]
        return MultiPolygon(polygons)
    elif isinstance(geojson[0][0][0], float):
        if int_coords:
            polygon_list = [[(_float_to_int(coord[0]), _float_to_int(coord[1])) for coord in linear_ring]
                            for linear_ring in geojson]
        else:
            polygon_list = [[tuple(coord) for coord in linear_ring]
                            for linear_ring in geojson]
        return Polygon(polygon_list[0], polygon_list[1:])
    else:
        raise ValueError("invalid geojson")


def shapely_to_geojson(shape, json_format=False):
    """Converts a shapely object into a list of coords using geojson protocol.

    :param shape: A shape that will be converted to geojson.
    :type shape: `shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`

    :param json_format: Whether or not to have the list of coords enclosed in proper geojson dicts.
    :type json_format: bool

    :return: `shape` represented as geojson.
    :rtype: list of list of list of float or list of list of list of list of float.
    """

    geojson = []
    if isinstance(shape, MultiPolygon):
        for polygon in shape.geoms:
            geojson.append(shapely_to_geojson(polygon))

    elif isinstance(shape, Polygon):
        exterior_coords = []
        for coord in list(shape.exterior.coords):
            exterior_coords.append(list(coord))
        geojson.append(exterior_coords)
        for interior in list(shape.interiors):
            interior_coords = []
            for coord in list(interior.coords):
                interior_coords.append(list(coord))
            geojson.append(interior_coords)
    else:
        raise TypeError("shapely_to_geojson only accepts arguments of type "
                        "shapely.geometry.Polygon or shapely.geometry.MultiPolygon")
    if json_format:
        return {"type": "FeatureCollection", "features": [
            {"type": "Feature", "geometry":{
                "type": ("Polygon" if isinstance(shape, Polygon) else "MultiPolygon"),
                "coordinates": geojson
            }}
        ]}
    else:
        return geojson


def get_if_bordering(shape1, shape2, inside=False):
    """Determines whether or not two shapes (shapely polygons) are bordering
    
    :param shape1: One of the shapes being considered.
    :type shape1: `shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`

    :param shape2: The other shape being considered.
    :type shape2: `shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`

    :param inside: Whether or not `shape2` is inside `shape1`. Order matters if this is `True`, defaults to `False`
    :type inside: bool, optional

    :return: Whether or not `shape1` and `shape2` are bordering.
    :rtype: bool
    """
    if inside:
        difference = shape1.difference(shape2)
        try:
            # The below code illustrated:
            # ________                       ________
            # |       |        ____         |____    |
            # |       | minus |____| equals  ____|   |
            # |_______|                     |________|
            # The border of the difference between the
            # difference and the subtrahend is this:
            # ________
            # |       |
            #         |
            # |_______|
            # It is not a closed figure, so they were bordering.

            # You can see that if they were not bordering, the final
            # figure in the above illustration would be a closed figure.

            # The fill `!= (shape1 == shape2)` is an "exclusive or"
            final_difference = LinearRing(difference.exterior.coords).difference(shape2)
            return isinstance(final_difference, MultiLineString) != (shape1 == shape2)
        except AttributeError:
            return False
    else:
        # Doesn't work if one shape is inside the other because it'll
        # always return false because their intersection would be a
        # Polygon, but they may still be bordering.
        intersection = shape1.intersection(shape2)
        return (isinstance(intersection, MultiLineString)
             or isinstance(intersection, LineString))


def get_compactness(district):
    """Calculates the Reock compactness score for a district

    Formula obtained from here: https://fisherzachary.github.io/public/r-output.html

    :param district: District to find compactness of.
    :type district: `shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`

    :return: Schwartzberg compactness of `district`.
    :rtype: float
    """

    district_coords = shapely_to_geojson(district)
    P = []
    if isinstance(district, Polygon):
        for linear_ring in district_coords:
            for point in linear_ring:
                P.append(point)
    elif isinstance(district, MultiPolygon):
        for polygon in district_coords:
            for linear_ring in polygon:
                for point in linear_ring:
                    P.append(point)

    # Move points towards origin for better precision.
    minx = min(P, key=lambda p: p[0])[0]
    miny = min(P, key=lambda p: p[1])[1]

    P = [(p[0] - minx, p[1] - miny) for p in P]

    # Get minimum bounding circle of district.
    mb = miniball.Miniball(P)
    center = mb.center()
    squared_radius = mb.squared_radius()
    circle_area = math.pi * squared_radius
    return district.area / circle_area


def area(ring):
    """
    Calculates the area of a json ring

    :param ring: polygon of which area you wish to find
    :type ring: LinearRing[Point[3,2]

    :return: area of polygon.
    :rtype: float
    """
    left_area = 0
    right_area = 0
    for i, point in enumerate(ring):
        if i == (len(ring) - 1):
            break
        if i == 0:
            left_area += float(point[0]) * float(ring[len(ring) - 1][1])
            right_area += float(point[1]) * float(ring[len(ring) - 1][0])
        else:
            left_area += float(point[0]) * float(ring[i + 1][1])
            right_area += float(point[1]) * float(ring[i + 1][0])

    return abs(left_area - right_area) / 2
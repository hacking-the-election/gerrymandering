"""
Various useful geometric functions.
"""


import math

from shapely.geometry import (
    LinearRing,
    MultiPolygon,
    MultiLineString,
    Polygon
)


def geojson_to_shapely(geojson):
    """Takes shape in geojson format and returns shapely object.

    :param geojson: List of coords in geojson format.
    :type geojson: 3d or 4d list

    :return: A shapely object containing the same information as `geojson`
    :rtype: `shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`
    """

    if isinstance(geojson[0][0][0], list):
        polygons = [[[tuple(coord) for coord in linear_ring]
                         for linear_ring in polygon] for polygon in multipolygon_list]
        return MultiPolygon(polygons)
    elif isinstance(geojson[0][0][0], float):
        polygon_list = [[tuple(coord) for coord in linear_ring]
                         for linear_ring in geojson]
        return Polygon(polygon_list[0], polygon_list[1:])
    else:
        raise ValueError("invalid geojson")


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
            # |____   |        ____         |____    |
            # |____|  | minus |____| equals  ____|   |
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

            # The final `!= (shape1 == shape2)` is an "exclusive or"
            return isinstance(
                LinearRing(difference.exterior.coords).difference(shape2),
                MultiLineString) != (shape1 == shape2)
        except AttributeError:
            return False
    else:
        # Doesn't work if one shape is inside the other because it'll
        # always return false because their intersection would be a
        # Polygon, but they may still be intersecting.
        # return isinstance(clip([shape1, shape2], UNION), Polygon)
        return isinstance(shape1.intersection(shape2), MultiLineString)


def get_compactness(polygon):
    """Calculates the Schwartzberg compactness score for a polygon

    Formula obtained from here: https://fisherzachary.github.io/public/r-output.html

    :param polygon: Polygon to find compactness of.
    :type polygon: `shapely.geometry.Polygon`

    :return: Schwartberg compactness of `polygon`.
    :rtype: float
    """

    area = polygon.area
    circumeference = 2 * math.pi * math.sqrt(area / math.pi)
    perimeter = polygon.coords.length
    return circumeference / perimeter


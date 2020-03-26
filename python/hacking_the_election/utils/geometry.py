"""
Various useful geometric functions.
"""


from shapely.geometry import (
    LinearRing,
    MultiPolygon,
    MultiLineString,
    Polygon
)


def _list_to_polygon(polygon_list):
    """
    Takes a polygon in geojson format and returns
    shapely.geometry.Polygon object.
    """

    # Change coordinates to tuples.
    polygon_list = [[tuple(coord) for coord in linear_ring]
                    for linear_ring in polygon_list]
    return Polygon(polygon_list[0], polygon_list[1:])


def _list_to_multipolygon(multipolygon_list):
    """
    Takes a mulyipolygon in geojson format and returns
    shapely.geometry.MultiPolygon object.
    """

    polygons = [_list_to_polygon(polygon) for polygon in multipolygon_list]
    return MultiPolygon(polygons)


def geojson_to_shapely(geojson):
    """
    Takes either multipolygon or polygon in geojson format and returns
    shapely.geometry.Polygon or shapely.geometry.MultiPolygon object.
    """

    if isinstance(geojson[0][0][0], list):
        return _list_to_multipolygon(geojson)
    elif isinstance(geojson[0][0][0], float):
        return _list_to_polygon(geojson)
    else:
        raise ValueError("invalid geojson")


def get_if_bordering(shape1, shape2, inside=False):
    """
    Returns whether or not two shapes (shapely polygons) are bordering as a bool
    If `inside` is `True`, then the function will assume that `shape2` is
    inside `shape1`. Therefore order or arguments matters.
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
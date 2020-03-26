"""
Various useful geometric functions.
"""


from shapely.geometry import Polygon, MultiPolygon


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
"""
Geometric functions
"""


import math
import json

from shapely.geometry import MultiLineString, Point, Polygon
from shapely.ops import unary_union


UNION = 1
DIFFERENCE = 2
INTERSECTION = 3


def get_distance(p1, p2):
    """
    Finds distance between points `p1` and `p2` as lists of floats
    """
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)


def get_if_bordering(shape1, shape2):
    """
    Returns whether or not two shapes (shapely polygons) are bordering as a bool
    """
    return isinstance(clip([shape1, shape2], INTERSECTION), MultiLineString)


def get_point_in_polygon(polygon, point):
    """
    Returns whether or not point is in polygon
    (with or without holes)

    Polygon can either be list or shapely.geometry.Polygon object
    Point can either be list or shapely.geomtry.Point object

    Returns bool
    """
    if isinstance(polygon, list):
        tuple_polygon = [[tuple(coord) for coord in ring] for ring in polygon]
        shapely_polygon = Polygon(tuple_polygon[0], tuple_polygon[1:])
    elif isinstance(polygon, Polygon):
        shapely_polygon = polygon
    if isinstance(point, list):
        coord = Point(*point)
    elif isinstance(point, Point):
        coord = point
    return shapely_polygon.contains(coord) or shapely_polygon.touches(coord)


def clip(shapes, clip_type):
    """
    Finds external border of a group of shapes

    Args:
    `shapes`: list of shapely polygons
    `clip_type`: either 1 (union) or 2 (difference) or 3 (intersection)

    if `clip_type` is difference, then there should only be 2 shapes in
    `shapes`

    Returns array of vertices
    """
    if clip_type == 1:
        solution = unary_union(shapes)
    else:
        if len(shapes) != 2:
            raise ValueError(
                "Polygon clip of type DIFFERENCE takes exactly 2 input shapes"
                )
        if clip_type == 2:    
            solution = shapes[0].difference(shapes[1])
        elif clip_type == 3:
            solution = shapes[0].intersection(shapes[1])
        else:
            raise ValueError(
                "Invalid clip type. Use utils.UNION, utils.DIFFERENCE, or utils.INTERSECTION")
    return solution


def polygon_to_shapely(polygon):
    """
    Converts list-type polygon `shape` to
    `shapely.geometry.Polygon`
    """
    tuple_polygon = [[tuple(coord) for coord in linear_ring]
                     for linear_ring in polygon]
    return Polygon(tuple_polygon[0], tuple_polygon[1:])


def shapely_to_polygon(polygon):
    """
    Creates json polygon from shapely.geometry.Polygon
    NOTE: Assumes shapely polygon only has exterior and no holes.
    """
    try:
        coordinates = list(polygon.exterior.coords)
    except AttributeError:
        try: 
            x = polygon[0]
        except:
            raise Exception('Incorrect input, not a shapely polygon')
        else:
            raise Exception('Incorrect input, a json polygon was inputted, \
                            switch to shapely')
    linear_ring = []
    for tuple1 in coordinates:
        x = tuple1[0]
        y = tuple1[1]
        point_list = [x, y]
        linear_ring.append(point_list)
    # return polygon
    return [linear_ring]


def get_area_intersection(polygon1, polygon2):
    """
    Finds the area intersecting between the two polygons passed as arguments
    Both polygons should be shapely polygons
    Returns float (area of intersection.)
    """
    # find area of both polygons
    area1 = polygon1.area
    area2 = polygon2.area
    # find area of union
    union_coords = clip([polygon1, polygon2], UNION)
    union_area = union_coords.area
    # The area of the intersection is the sum of the two areas minus the area of the union
    intersect_area = area1 + area2 - union_area
    # if the intersection area is negative, raise exception
    if abs(intersect_area) != intersect_area:
        raise Exception("Negative area found, check inputs")
    return intersect_area

def communities_to_json(communities_list, output_path):
    """
    convert lists of communities of the kind outputed by communities
    into a json file for viewing
    """
    features = []
    for community in communities_list:
        coords = shapely_to_polygon(community.coords)
        features.append({"geometry": {"type":"Polygon", "coordinates":coords}})
    completed_json = {"type":"FeatureCollection", "features":features}
    with open(output_path, 'w') as f:
        json.dump(f)
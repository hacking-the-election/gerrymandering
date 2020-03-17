"""
Functions for partisanship refinement step in communities algorithm
"""
from time import time

from .geometry import (
    clip,
    get_area_intersection,
    get_point_in_polygon,
    polygon_to_shapely,
    shapely_to_polygon,
    get_if_bordering,
    UNION
)

from shapely.geometry import LineString


def get_bordering_precincts(community1, community2):
    """
    Takes two community objects and returns list of precinct objects along either side of 
    the border between them.
    If the two communities share no border, returns []
    """
    # finds coordinates of the two communities lol fu
    coords1 = community1.coords
    coords2 = community2.coords
    area_intersection = clip([coords1, coords2], 3).area
    # if area_intersection is positive, something's wrong
    if area_intersection > 0:
        raise Exception('Communities intersect!')
    border_precincts = {community1.id: [], community2.id: []}
    for precinct in community1.precincts.values():
        if get_if_bordering(precinct.coords, coords2):
            border_precincts[community1.id].append(precinct)
    for precinct1 in community2.precincts.values():
        if get_if_bordering(precinct1.coords, coords1):
            border_precincts[community2.id].append(precinct1)
    # # create combined list of coordinates in each community,
    # combined_coords = shapely_to_polygon(coords1)[0]
    # to_combine = shapely_to_polygon(coords2)[0]
    # for coord in to_combine:
    #     combined_coords.append(coord)
    # # find union of two communities
    # combined_union = clip([coords1, coords2], 1)
    # combined_union = shapely_to_polygon(combined_union)
    # union_points = [combined_union[0][y] for y in range(len(combined_union[0]))]
    # # border_coords will have points in combined_coords
    # # not in combined_union
    # border_coords = []
    # for point in combined_coords:
    #     # point, e.x. -73.142 29.538
    #     if point in union_points:
    #         border_coords.append(point)
    return border_precincts
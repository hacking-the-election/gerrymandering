"""
Functions for partisanship refinement step in communities algorithm
"""


from .geometry import (
    clip,
    get_area_intersection,
    get_point_in_polygon,
    polygon_to_shapely,
    shapely_to_polygon,
    UNION
)

from shapely.geometry import LineString


def get_bordering_precincts(community1, community2):
    """
    Takes two community objects and returns list of precinct objects along either side of 
    the border between them.
    If the two communities share no border, returns []
    """
    # finds coordinates of the two communities
    coords1 = community1.coords
    coords2 = community2.coords
    combined_precincts = {**community1.precincts, **community2.precincts}
    area_intersection = clip([coords1, coords2], 3).area
    # if area_intersection is positive, something's wrong
    if area_intersection > 0:
        raise Exception('Communities intersect!')
    # create combined list of coordinates in each community,
    combined_coords = shapely_to_polygon(coords1)[0]
    to_combine = shapely_to_polygon(coords2)[0]
    for coord in to_combine:
        combined_coords.append(coord)
    # find union of two communities
    combined_union = clip([coords1, coords2], 1)
    combined_union = shapely_to_polygon(combined_union)
    union_points = [combined_union[0][y] for y in range(len(combined_union[0]))]
    # border_coords will have points in combined_coords
    # not in combined_union
    border_coords = []
    for point in combined_coords:
        # point, e.x. -73.142 29.538
        if point in union_points:
            border_coords.append(point)
    if border_coords == []:
        return {community1.id:[], community2.id:[]}
    # check all precincts in either commmunity to see if 
    # they fall along border line 
    # keys: ids of the two communities in question
    # values: list of ids of border precincts in respective communities
    border_precincts = {community1.id : [], community2.id : []}
    for precinct in combined_precincts.values():
        if precinct in border_precincts.values():
            break
        if precinct.coords.touches(LineString([(point[0], point[1]) for point in border_coords])):
            if precinct in community1.precincts.values():
                border_precincts[community1.id].append(precinct)
            else:
                border_precincts[community2.id].append(precinct)
    return border_precincts
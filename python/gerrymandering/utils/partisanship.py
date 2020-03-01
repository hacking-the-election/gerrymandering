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
    area_intersection = get_area_intersection(coords1, coords2)
    # if area_intersection is positive, something's wrong
    if area_intersection > 0:
        raise Exception('Communities intersect!')
    # create combined list of coordinates in each community,
    combined_coords = list(coords1.boundary).extend(list(coords2.boundary))
    # find union of two communities
    combined_union = clip([shapely_to_polygon(coords1)[0], 
                          shapely_to_polygon(coords2)[0]],
                          1)
    # border_coords will have points in combined_coords
    # not in combined_union
    border_coords = []
    for point in combined_coords:
        # point, e.x. -73.142 29.538
        point_list  = str(point).split()
        if point_list in combined_union:
            border_coords.append(point_list)
    if border_coords == []:
        return []
    # check all precincts in either commmunity to see if 
    # they fall along border line 
    # keys: ids of the two communities in question
    # values: list of ids of border precincts in respective communities
    border_precincts = {community1.id : [], community2.id : []}
    for precinct in combined_precincts:
        for point in border_coords:
            # if precinct is already in border_precincts, there's
            # no need to check it again
            if precinct.vote_id in border_precincts:
                break
            if get_point_in_polygon(precinct.coords, point):
                if get_point_in_polygon(polygon_to_shapely(coords1.boundary), precinct.coords[0][0]):
                    border_precincts[community1.id].append(precinct.vote_id)
                else:
                    border_precincts[community2.id].append(precinct.vote_id)
    return border_precincts
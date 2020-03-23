"""
Functions specific to initial configuration step in communities algorithm
"""


import random
import sys
from shapely.geometry import MultiPolygon, Polygon

from hacking_the_election.utils.exceptions import (
    CommunityFillCompleteException,
    CreatesMultiPolygonException,
    ZeroPrecinctCommunityException
)
from hacking_the_election.utils.geometry import (
    clip,
    DIFFERENCE,
    get_distance,
    get_if_bordering,
    get_point_in_polygon,
    UNION
)


GIVE_PRECINCT_COORDS_ONLY_KWARGS = {
    "partisanship": False,
    "standard_deviation": False,
    "population": False, 
    "compactness": False, 
    "coords": True
}


def fill(self, precincts, linked_precincts, island_index, 
         island_border, community_borders, used_starting_precincts=set()):
    """
    Fills a community up with precincts.

    Args:
        `precincts`:               List of Precinct objects in the
                                    island the community is on.
        `linked_precincts`:        Set of precincts that are meant
                                    to be part of communities that
                                    span islands, therefore making
                                    them untouchable during this step.
        `island_index`    :        Index of island that corresponds
                                    to a key in `self.islands`.
        `island_border`   :        Border of island with `island_index`
        `community_borders`:       List of borders of all
                                    communities on island.
        `used_starting_precincts`: Set of precinct ids that have
                                    already been tried for this
                                    community and led to the
                                    inevitable creation of an island.

    Returns list of added precincts and Polygon that is outer border
    of group of precincts on island that have not been added to a
    community.
    """

    added_precincts = set()
    unchosen_precincts = self.create_instance(
        precincts[:],  # copy
        0,
        {},
        coords=Polygon(island_border)  # copy
    )

    # Add first precinct to community.

    # All the precincts are "bordering" because this community has
    # no precincts on this island yet.
    all_bordering_precincts = {p.vote_id for p in precincts}
    island_bordering_precincts = {
        p.vote_id for p in unchosen_precincts.precincts.values()
        if get_if_bordering(island_border, p.coords, True)
    }
    # Community objects to store the coords.
    communities = [self.create_instance([], i, {}, coords=border)
                    for i, border in enumerate(community_borders)]
    community_bordering_precincts = set()
    for c in communities:
        for p in c.get_bordering_precincts(unchosen_precincts):
            community_bordering_precincts.add(p)
    # precincts that border a community and the island border
    eligible_precincts = (
        island_bordering_precincts
        & community_bordering_precincts
        & all_bordering_precincts)
    if eligible_precincts == set():
        eligible_precincts = \
            island_bordering_precincts & all_bordering_precincts
    if eligible_precincts == set():
        eligible_precincts = all_bordering_precincts
    initial_precinct = random.sample(
        eligible_precincts - used_starting_precincts, 1)[0]
    added_precincts.add(initial_precinct)
    unchosen_precincts.give_precinct(self, initial_precinct,
                                     **GIVE_PRECINCT_COORDS_ONLY_KWARGS,
                                     allow_zero_precincts=True,
                                     allow_multipolygons=True)
    sys.stdout.write(f"\r{add_leading_zeroes(len(self.precincts))} precincts in community {self.id}")
    sys.stdout.flush()

    while True:
        
        bordering_precincts = \
            self.get_bordering_precincts(unchosen_precincts)
        n_added_precincts = 0
        now_added_precincts = set()
        for precinct in bordering_precincts - linked_precincts:
            if len(self.precincts) >= self.islands[island_index]:
                # Use this instead of returning to break from all
                # levels of recursion.

                # Move to new line so later print statements don't
                # write over current line.
                print()
                del self.islands[island_index]
                raise CommunityFillCompleteException(
                    list(unchosen_precincts.precincts.values()),
                    unchosen_precincts.coords)
            else:
                unchosen_precincts.give_precinct(
                    self, precinct, **GIVE_PRECINCT_COORDS_ONLY_KWARGS,
                    allow_zero_precincts=True,
                    allow_multipolygons=True)
                if (
                        isinstance(unchosen_precincts.coords, MultiPolygon)
                     or isinstance(self.coords, MultiPolygon)
                        ):
                    self.give_precinct(
                        unchosen_precincts,
                        precinct,
                        **GIVE_PRECINCT_COORDS_ONLY_KWARGS,
                        allow_zero_precincts=True,
                        allow_multipolygons=True
                    )
                else:
                    now_added_precincts.add(precinct)
                    added_precincts.add(precinct)
                    n_added_precincts += 1
                    sys.stdout.write(f"\r{add_leading_zeroes(len(self.precincts))}")
                    sys.stdout.flush()
        if n_added_precincts == 0:
            # No precincts can be added without making an island.
            print(f"\nrestarting filling of community {self.id}")
            # Remove all precincts added during this fill session.
            self.precincts = {
                p: self.precincts[p] for p in
                set(self.precincts.keys()) - added_precincts
            }
            self.fill(precincts, linked_precincts, island_index,
                      island_border, community_borders,
                      used_starting_precincts | {initial_precinct})


def group_by_islands(precincts):
    """
    Returns list of precincts sorted into
    lists by what island they are on.
    """
    state_border = clip([p.coords for p in precincts], UNION)

    if isinstance(state_border, MultiPolygon):
        # There are islands in the state.

        island_polygons = state_border.geoms
        islands = [[] for _ in range(len(island_polygons))]

        for precinct in precincts:
            for i, island in enumerate(island_polygons):
                # If the first point of the precinct is in this island,
                # all of them must be.
                if get_point_in_polygon(
                        island, list(precinct.coords.exterior.coords[1])):
                    islands[i].append(precinct)
        return islands
    else:
        return [[p for p in precincts]]


def get_closest_precinct_on_island(island_centroid,
                                   other_island_border_precincts,
                                   other_island_precincts,
                                   other_island_border):
    """
    Finds precinct on `other_island` that is closest to `island`

    Args:
        `island_centroid`:               Centroid of island.
        `other_island_border_precincts`: Precincts on border of other
                                         island.
        `other_island_precincts`:        Precincts in other island.
        `other_island_border`:           Polygon that is outside border
                                         of other island

    Returns `save_precincts.Precinct` object.
    """
    closest_precinct = None
    closest_precinct_distance = 0
    for p in other_island_border_precincts:
        distance = get_distance(island_centroid,
                                p.coords.centroid.coords[0])
        if (
                closest_precinct is None
                or distance < closest_precinct_distance):
            # If removing this precinct from the island
            # creates an isolated section within the island.
            if isinstance(clip([other_island_border, p.coords], DIFFERENCE),
                          MultiPolygon):
                continue
            else:
                closest_precinct = p
                closest_precinct_distance = distance
    
    if closest_precinct is None:
        for p in other_island_precincts:
            if p not in other_island_border_precincts:
                distance = get_distance(island_centroid,
                                        p.coords.centroid.coords[0])
                if (
                        closest_precinct is None
                        or distance < closest_precinct_distance):
                    # If removing this precinct from the island
                    # creates an isolated section within the island.
                    if isinstance(clip([other_island_border, p.coords], DIFFERENCE),
                                MultiPolygon):
                        continue
                    else:
                        closest_precinct = p
                        closest_precinct_distance = distance

    return closest_precinct, closest_precinct_distance


def get_closest_precinct(island, island_precinct_groups,
                         island_borders, island_border,
                         state_island_borders):
    """
    Finds precinct on border of any island in `island_precinct_groups`
    that is closest to `island`. Meant to be used as second precinct in
    a "linked pair" of precincts.
    
    Args:
        `island`:                 List of `save_precincts.Precinct`
                                  objects that is create the island
                                  you want to find the precinct that
                                  is closest to.
        `island_precinct_groups`: List of lists of
                                  `save_precincts.Precinct` objects
                                  grouped by islands to search for
                                  close precincts in.
        `island_borders`:         List of Polygons that are borders of
                                  all the islands in the state.
        `island_border`:          Polygon that is the border of
                                  `island`.
        `state_island_borders`:   Border of every island in state (so
                                  that the indices of islands can be
                                  found.)

    Returns vote_id attribute of closest precinct and index of island
    that contains that precinct.
    """
    # List of precincts that border the "coastline" of the each island.
    # Grouped by island.
    border_precincts = \
        [[precinct for precinct in il
          if get_if_bordering(i_border, precinct.coords, True)]
         for i_border, il in zip(island_borders, island_precinct_groups)]
    island_centroid = island_border.centroid.coords[0]
    
    closest_precinct = None
    closest_precinct_distance = 0
    closest_precinct_island_index = 0

    for border, island_border_precincts, precinct_group in zip(
            island_borders, island_precinct_groups, border_precincts):
        closest_precinct_on_island, distance = \
            get_closest_precinct_on_island(
                island_centroid, island_border_precincts,
                precinct_group, island_border)
        if (
                closest_precinct is None
                or closest_precinct_distance > distance):
            if closest_precinct_on_island is not None:
                closest_precinct_distance = distance
                closest_precinct = closest_precinct_on_island
                closest_precinct_island_index = state_island_borders.index(border)

    return closest_precinct, closest_precinct_island_index


def get_precinct_link_pair(island, island_precinct_groups,
                           island_border, island_borders,
                           state_island_borders):
    """
    Finds pair of two closest precincts on borders of `island` and any
    island in `island_precinct_groups`.

    Args:
        `island`:                 List of `save_precincts.Precinct`
                                  objects that is create the island
                                  you want to find the precinct that
                                  is closest to.
        `island_precinct_groups`: List of lists of
                                  `save_precincts.Precinct` objects
                                  grouped by islands to search for
                                  close precincts in.
        `island_border`:          Polygon that is the border of `island`
        `island_borders`:         List of Polygons that are borders of
                                  all the islands in the state.
        `state_island_borders`:   Border of every island in state (so
                                  that the indices of islands can be
                                  found.)

    Returns two strings (vote_ids of precinct on `island` and other
    precinct) and one integer (index of the island the other precinct
    is on).
    """

    precinct1, closest_precinct_island_index = get_closest_precinct(
        island, island_precinct_groups, island_borders,
        island_border, state_island_borders)
    
    island_border_precincts = \
        [p for p in island if get_if_bordering(island_border, p.coords, True)]

    precinct2, _ = get_closest_precinct_on_island(
            state_island_borders[
                closest_precinct_island_index].centroid.coords[0],
            island_border_precincts,
            island_precinct_groups[closest_precinct_island_index],
            island_border
        )

    return precinct1, precinct2, closest_precinct_island_index


def add_leading_zeroes(n):
    """
    Takes an integer and returns a string with 3 characters.
    Adds leading zeroes if integer has less than 3 digits.
    """
    n_chars = list(str(n))
    while len(n_chars) != 3:
        n_chars.insert(0, "0")
    return "".join(n_chars)
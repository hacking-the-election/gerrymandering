"""
Algorithm implementation for breaking a state into political communities
"""

import pickle
import random
from os.path import abspath, dirname
import sys

from shapely.geometry import Polygon, MultiPolygon

sys.path.append(abspath(dirname(dirname(__file__))))

from gerrymandering.utils import (Community, group_by_islands, clip, UNION,
                                  LoopBreakException, get_precinct_link_pair,
                                  get_closest_precinct, average)
from .test.utils import convert_to_json


import logging

logging.basicConfig(filename="precincts.log", level=logging.DEBUG)


def create_initial_configuration(island_precinct_groups, n_districts,
                                 state_border):
    """
    Returns a list of communities that consist of random groups of communities.
    """

    precincts = [precinct for island in island_precinct_groups
                 for precinct in island]

    # Calculate `community_sizes`, a list of numbers corresponding to
    # the number of precincts each community should have.
    n_precincts = len(precincts)
    community_sizes = [n_precincts // n_districts for _ in range(n_districts)]
    for i in range(n_districts % n_precincts):
        community_sizes[i] += 1

    print(community_sizes)

    # Find which islands are capable of containing a whole number of
    # communities to start with.
    unchosen_precincts = Community(precincts[:], 0, {}, state_border)
    communities = []  # Will contain Community objects.
    # State is multipolygon, which means it has islands.

    # There should be communities of two sizes
    small_community = min(community_sizes)
    large_community = max(community_sizes)
    # Should eventually all become zero
    island_available_precincts = \
        [len(island) for island in island_precinct_groups]
    print(f"number of precincts on each island: {island_available_precincts}")

    island_borders = [clip([p.coords for p in il], UNION)
                      for il in island_precinct_groups]

    try:
        # Add communities with only
        # id, island, and size attributes filled.
        # Also fill list below with links between
        # islands with fractional number of communities.
        linked_precinct_chains = []
        for i, island in enumerate(island_precinct_groups):
            try:
                # Each element is list of x, y, and difference with number
                # of available precincts.
                community_grouping_attempts = []
                for x in range(community_sizes.count(small_community)):
                    for y in range(community_sizes.count(large_community)):
                        if ((n_precincts := \
                            x * small_community + y * large_community)
                                == island_available_precincts[i]):
                            # Remove x small communities and y large ones
                            # because those many precincts were used in
                            # making this community.
                            for _ in range(x):
                                community_sizes.remove(small_community)
                                communities.append(
                                    Community([], len(communities) + 1,
                                            {i: large_community}))
                            for _ in range(y):
                                community_sizes.remove(large_community)
                                communities.append(
                                    Community([], len(communities) + 1,
                                            {i: large_community}))

                            # All the precincts are used in the island,
                            # so none are available anymore.
                            island_available_precincts[i] = 0
                            print(f"perfect configuration found for island {i} "
                                  f"which started with {len(island)} precincts "
                                  f"and had {island_available_precincts[i]} until "
                                   "now.")
                            raise LoopBreakException
                        elif (n_precincts + large_community
                            > island_available_precincts[i]):
                            # This is as close as we can get without going
                            # over available precincts (for this value of `x`)
                            community_grouping_attempts.append(
                                [x, y, island_available_precincts[i]
                                        - n_precincts]
                            )
                            break

                # No configuration works out
                print(f"island {i} has to be linked.")

                # Find configuration closest to available precincts.
                best_configuration = min(community_grouping_attempts,
                                        key=lambda x: x[-1])
                for _ in range(best_configuration[0]):
                    community_sizes.remove(small_community)
                    communities.append(
                        Community([], len(communities) + 1,
                                {i: small_community}))
                for _ in range(best_configuration[1]):
                    community_sizes.remove(large_community)
                    communities.append(
                        Community([], len(communities) + 1,
                                {i: large_community}))

                # Number of precincts that have been added through
                # linking to other islands. Eventually will become
                # equal to `large_community` or `small_community`
                n_extra_precincts = n_precincts
                # dict with keys of island index and value of number of
                # precincts used in community
                islands_used = {i: n_precincts}
                # Loop through islands with some precincts left
                eligible_islands = \
                    [[il, n_il_precincts, island_precinct_groups.index(il)]
                    for il, n_il_precincts in zip(island_precinct_groups,
                        island_available_precincts)
                    if n_il_precincts != 0 and il != island]

                last_island_used = i

                while n_extra_precincts < community_sizes[0]:
                    # Islands that can be linked to:
                    # All islands with a fractional number of communities
                    # other than current island and islands we have already
                    # used.

                    # Add linked pair
                    eligible_island_precinct_groups = [il[0] for il in
                                                       eligible_islands]
                    eligible_island_borders = \
                        [island_borders[il[2]] for il in eligible_islands]
                    if len(islands_used) == 1:
                        # It's the first link
                        island_border = island_borders[i]
                        precinct1, precinct2, new_island_index = \
                            get_precinct_link_pair(
                                island,
                                eligible_island_precinct_groups,
                                island_border,
                                eligible_island_borders,
                                island_borders[:]
                            )
                        linked_precinct_chains.append([precinct1, precinct2])
                    else:
                        # Linking from last island in link chain.
                        precinct2, new_island_index = \
                            get_closest_precinct(
                                island_precinct_groups[last_island_used],
                                eligible_island_precinct_groups,
                                eligible_island_borders,
                                island_borders[last_island_used],
                                island_borders[:]
                            )
                        linked_precinct_chains[-1].append(precinct2)

                    print(f"Precinct added to link chain for island {i} "
                          f"between island {last_island_used} "
                          f"and island {new_island_index}")

                    # Update information
                    last_island_used = new_island_index
                    n_extra_precincts += island_available_precincts[
                        new_island_index]
                    islands_used[new_island_index] = island_available_precincts[
                        new_island_index]
                    island_available_precincts[new_island_index] = 0

                    for j, eligible_island in enumerate(eligible_islands[:]):
                        if eligible_island[-1] == last_island_used:
                            eligible_islands.pop(j)

                # Add precincts back to last island to link to.
                island_available_precincts[last_island_used] += \
                    n_extra_precincts - community_sizes[0]
                islands_used[last_island_used] -= \
                    n_extra_precincts - community_sizes[0]
                island_available_precincts[i] = 0
                communities.append(Community([], len(communities) + 1,
                                islands_used))
                print("linking required to create community from leftover "
                     f"precincts on island {i} complete.")
                print(f"{islands_used=}")
            except LoopBreakException:
                pass
    except Exception as e:
        # Save as much information about what happened in the program as possible.
        with open("test_communities_debug.pickle", "wb+") as f:
            # ADD MORE TO THIS LIST AS YOU DEBUG
            pickle.dump(
                [
                    island_available_precincts,
                    island_precinct_groups,
                    communities,
                    linked_precinct_chains
                ],
                f)
        raise e

    print([[precinct.vote_id for precinct in chain]
          for chain in linked_precinct_chains])
    print(island_available_precincts)

    all_linked_precincts = set(
        [p for pair in linked_precinct_chains for p in pair])

    try:
        # Add precincts to each community that has any area on each island.
        for i, island in enumerate(island_precinct_groups):
            for community in communities:
                if i in community.islands:
                    community.fill(island, all_linked_precincts, i)

    except Exception as e:
        # Save your progress!
        logging.info(unchosen_precincts.precincts)
        with open("test_communities.pickle", "wb+") as f:
            pickle.dump(communities, f)
        raise e

    return communities
    
def modify_for_partisanship(communities_list, threshold):
    '''
    Takes list of community objects, and returns a different list with the modified communities
    '''
    communities_coords = {community.id : community.precincts for community in communities_list}
    # update partisanship values (in case this hasn't already been done)
    for community in communities_list.values():
        community.update_standard_deviation()
    # create dictionary of ids and community partisanship standard deviations
    community_stdev = {community.id : community.standard_deviation for community in communities_list}
    # check if any communities are above the threshold
    # count the number of times the list has been checked
    count = 0
    num_of_above_precincts = 0
    average_stdev = average(community_stdev.values())
    for id, community in community_stdev.items():
        # if a community is above the threshold
        if community > threshold:
            most_stdev = {}
            for id1, community1 in community_stdev.items():
                if community > most_stdev.get(id, 0):
                    most_stdev[id] = community
            biggest_precincts = communities_coords[most_stdev.keys()[0]]
            group_by_islands(biggest_precincts)

def make_communities(state_file):
    """
    `state_file` - path to state that is to be divided into communities
    """

    with open(state_file, 'rb') as f:
        state_data = pickle.load(f)
    precincts = state_data[0]
    districts = state_data[1]
    state_border = state_data[2]

    # Step 1
    communities = create_initial_configuration(
        precincts, len(districts), state_border)

    return communities
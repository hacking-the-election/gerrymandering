"""
Algorithm implementation for breaking a state into political communities
"""

import pickle
import random
from os.path import abspath, dirname
import sys

from shapely.geometry import Polygon, MultiPolygon

sys.path.append(abspath(dirname(dirname(__file__))))

from gerrymandering.utils import (Community, group_by_islands,
                                  LoopBreakException, get_precinct_link_pair)
from .test.utils import convert_to_json


import logging

logging.basicConfig(filename="precincts.log", level=logging.DEBUG)


def create_initial_configuration(island_precinct_groups, n_districts):
    """
    Takes `precincts` as a list of precincts and returns a list of
    communities that consist of random groups of communities.
    """

    precincts = [precinct for island in island_precinct_groups
                 for precinct in island]

    # Calculate `community_sizes`, a list of numbers corresponding to
    # the number of precincts each community should have.
    n_precincts = len(precincts)
    community_sizes = [n_precincts // n_districts for _ in range(n_districts)]
    for i in range(n_districts % n_precincts):
        community_sizes[i] += 1

    # Find which islands are capable of containing a whole number of
    # communities to start with.
    unchosen_precincts = Community(precincts[:], 0)
    communities = []  # Will contain Community objects.
    # State is multipolygon, which means it has islands.

    # There should be communities of two sizes
    small_community = min(community_sizes)
    large_community = max(community_sizes)
    island_precinct_groups = group_by_islands(precincts)
    # Should eventually all become zero
    island_available_precincts = \
        [len(island) for island in island_precinct_groups]

    # Add communities with only
    # id, island, and size attributes filled.
    # Also fill list below with links between
    # islands with fractional number of communities.
    linked_precinct_pairs = []
    for i, island in enumerate(island_precinct_groups):
        # Each element is list of x, y, and difference with number
        # of precincts.
        try:
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
                                Community([], len(communities),
                                          {i: large_community}))
                        for _ in range(y):
                            community_sizes.remove(large_community)
                            communities.append(
                                Community([], len(communities),
                                          {i: large_community}))

                        # All the precincts are used in the island,
                        # so none are available anymore.
                        island_available_precincts[i] = 0
                        raise LoopBreakException
                    elif n_precincts > island_available_precincts[i]:
                        # Any attempts with a higher y will also
                        # yield too many communities, so stop now.
                        break
                    else:
                        community_grouping_attempts.append(
                            [x, y, island_available_precincts
                                    - n_precincts]
                        )
            # No configuration works out
            best_configuration = min(community_grouping_attempts,
                                        key=lambda x: x[-1])
            for _ in range(best_configuration[0]):
                community_sizes.remove(small_community)
                communities.append(
                    Community([], len(communities),
                              {i: small_community}))
            for _ in range(best_configuration[1]):
                community_sizes.remove(large_community)
                communities.append(
                    Community([], len(communities),
                              {i: large_community}))

            # Subtract number of precincts taken up by communities
            # on island from available precincts.
            island_available_precincts[i] -= n_precincts

            # Number of precincts that have been added through
            # linking to other islands. Eventually will become
            # equal to `large_community` or `small_community`
            n_extra_precincts = 0
            # dict with keys of island index and value of number of
            # precincts used in community
            islands_used = {}
            # Loop through islands with some precincts left
            islands_with_precincts = \
                [[il, n_il_precincts] for il, n_il_precincts in
                    zip(island_precinct_groups, island_available_precincts)
                    if n_il_precincts != 0 and il != island]
            for other_island, n_other_island_precincts in \
                    islands_with_precincts:

                precinct_link_pair = get_precinct_link_pair(island,
                                                            other_island)
                linked_precinct_pairs.append(precinct_link_pair)
                other_island_index = islands_with_precincts.index(
                    [other_island, n_other_island_precincts])
                if (n_extra_precincts + n_other_island_precincts
                        >= community_sizes[0]):
                    precincts_added = (
                        (community_sizes[0] - n_extra_precincts)
                        + n_other_island_precincts
                    )
                    island_available_precincts[other_island_index] \
                        -= precincts_added
                    islands_used[other_island_index] = precincts_added
                    break
                else:
                    n_extra_precincts += n_other_island_precincts
                    islands_used[other_island_index] = \
                        n_other_island_precincts

            communities.append(Community([], len(communities),
                               islands_used))
        except LoopBreakException:
            pass

    all_linked_precincts = set(
        [p for pair in linked_precinct_pairs for p in pair])

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

    communities.append(unchosen_precincts)

    return communities
    


def make_communities(state_file):
    """
    `state_file` - path to state that is to be divided into communities
    """

    with open(state_file, 'rb') as f:
        state_data = pickle.load(f)
    precincts = state_data[0]
    districts = state_data[1]

    # Step 1
    communities = create_initial_configuration(precincts, len(districts))

    return communities
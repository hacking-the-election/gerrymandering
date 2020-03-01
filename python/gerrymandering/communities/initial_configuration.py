"""
First step of iterative method (and therefore our communities algorithm):
Random initial guess.

Groups state into random communities that each have the same number of
precincts.
"""

import logging
from os.path import dirname
import pickle
import random
import sys

sys.path.append(dirname(dirname(__file__)))

from shapely.geometry import MultiPolygon, Polygon

from utils.geometry import (
    clip,
    get_if_bordering,
    polygon_to_shapely,
    shapely_to_polygon,
    UNION
)
from utils.initial_configuration import (
    Community,
    get_closest_precinct,
    get_precinct_link_pair,
    group_by_islands,
    LoopBreakException
)


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
    for i in range(n_precincts % n_districts):
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
    print(f"{island_available_precincts=}")

    # island_borders = [clip([p.coords for p in il], UNION)
    #                   for il in island_precinct_groups]
    with open("../data/test/python/alaska_island_borders.pickle", "rb") as f:
        island_borders = pickle.load(f)

    # vote_ids of precincts in chains
    linked_precinct_chains = []

    try:
        for i, island in enumerate(island_precinct_groups):
            try:
                # Each element is list of x, y, and difference with number
                # of available precincts.
                community_grouping_attempts = []
                for x in range(community_sizes.count(small_community) + 1):
                    for y in range(community_sizes.count(large_community) + 1):

                        if ((island_n_precincts := \
                            x * small_community + y * large_community)
                                > island_available_precincts[i]):
                            break

                        if island_n_precincts == island_available_precincts[i]:
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
                                  f"which started with {len(island)} precincts ")
                            raise LoopBreakException
                        elif (island_n_precincts + large_community
                              > island_available_precincts[i]):
                            # This is as close as we can get without going
                            # over available precincts (for this value of `x`)
                            community_grouping_attempts.append(
                                [x, y, island_available_precincts[i]
                                        - island_n_precincts]
                            )
                            break

                # No configuration works out

                # Find configuration closest to available precincts.
                try:
                    best_configuration = min(community_grouping_attempts,
                                            key=lambda x: x[-1])
                except ValueError as ve:
                    print(community_sizes)
                    raise ve

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

                island_available_precincts[i] -= \
                    (best_configuration[0] * small_community \
                   + best_configuration[1] * large_community)
                
            except LoopBreakException:
                pass

        print(f"{island_available_precincts=}")

        for i, available_precincts in enumerate(island_available_precincts):
            if available_precincts == 0:
                continue

            print(f"start chain from island {i}")

            link_community = Community(
                [], len(communities) + 1,
                {i: available_precincts})
            island_available_precincts[i] = 0
            # All islands with extra precincts excluding current island.
            eligible_islands = \
                [j for j, il in enumerate(island_available_precincts)
                 if il != 0]
            linked_precinct_chains.append([])
            last_island_used = i

            while sum(link_community.islands.values()) < community_sizes[0]:
                
                if linked_precinct_chains[-1] == []:
                    # First link in chain
                    precinct1, precinct2, new_island = \
                        get_precinct_link_pair(
                            island_precinct_groups[i],
                            [
                                island_precinct_groups[j]
                                for j in eligible_islands
                            ],
                            island_borders[i],
                            [
                                island_borders[j]
                                for j in eligible_islands
                            ],
                            island_borders[:]
                        )
                    linked_precinct_chains[-1].append(precinct1)
                else:
                    precinct2, new_island = \
                        get_closest_precinct(
                            island_precinct_groups[last_island_used],
                            [
                                island_precinct_groups[j]
                                for j in eligible_islands
                            ],
                            [
                                island_borders[j]
                                for j in eligible_islands
                            ],
                            island_borders[last_island_used],
                            island_borders[:]
                        )

                print(f"island {last_island_used} linked to island {new_island}")
                print(f"got {island_available_precincts[new_island]} more precincts.")
                print(f"number of precincts in community so far is {sum(link_community.islands.values())}")
                linked_precinct_chains[-1].append(precinct2)
                last_island_used = new_island
                link_community.islands[new_island] = \
                    island_available_precincts[new_island]
                island_available_precincts[new_island] = 0
                try:
                    eligible_islands.remove(new_island)
                except ValueError as  ve:
                    print(eligible_islands)
                    raise ve

            extra_precincts_added = \
                sum(link_community.islands.values()) - community_sizes[0]
            island_available_precincts[last_island_used] = extra_precincts_added
            link_community.islands[last_island_used] -= extra_precincts_added
            print(f"removed {extra_precincts_added} precincts from island {last_island_used}.")
            community_sizes.pop(0)
            print(f"chain completed using islands {link_community.islands}. "
                  f"{sum(link_community.islands.values())} precincts in "
                   "multi-island community")

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
    
    print(linked_precinct_chains)
    print([[p.vote_id for p in chain] for chain in linked_precinct_chains])
    print(island_available_precincts)

    all_linked_precincts = set(
        [p for pair in linked_precinct_chains for p in pair])

    try:
        # First fill communities with links with the rest of their precincts
        for chain in linked_precinct_chains:
            first_chain_island = [i for i, il in enumerate(island_precinct_groups)
                                  if chain[0] in il][0]
            for community in communities:
                # Community spans multiple islands and one of them is
                # `first_chain_island`
                if (
                        len(community.islands) > 1
                        and first_chain_island in list(community.islands.values())
                        ):
                    for island_index in community.islands.keys():
                        added_precincts = \
                            community.fill(island_precinct_groups[island_index],
                                           all_linked_precincts, island_index)

                        # Remove these precincts from being listed as in
                        # island. When other communities fill using this
                        # island, they should not have the precincts
                        # added to this community available to them.
                        for precinct in island_precinct_groups[island_index][:]:
                            if precinct.vote_id in added_precincts:
                                island_precinct_groups[island_index].remove(
                                    precinct)
                        print(f"community {community.id} has been given "
                              f"{community.islands[island_index]} precincts "
                              f"from island {island_index}")

        # Then fill communities that are on only one island
        for community in communities:
            if len(community.precincts) == 0:
                island_index = list(community.islands.keys())[0]
                added_precincts = community.fill(
                    island_precinct_groups[island_index],
                    all_linked_precincts, island_index
                )
                for precinct in island_precinct_groups[island_index][:]:
                    if precinct.vote_id in added_precincts:
                        island_precinct_groups[island_index].remove(
                            precinct)
                print(f"community {community.id} completely filled")

    except Exception as e:
        # Save your progress!
        logging.info(unchosen_precincts.precincts)
        with open("test_communities.pickle", "wb+") as f:
            pickle.dump([communities, linked_precinct_chains], f)
        raise e

    return communities
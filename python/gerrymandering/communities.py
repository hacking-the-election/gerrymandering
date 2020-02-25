"""
Algorithm implementation for breaking a state into political communities
"""

import pickle
import random
from os.path import abspath, dirname
import sys

from shapely.geometry import Polygon, MultiPolygon

sys.path.append(abspath(dirname(dirname(__file__))))

from gerrymandering.utils import *
from .test.utils import convert_to_json


import logging

logging.basicConfig(filename="precincts.log", level=logging.DEBUG)


def create_initial_configuration(precincts, n_districts):
    """
    Takes `precincts` as a list of precincts and returns a list of
    communities that consist of random groups of communities.
    """

    kwargs = {
        "partisanship": False,
        "standard_deviation": False,
        "population": False, 
        "compactness": False, 
        "coords": True
    }

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
    if isinstance(unchosen_precincts.coords, MultiPolygon):
        # State is multipolygon, which means it has islands.

        # There should be communities of two sizes
        small_community = min(community_sizes)
        large_community = max(community_sizes)
        
        island_precinct_groups = group_by_islands(precincts)
        # Dictionary mapping island index in above list to number of
        # communities it can hold.
        island_community_counts = {}
        for i, island in enumerate(island_precinct_groups):
            for x in range(community_sizes.count(small_community)):
                for y in range(community_sizes.count(large_community)):
                    if (x * small_community + y * large_community
                            == len(island)):
                        island_community_counts[i] = x + y

        linked_precinct_pairs = []
        for island, n_communities in island_precinct_groups.items():
            # TODO: fill out above list
            pass
        all_linked_precincts = set([precinct for pair in linked_precinct_pairs
                                    for precinct in pair])
    else:
        # Create all communities except last
        # (which will be unchosen precincts)
        for i, community_size in enumerate(community_sizes[:-1], 1):
            community = Community([], i)
            for _ in range(community_size):
                try:
                    # Set of precincts that have been tried
                    tried_precincts = set()

                    # Give random precinct to `community`
                    random_precinct = random.sample(
                        community.get_bordering_precincts(unchosen_precincts) \
                        - tried_precincts, 1)[0]

                    unchosen_precincts.give_precinct(
                        community, random_precinct, **kwargs)
                    tried_precincts.add(random_precinct)

                    # Keep trying other precincts until one of them
                    # doesn't make an island.
                    while isinstance(unchosen_precincts.coords, MultiPolygon):
                        # Give it back
                        community.give_precinct(
                            unchosen_precincts, random_precinct, **kwargs)
                        print(f"precinct {random_precinct} added to and removed"
                              f" from community {i} because it created an island")
                        # Random precinct that hasn't already been
                        # tried and also borders community.
                        random_precinct = random.sample(
                            community.get_bordering_precincts(unchosen_precincts) \
                            - tried_precincts, 1)[0]
                        unchosen_precincts.give_precinct(
                            community, random_precinct, **kwargs)
                        tried_precincts.add(random_precinct)

                    print(f"precinct {random_precinct} added to community {i}")

                except Exception as e:
                    # Save your progress!
                    logging.info(unchosen_precincts.precincts)
                    with open("test_communities.pickle", "wb+") as f:
                        pickle.dump(communities + [community], f)
                    raise e

            communities.append(community)

    communities.append(unchosen_precincts)

    return communities
    


def make_communities(state_file):
    """
    `state_file` - path to state that is to be divided into communities
    """

    with open(state_file, 'rb') as f:
        state_data = pickle.load(f)
    precincts = state_data[0]

    # Step 1
    communities = create_initial_configuration(precincts, len(districts))

    return communities
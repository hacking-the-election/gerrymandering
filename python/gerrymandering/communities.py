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
        pass
    else:
        for i, community_size in enumerate(community_sizes, 1):
            community = Community([], i)
            for s in range(community_size):
                try:
                    # Set of precincts that have been tried
                    random_precincts = set()

                    # Give random precinct to `community`
                    random_precinct = random.sample(
                        community.get_bordering_precincts(unchosen_precincts) \
                        - random_precincts, 1)[0]

                    unchosen_precincts.give_precinct(
                        community, random_precinct, **kwargs)
                    random_precincts.add(random_precinct)

                    # Keep trying other precincts until one of them doesn't
                    # make an island
                    while isinstance(unchosen_precincts.coords, MultiPolygon):
                        # Give it back
                        community.give_precinct(
                            unchosen_precincts, random_precinct, **kwargs)
                        print(f"precinct {random_precinct} added to and removed"
                            f" from community {i} because it created an island")
                        # Random precinct that hasn't already been tried and also borders community.
                        random_precinct = random.sample(
                            community.get_bordering_precincts(unchosen_precincts) \
                            - random_precincts, 1)[0]
                        unchosen_precincts.give_precinct(
                            community, random_precinct, **kwargs)
                        random_precincts.add(random_precinct)

                    print(f"precinct {random_precinct} added to community {i}")

                except Exception as e:
                    print(unchosen_precincts.precincts)
                    # to save progress
                    with open("test_communities.pickle", "wb+") as f:
                        pickle.dump([communities, community], f)
                    raise e

            communities.append(community)

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
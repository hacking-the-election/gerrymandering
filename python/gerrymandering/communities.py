"""
Algorithm implementation for breaking a state into political communities
"""

import pickle
import random
from os.path import abspath, dirname
import sys

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
    if get_if_multipolygon(unchosen_precincts.border):
        # State is multipolygon, which means it has islands.
        pass
    else:
        for i, community_size in enumerate(community_sizes, 1):
            community = Community([], i)
            random_precincts = set()  # precincts that have been tried (whether or not they were added)
            for s in range(community_size):
                # Give random precinct to `community`
                try:
                    random_precinct = random.sample(
                        set(community.get_bordering_precincts(
                            unchosen_precincts)) \
                        - random_precincts, 1)[0]
                except IndexError as ie:
                    print(f"community {i} has these precincts: {list(community.precincts.keys())}")
                    logging.debug(unchosen_precincts.border)
                    print(f"precinct {s} from community {i} threw this:")
                    raise ie
                while get_if_multipolygon(unchosen_precincts.border):
                    # Adding that precinct created an island
                    community.give_precinct(unchosen_precincts,
                        random_precinct, partisanship=False,
                        standard_deviation=False, population=False,
                        compactness=False)  # give it back
                    print(f"precinct {random_precinct} added to and removed from community {i} because it created an island")
                    # Random precint that hasn't already been tried and also borders community.
                    random_precinct = random.sample(
                        set(community.get_bordering_precincts(
                            unchosen_precincts)) \
                        - random_precincts, 1)[0]
                    unchosen_precincts.give_precinct(
                        community, random_precinct, partisanship=False,
                        standard_deviation=False, population=False,
                        compactness=False)
                    random_precincts.add(random_precinct)

                # Add the one that worked
                unchosen_precincts.give_precinct(
                    community, random_precinct, partisanship=False,
                    standard_deviation=False, population=False,
                    compactness=False)
                random_precincts.add(random_precinct)
                print(f"precinct {random_precinct} added to community {i}")
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
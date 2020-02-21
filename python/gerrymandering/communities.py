"""
Algorithm implementation for breaking a state into political communities
"""

import pickle
import random
from os.path import abspath, dirname
import sys

sys.path.append(abspath(dirname(dirname(__file__))))

from gerrymandering.utils import *


def create_initial_configuration(precincts):
    """
    Takes `precincts` as a list of precincts and returns a list of
    communities that consist of random groups of communities.
    """

    # Calculate `community_sizes`, a list of numbers corresponding to
    # the number of precincts each community should have.
    n_districts = len(districts["features"])
    n_precincts = len(precincts)
    community_sizes = [n_precincts // n_districts for _ in range(n_districts)]
    for i in range(n_districts % n_precincts):
        community_sizes[i] += 1

    # Find which islands are capable of containing a whole number of
    # communities to start with.
    state_border = clip([p.coords for p in precincts])
    unchosen_precincts = Community(precincts[:], 0)
    communities = []  # Will contain Community objects.
    if get_if_multipolygon(state_border):
        # State is multipolygon, which means it has islands.
        pass
    else:
        for i, community_size in enumerate(community_sizes):
            community = Community([], i)
            for _ in range(community_size):
                # Give random precinct to `community`
                random_precincts = {}
                while True:
                    # Random precint that hasn'talready been tried.
                    random_precinct = random.choice(
                        set(unchosen_precinct.precincts.keys())\
                            - random_precincts)
                    unchosen_precincts.give_precinct(
                        random_precinct, community, partisanship=False,
                        standard_deviation=False, population=False,
                        compactness=False)
                    random_precincts.add(random_precinct)
                    if (get_if_multipolygon(unchosen_precincts.border)):
                        # Adding that precinct created an island
                        community.give_precinct(random_precinct,
                            unchosen_precincts, partisanship=False,
                            standard_deviation=False, population=False,
                            compactness=False)  # give it back
                    else:
                        break

    return communities
    


def make_communities(state_file):
    """
    `state_file` - path to state that is to be divided into communities
    """

    with open(state_file, 'rb') as f:
        state_data = pickle.load(f)
    precincts = state_data[0]
    districts = state_data["features"]  # Still in geojson format.

    # Step 1
    communities = create_initial_configuration(precincts)

    return communities
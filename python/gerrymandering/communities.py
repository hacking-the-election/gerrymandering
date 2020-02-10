"""
Algorithm implementation for breaking a state into political communities
"""

import pickle
import random

from utils import *


def make_communities(state_file, number_of_communities,
                     compactness_tolerance, partisanship_tolerance,
                     population_tolerance):
    """
    `state_file` - path to state that is to be divided into communities
    """
    
    with open(state_file, 'rb') as f:
        precincts, districts = pickle.load(f)

    community_sizes = []  # number of precincts in each community

    n_districts = len(districts["features"])
    n_precincts = len(precincts)
    community_size = [n_precincts // n_districts for _ in range(n_districts)]
    for i in range(n_districts % n_precincts):
        community_size[i] += 1
    
    # create random communities of similar size (in precincts)
    communities = []

    border = Community(precincts)
    for s in community_sizes:
        community = Community([])
        for p in range(s):
            eligible_precincts = border.precincts[:]
            precinct = random.choice(eligible_precincts)
            while not get_if_addable(precinct, community, border):
                eligible_precincts.remove(precinct)
                precinct = random.choice(border.precincts)
            community.precincts.append(precinct)
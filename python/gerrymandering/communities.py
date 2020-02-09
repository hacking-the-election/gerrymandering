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
    
    # choose random precinct on edge of state to begin with
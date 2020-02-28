"""
Algorithm implementation for breaking a state into political communities
"""

import pickle
import random
from os.path import abspath, dirname
import sys

from shapely.geometry import Polygon, MultiPolygon

sys.path.append(abspath(dirname(dirname(__file__))))

from utils import *
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
        # Configuration of communities for each island.
        island_community_counts = []
        for i, island in enumerate(island_precinct_groups):
            # Each element is list of x, y, and difference with number
            # of precincts.
            try:
                community_grouping_attempts = []
                for x in range(community_sizes.count(small_community)):
                    for y in range(community_sizes.count(large_community)):
                        if ((n_precincts := \
                            x * small_community + y * large_community)
                                == len(island)):
                            island_community_counts.append([x, y])
                            # Remove x small communities and y large ones
                            # because those many precincts were used in
                            # making this community.
                            for _ in range(x):
                                community_sizes.remove(small_community)
                            for _ in range(y):
                                community_sizes.remove(large_community)
                            raise LoopBreakException
                        elif n_precinct > len(island):
                            break
                        else:
                            community_grouping_attempts.append(
                                [x, y, n_precincts - len(island)]
                            )
                # No configuration works out
                best_configuration = min(community_grouping_attempts,
                                         key=lambda x: x[-1])[0:2]
                for _ in range(x):
                    community_sizes.remove(small_community)
                for _ in range(y):
                    community_sizes.remove(large_community)
                # 0.5 added to show it's going
                # to have a split community
                island_community_counts.append([x + 0.5, y + 0.5])
            except LoopBreakException:
                pass
         
        # "linked precincts" are chosen between islands.
        linked_precinct_pairs = []
        all_linked_precincts = set()
        for island_precincts, island_communities in zip(
                island_precinct_groups, island_community_counts):
            
            try:
                if island_community_counts[0] % 1 != 0:
                    # There is a 0.5 added to the first value of the
                    # community configuration meaning there is a fracional
                    # community on this island.
                    try:
                        for precinct in island_precincts:
                            if precinct.vote_id in all_linked_precincts:
                                raise LoopBreakException

                        precinct_group_copy = island_precinct_groups[:]
                        precinct_group_copy.remove(island_precincts)
                        precinct_pair = get_precinct_link_pair(
                            island_precinct_groups, precinct_group_copy)
                        linked_precinct_pairs.append(precinct_pair)
                        for precinct in precinct_pair:
                            all_linked_precincts.add(precinct.vote_id)

                    except LoopContinueException:
                        continue
            except LoopBreakException:
                pass
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

    # Step 1
    communities = create_initial_configuration(precincts, len(districts))

    return communities
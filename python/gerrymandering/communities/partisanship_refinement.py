"""
One of the refinement processes in the communities algorithm.

Alters a set of communities such that their precincts all have a
standard deviation below a certain threshold.
"""


from os.path import dirname
import sys

sys.path.append(dirname(dirname(__file__)))

from utils.partisanship import get_bordering_precincts
from utils.stats import average, stdev
from utils.geometry import shapely_to_polygon, get_if_bordering


def modify_for_partisanship(communities_list, precinct_corridors, threshold):
    '''
    Takes list of community objects, and returns a different list with the modified communities.,
    as well as the # of precincts that changed hands during this step.
    precinct_corridors should be a list of lists of two precincts, which are "connected", i.e.
    should be considered bordering. The threshold is a decimal for maximum community partisanship 
    standard deviation, i.e. (0.05)
    '''
    communities_coords = {community.id : community.precincts for community in communities_list}
    # update partisanship values (in case this hasn't already been done)
    for community in communities_list.values():
        community.update_standard_deviation()
    # create dictionary of ids and community partisanship standard deviations
    community_stdev = {community.id : community.standard_deviation for community in communities_list}
    # create json polygon coordinate dict for our communities
    json_communities = {community.id : shapely_to_polygon(community.coords) 
                        for community in communities_list}
    # check if any communities are above the threshold
    # count the number of times the list has been checked
    count = 0
    num_of_above_precincts = 0
    average_stdev = average(community_stdev.values())
    for id, community in community_stdev.items():
        # if a community is above the threshold
        if community > threshold:
            # find the community with the highest political deviaiton
            most_stdev = {}
            for id1, community1 in community_stdev.items():
                if community > most_stdev.get(id, 0):
                    most_stdev[id] = community
            biggest_community_precincts = communities_coords[most_stdev.keys()[0]]
            # relates border precinct to community border it's from (not including biggest_community)
            border_precincts = {'biggest_community': []}
            # create dictionary of the precincts bordering it
            # group_by_islands(biggest_community_precincts)
            for id1, community1 in community_stdev.items():
                # if community is biggest one, skip
                if id1 == most_stdev.keys()[0]:
                    continue
                if get_if_bordering(shapely_to_polygon(most_stdev.values()[0].coords), 
                                    json_communities[id1]):
                    # the following result has first key: precincts ids inside most stdev community
                    # second key: precinct ids in other community
                    specific_border_precincts = get_bordering_precincts(most_stdev.values()[0], community1)
                    for precinct in specific_border_precincts[most_stdev.keys()[0]]:
                        border_precincts['biggest_community'].append(precinct)
                    if specific_border_precincts[id1] != 0: 
                        try:
                            border_precincts[id1].extend(specific_border_precincts[id1])
                        except KeyError:
                            border_precincts[id1] = [specific_border_precincts[id1]]
            # add precinct corridors that have precincts inside community, but only one
            for precinct in most_stdev.values()[0].precincts:
                for pair in precinct_corridors:
                    if precinct in pair:
                        # if both sides of a precinct corridor are in the community, 
                        # no need to be added
                        if pair[:].remove(precinct)[0] in community.precincts:
                            continue
                        else:
                            border_precincts.other(pair[:].remove(precinct))
            # TODO: remove duplicates
            # find which precinct exchanges are the best
            precinct_exchanges_dict = {}
            # for border precincts within the highest stdev community, find stdev without that precinct
            community_stdev = most_stdev.values()[0].standard_deviation
            for precinct in border_precincts['biggest_community']:
                other_precinct_list = most_stdev.values()[0].precincts.values()[:].remove(precinct)
                precinct_stdev = stdev([precinct.r_election_sum for precinct in other_precinct_list])
                precinct_exchanges_dict[(community_stdev - precinct_stdev)] = precinct 
            # for border precincts outside the highest stdev community, find stdev with that precinct
            for key in border_precincts.keys()[1:]:
                for precinct in border_precincts[key]:
                    added_precinct_list = most_stdev.values()[0].precincts.values()[:].append(precinct)
                    precinct_stdev = stdev([precinct.r_election_sum for precinct in other_precinct_list])
                    precinct_exchanges_dict[(community_stdev - precinct_stdev)] = precinct
            # add or remove precincts from border_precincts until there are no more beneficial exchanges,
            # or until the community's standard deviation is below the threshold
            while most_stdev.values()[0].standard_deviation > threshold:
                # if there is only one precinct left, just stop
                if len(most_stdev.values()[0].precincts) == 1:
                    break
                highest_precinct_exchange = max(precinct_exchanges_dict.keys())
                high_precinct = precinct_exchanges_dict[highest_precinct_exchange]
                # TODO: if removing or adding precinct would create island, delete that precinct
                # from consideration
                
                # find other community to add/subtract from
                for community2, precincts in border_precincts.items():
                    if high_precinct in precincts:
                        other_community = community2
                # if precinct is in biggest stdev community:
                if most_stdev.values()[0].coords.contains(high_precinct.coords):
                    # give precinct from most_stdev to other community
                    most_stdev.values()[0].give_precinct(other_community, high_precinct.id)
                else:
                    other_community.give_precinct(most_stdev.values()[0], high_precinct.id)
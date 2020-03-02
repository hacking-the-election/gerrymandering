"""
One of the refinement processes in the communities algorithm.

Alters a set of communities such that their precincts all have a
standard deviation below a certain threshold.
"""


from os.path import dirname, abspath
import sys
import pickle

sys.path.append(dirname(dirname(dirname(abspath(__file__)))))

from gerrymandering.utils.initial_configuration import Community
from gerrymandering.utils.partisanship import get_bordering_precincts
from gerrymandering.utils.stats import average, stdev
from gerrymandering.utils.geometry import shapely_to_polygon, polygon_to_shapely, get_if_bordering, communities_to_json
from serialization import save_precincts
sys.modules['save_precincts'] = save_precincts

def modify_for_partisanship(communities_list, precinct_corridors, threshold):
    '''
    Takes list of community objects, and returns a different list with the modified communities.,
    as well as the # of precincts that changed hands during this step.
    precinct_corridors should be a list of lists of two precincts, which are "connected", i.e.
    should be considered bordering. The threshold is a decimal for maximum community partisanship 
    standard deviation, i.e. (0.05)
    '''
    communities_dict = {community.id : community for community in communities_list}
    communities_coords = {community.id : community.precincts for community in communities_list}
    # update partisanship values (in case this hasn't already been done)
    for community in communities_list:
        
        community.update_standard_deviation()
        print(community.standard_deviation)
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
            for community in communities_list:
                if len(most_stdev) == 0: 
                    most_stdev[community.id] = community

                elif community.standard_deviation > most_stdev[list(most_stdev.keys())[0]].standard_deviation:
                    del most_stdev[list(most_stdev.keys())[0]]
                    most_stdev[community.id] = community
            most_stdev_id = list(most_stdev.keys())[0]
            most_stdev_community = list(most_stdev.values())[0]
            biggest_community_precincts = communities_coords[most_stdev_id]
            # relates border precinct to community border it's from (not including most_stdev_community)
            border_precincts = {most_stdev_community.id: []}
            # create dictionary of the precincts bordering it
            for id1, community1 in communities_dict.items():
                # if community is biggest one, skip
                if id1 == most_stdev_id:
                    continue
                print(most_stdev_id, community1.id)
                if get_if_bordering(most_stdev_community.coords, 
                                    polygon_to_shapely(json_communities[id1])):
                    # the following result has first key: precincts ids inside most stdev community
                    # second key: precinct ids in other community
                    specific_border_precincts = get_bordering_precincts(most_stdev_community, community1)
                    for precinct in specific_border_precincts[most_stdev_id]:
                        border_precincts[most_stdev_community.id].append(precinct)
                    if specific_border_precincts[id1] != 0: 
                        try:
                            border_precincts[id1].extend(specific_border_precincts[id1])
                        except KeyError:
                            border_precincts[id1] = [specific_border_precincts[id1]]
            # add precinct corridors that have precincts inside community, but only one
            for precinct in most_stdev_community.precincts:
                for pair in precinct_corridors:
                    if precinct in pair:
                        # if both sides of a precinct corridor are in the community, 
                        # no need to be added
                        if pair[:].remove(precinct)[0] in community.precincts:
                            continue
                        else:
                            for community1 in communities_list:
                                if precinct in community1.precincts.values():
                                    border_precincts[community1] = pair[:].remove(precinct)
            # remove duplicate border precincts from list to consider
            no_duplicate_list = []
            for id, precinct_list in border_precincts.items():
                no_duplicate_precinct_list = []
                for precinct in precinct_list:
                    if precinct not in no_duplicate_list:
                        no_duplicate_list.append(precinct)
                        no_duplicate_precinct_list.append(precinct)
                border_precincts[id] = no_duplicate_precinct_list
            # find which precinct exchanges are the best
            precinct_exchanges_dict = {}
            # for border precincts within the highest stdev community, find stdev without that precinct
            community_stdev = most_stdev_community.standard_deviation
            print(community_stdev)
            for num, precinct in enumerate(border_precincts[most_stdev_community.id]):
                other_precinct_list = list(most_stdev_community.precincts.values())[:]
                del other_precinct_list[num]
                precinct_stdev = stdev([(precinct.r_election_sum * 100)/(precinct.r_election_sum + precinct.d_election_sum) 
                                       for precinct in other_precinct_list])
                print(precinct_stdev)
                precinct_exchanges_dict[(community_stdev - precinct_stdev)] = precinct 
            # for border precincts outside the highest stdev community, find stdev with that precinct
            for key in list(border_precincts.keys())[1:]:
                for precinct_list in border_precincts[key]:
                    for precinct in precinct_list:
                        added_precinct_list = list(most_stdev_community.precincts.values())[:].append(precinct)
                        precinct_stdev = stdev([precinct.r_election_sum for precinct in other_precinct_list])
                        precinct_exchanges_dict[(community_stdev - precinct_stdev)] = precinct
            # add or remove precincts from border_precincts until there are no more beneficial exchanges,
            # or until the community's standard deviation is below the threshold
            while most_stdev_community.standard_deviation > threshold:
                print(most_stdev_community.standard_deviation, threshold)
                # if there is only one precinct left, just stop
                if len(most_stdev_community.precincts) == 1:
                    break
                highest_precinct_exchange = max(precinct_exchanges_dict.keys())
                high_precinct = precinct_exchanges_dict[highest_precinct_exchange]
                print(precinct_exchanges_dict.keys())
                # if there are no beneficial precinct exchanges left, stop
                if highest_precinct_exchange <= 0:
                    break
                # find other community to add/subtract from
                for community_id, precincts_list in border_precincts.items():
                    if high_precinct in precincts_list:
                        # find the community with the corresponding id
                        for community3 in communities_list:
                            if community3.id == community_id:
                                other_community = community3
                                to_replace = community3
                # find precincts that can no longer be used now once a precinct has changed hands
                no_longer_applicable_precincts = []
                # if precinct is in biggest stdev community:
                if most_stdev_community.coords.contains(high_precinct.coords):
                    # give precinct from most_stdev to other community
                    most_stdev_community.give_precinct(other_community, high_precinct.vote_id)
                    del precinct_exchanges_dict[highest_precinct_exchange]
                    for precinct_list in border_precincts.values():
                        for precinct in precinct_list:
                            try:
                                if not get_if_bordering(polygon_to_shapely(precinct.coords), most_stdev_community.coords):
                                    no_longer_applicable_precincts.append(precinct)
                            except AttributeError:
                                # precinct has no coordinates, which happens sometimes for some reason
                                pass
                # precinct is not in biggest stdev community
                else:
                    other_community.give_precinct(polygon_to_shapely(most_stdev_community), high_precinct.vote_id)
                    del precinct_exchanges_dict[highest_precinct_exchange]
                    for precinct_list in border_precincts.values():
                        for precinct in precinct_list:
                            try:
                                if not get_if_bordering(precinct.coords, other_community.coords):
                                    no_longer_applicable_precincts.append(precinct)
                            except AttributeError:
                                # precinct has no coordinates, which happens sometimes for some reason
                                pass
                # removes precincts that can't be added/removed now that a precinct has been added
                for id, precinct_list in border_precincts.items():
                    precinct_list_to_remove = []
                    for precinct in precinct_list:
                        if precinct not in no_longer_applicable_precincts:
                            precinct_list_to_remove.append(precinct)
                    border_precincts[id] = precinct_list_to_remove
                for community in communities_list:
                    if community.id == most_stdev_id:
                        community = most_stdev_community
                for community in communities_list:
                    if community.id == other_community.id:
                        community = other_community
                
    communities_to_json(communities_list, '../../../../partisanship_after.json')
    return communities_list

# just for testing, will delete later
with open('../../../../test_communities.pickle', 'rb') as f:
    x = pickle.load(f)

modify_for_partisanship(x, [], 0.1)
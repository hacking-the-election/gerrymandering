"""
One of the refinement processes in the communities algorithm.

Alters a set of communities such that their precincts all have a
standard deviation below a certain threshold.
"""

import sys
import pickle

from hacking_the_election.utils.initial_configuration import Community
from hacking_the_election.utils.partisanship import get_bordering_precincts
from hacking_the_election.utils.stats import average, stdev
from hacking_the_election.utils.geometry import shapely_to_polygon, polygon_to_shapely, get_if_bordering, communities_to_json
from hacking_the_election.serialization import save_precincts
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
        print(community.id, community.standard_deviation)
    # create dictionary of ids and community partisanship standard deviations
    community_stdev = {community.id : community.standard_deviation for community in communities_list}
    # create json polygon coordinate dict for our communities
    json_communities = {community.id : shapely_to_polygon(community.coords) 
                        for community in communities_list}
    # check if any communities are above the threshold
    # count the number of times the list has been checked
    count = 0
    num_of_changed_precincts = []
    success = 'no'
    # average stdev tracks the average_standard_deviation across all communities throughout iterations
    average_stdev = []
    # standard_deviations will store comma seperated standard deviations for communities, with rows 
    # being iterations
    standard_deviations = []
    while count < 10:
        count += 1
        # add to average_stdev
        average_stdev.append(average([community.standard_deviation for community in communities_list]))
        # create number_of_changed_iteration variable to store # of precincts changed this iteration
        num_of_changed_iteration = 0
        # find the highest standard deviation out of the communities in community_list
        most_stdev = {}
        for community in communities_list:
            if len(most_stdev) == 0: 
                most_stdev[community.id] = community

            elif community.standard_deviation > most_stdev[list(most_stdev.keys())[0]].standard_deviation:
                del most_stdev[list(most_stdev.keys())[0]]
                most_stdev[community.id] = community
        most_stdev_id = list(most_stdev.keys())[0]
        most_stdev_community = list(most_stdev.values())[0] 
        if most_stdev_community.standard_deviation < threshold:
            success = 'yes!'
            break       
        # if all the code in this while loop below runs, then there must be a community
        # with greater than average standard deviation
        biggest_community_precincts = communities_coords[most_stdev_id]
        # relates border precinct to community border it's from (not including most_stdev_community)
        border_precincts = {most_stdev_community.id: []}
        other_commmunities_dict = {community: [] for community in communities_list}
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
                    border_precincts[most_stdev_id].append(precinct)
                    other_commmunities_dict[communities_dict[id1]].append(precinct)
                if specific_border_precincts[id1] != 0: 
                    for precinct in specific_border_precincts[id1]:
                        other_commmunities_dict[most_stdev_community].append(precinct)
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
                                other_commmunities_dict[most_stdev_community].append(precinct)
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
        community_stdev_stat = most_stdev_community.standard_deviation
        for num, precinct in enumerate(border_precincts[most_stdev_community.id]):
            other_precinct_list = list(most_stdev_community.precincts.values())[:]
            del other_precinct_list[num]
            precinct_stdev = stdev([(precinct.r_election_sum * 100)/(precinct.r_election_sum + precinct.d_election_sum) 
                                    for precinct in other_precinct_list
                                    if (precinct.r_election_sum + precinct.d_election_sum) != 0])
            precinct_exchanges_dict[(community_stdev_stat - precinct_stdev)] = precinct 
        # for border precincts outside the highest stdev community, find stdev with that precinct
        for key in list(border_precincts.keys())[1:]:
            for precinct_list in border_precincts[key]:
                for precinct in precinct_list:
                    added_precinct_list = list(most_stdev_community.precincts.values())[:].append(precinct)
                    precinct_stdev = stdev([(precinct.r_election_sum * 100)/(precinct.r_election_sum + precinct.d_election_sum) 
                    for precinct in other_precinct_list
                    if (precinct.r_election_sum + precinct.d_election_sum) != 0])
                    precinct_exchanges_dict[(community_stdev_stat - precinct_stdev)] = precinct
        # add or remove precincts from border_precincts until there are no more beneficial exchanges,
        # or until the community's standard deviation is below the threshold
        while most_stdev_community.standard_deviation > threshold:
            print(most_stdev_id, most_stdev_community.standard_deviation, threshold)
            # if there is only one precinct left, just stop
            if len(most_stdev_community.precincts) == 1:
                break
            try:
                highest_precinct_exchange = max(precinct_exchanges_dict.keys())
                high_precinct = precinct_exchanges_dict[highest_precinct_exchange]
            except ValueError:
                break
            # if there are no beneficial precinct exchanges left, stop
            if highest_precinct_exchange <= 0:
                break
            # find other community to add/subtract from
            for community1, precinct_list in other_commmunities_dict.items():
                if high_precinct in precinct_list:
                    other_community = community1
            # find not involved community
            for community1 in communities_list:
                if community1 != other_community:
                    if community1 != most_stdev_community:
                        not_involved_community = community1
            try:
                print(not_involved_community.id, other_community.id, most_stdev_id)
            except:
                print('missing community')
            # for community_id, precincts_list in border_precincts.items():
            #     if high_precinct in precincts_list:
            #         # find the community with the corresponding id
            #         for community3 in communities_list:
            #             if community3.id == community_id:
            #                 other_community = community3
            #                 to_replace = community3

            # find precincts that can no longer be used now once a precinct has changed hands
            no_longer_applicable_precincts = []
            # if precinct is in biggest stdev community:
            if high_precinct in most_stdev_community.precincts.values():
                # give precinct from most_stdev to other community
                del most_stdev_community.precincts[high_precinct.vote_id]
                other_community.precincts[high_precinct.vote_id] = high_precinct
                most_stdev_community.update_partisanship()
                most_stdev_community.update_standard_deviation()
                most_stdev_community.update_compactness()
                most_stdev_community.update_coords()
                other_community.update_partisanship()
                other_community.update_standard_deviation()
                other_community.update_compactness()
                other_community.update_coords()
                # update number of precincts changed on this iteration
                num_of_changed_iteration += 1
                # most_stdev_community.give_precinct(other_community, high_precinct.vote_id)
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
                most_stdev_community.precincts[high_precinct.vote_id] = high_precinct
                del other_community.precincts[high_precinct.vote_id]
                most_stdev_community.update_partisanship()
                most_stdev_community.update_standard_deviation()
                most_stdev_community.update_compactness()
                most_stdev_community.update_coords()
                other_community.update_partisanship()
                other_community.update_standard_deviation()
                other_community.update_compactness()
                other_community.update_coords()
                # update number of precincts changed on this iteration
                num_of_changed_iteration += 1
                # other_community.give_precinct(polygon_to_shapely(most_stdev_community), high_precinct.vote_id)
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
        stdev_stat = str([community.standard_deviation for community in communities_list])[1:-1]
        standard_deviations.append(stdev_stat)
        for community in communities_list:
            print(community.standard_deviation)
        # save number_of_changed_precincts_this_iteration to running list
        num_of_changed_precincts.append(num_of_changed_iteration)
        # create new communities list
        community_change_snapshot = [other_community, most_stdev_community, not_involved_community]
        communities_to_json(community_change_snapshot, str('../../../../partisanship_after' + str(count) + '.json'))
    communities_to_json(communities_list, '../../../../partisanship_after.json')
    return communities_list, count, standard_deviations, num_of_changed_precincts, average_stdev

# just for testing, will delete later
with open('../../../../test_communities.pickle', 'rb') as f:
    x = pickle.load(f)

_, count1, standard_deviations1, num_of_changed_precincts1, average_stdev1 = modify_for_partisanship(x, [], 0.1)

print('# of iterations:', count1)
print('standard deviations:', standard_deviations1)
print('# of changed precincts per iteration:', num_of_changed_precincts1)
print('average standard deviation across iterations:', average_stdev1)
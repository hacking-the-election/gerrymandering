"""
One of the refinement processes in the communities algorithm.

Alters a set of communities such that their precincts all have a
standard deviation below a certain threshold.
"""

import sys
import pickle
import json
from time import time
from hacking_the_election.utils.community import Community
from hacking_the_election.utils.partisanship import get_bordering_precincts
from hacking_the_election.utils.stats import average, stdev
from hacking_the_election.utils.geometry import shapely_to_polygon, polygon_to_shapely, get_if_bordering, communities_to_json, clip
from hacking_the_election.utils.exceptions import CreatesMultiPolygonException
from hacking_the_election.serialization import save_precincts
from shapely.geometry import MultiPolygon
sys.modules['save_precincts'] = save_precincts

def modify_for_partisanship(communities_list, precinct_corridors, threshold):
    '''
    Takes list of community objects, and returns a different list with the modified communities.,
    as well as the # of precincts that changed hands during this step.
    precinct_corridors should be a list of lists of two precincts, which are "connected", i.e.
    should be considered bordering. The threshold is a decimal for maximum community partisanship 
    standard deviation, i.e. (0.05)
    '''
    with open('../../../../geodata-1.json', 'r') as f:
        x = json.load(f)
        shape1 = polygon_to_shapely(x["features"][0]["geometry"]["coordinates"])
        shape2 = polygon_to_shapely(x["features"][1]["geometry"]["coordinates"])
    communities_dict = {community.id : community for community in communities_list}
    communities_precincts = {community.id : community.precincts for community in communities_list}
    # create dictionary of ids and community partisanship standard deviations
    community_stdev = {community.id : community.standard_deviation for community in communities_list}
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
        # update attribute values (in case this hasn't already been done)
        for community in communities_list:
            community.update_standard_deviation()
            community.update_coords()
            community.update_partisanship()
            print(community.id, community.standard_deviation)
        print(time())
        count += 1
        # add to average_stdev
        average_stdev.append(average([community.standard_deviation for community in communities_list]))
        # create number_of_changed_iteration variable to store # of precincts changed this iteration
        num_of_changed_iteration = 0
        # find the highest standard deviation out of the communities in community_list
        most_stdev = {}
        for community5 in communities_list:
            if len(most_stdev) == 0: 
                most_stdev[community5.id] = community5

            elif community5.standard_deviation > most_stdev[list(most_stdev.keys())[0]].standard_deviation:
                del most_stdev[list(most_stdev.keys())[0]]
                most_stdev[community5.id] = community5
        most_stdev_id = list(most_stdev.keys())[0]
        most_stdev_community = list(most_stdev.values())[0] 
        if most_stdev_community.standard_deviation < threshold:
            success = 'yes!'
            break       


        # if all the code in this while loop below runs, then there must be a community
        # with greater than average standard deviation
        biggest_community_precincts = communities_precincts[most_stdev_id]
        # relates border precinct to community border it's from (not including most_stdev_community)
        border_precincts = {most_stdev_community.id: []}
        other_communities_dict = {community9: [] for community9 in communities_list}

        # create dictionary of the precincts bordering it
        for id1, community1 in communities_dict.items():
            # if community is biggest one, skip
            if id1 == most_stdev_id:
                continue
            print(most_stdev_id, community1.id)
            if get_if_bordering(most_stdev_community.coords, 
                                community1.coords):
                # the following result has first key: precincts ids inside most stdev community
                # second key: precinct ids in other community
                specific_border_precincts = get_bordering_precincts(most_stdev_community, community1)
                for precinct in specific_border_precincts[most_stdev_id]:
                    border_precincts[most_stdev_id].append(precinct)
                    other_communities_dict[communities_dict[id1]].append(precinct)
                if len(specific_border_precincts[id1]) != 0: 
                    for precinct5 in specific_border_precincts[id1]:
                        other_communities_dict[most_stdev_community].append(precinct5)
                    try:
                        border_precincts[id1].extend(specific_border_precincts[id1])
                    except KeyError:
                        border_precincts[id1] = specific_border_precincts[id1]
        # add precinct corridors that have precincts inside community, but only one
        for precinct6 in most_stdev_community.precincts:
            for pair in precinct_corridors:
                if precinct6 in pair:
                    # if both sides of a precinct corridor are in the community, 
                    # no need to be added
                    if pair[:].remove(precinct6)[0] in community.precincts:
                        continue
                    else:
                        for community6 in communities_list:
                            if precinct in community6.precincts.values():
                                border_precincts[community6] = pair[:].remove(precinct6)
                                other_communities_dict[most_stdev_community].append(precinct6)
        # remove duplicate border precincts from list to consider
        no_duplicate_list = []
        for id3, precinct_list in border_precincts.items():
            no_duplicate_precinct_list = []
            for precinct7 in precinct_list:
                if precinct7 not in no_duplicate_list:
                    no_duplicate_list.append(precinct7)
                    no_duplicate_precinct_list.append(precinct7)
            border_precincts[id3] = no_duplicate_precinct_list
        border_precincts_list = []
        for precinct_list1 in border_precincts.values():
            for precinct1 in precinct_list1:
                border_precincts_list.append(precinct1)
        z = '../../../../border' + str(count) + '.json'
        with open(z, 'w') as f:
            try:
                _ = shapely_to_polygon(clip([precinct8.coords for precinct8 in border_precincts_list], 1))[0][0][0][0]
                y = json.dumps({"type":"FeatureCollection", "features":[{"type": "Feature", "geometry": {"type":"MultiPolygon", "coordinates":shapely_to_polygon(clip([precinct.coords for precinct in border_precincts_list], 1)), "properties":{"ID":"border"}}}]})
            except:
                y = json.dumps({"type":"FeatureCollection", "features":[{"type": "Feature", "geometry": {"type":"Polygon", "coordinates":shapely_to_polygon(clip([precinct.coords for precinct in border_precincts_list], 1)), "properties":{"ID":"border"}}}]})
            f.write(y)
            print('hello, border json ready')
        
        for bro, precinctlist in other_communities_dict.items():
            for precinct20 in precinctlist:
                if "50005VD47" == precinct20.vote_id:
                    print("see", bro.id)
        # find which precinct exchanges are the best
        precinct_exchanges_dict = {}
        # for border precincts within the highest stdev community, find stdev without that precinct
        community_stdev_stat = most_stdev_community.standard_deviation
        for num, precinct3 in enumerate(border_precincts[most_stdev_community.id]):
            # check to make sure removing community from most_stdev_community
            # does not lead to non-contiguous communities
            # if isinstance(clip([most_stdev_community.coords, precinct3.coords], 2), MultiPolygon):
            #     print('stuff eliminated')
            #     continue
            # after new precincts are serialized use stdev(precinct.rep_total_ratio) instead of current
            # else:
            other_precinct_list = list(most_stdev_community.precincts.values())[:]
            del other_precinct_list[num]
            precinct_stdev = stdev([(precinct3.r_election_sum * 100)/(precinct3.r_election_sum + precinct3.d_election_sum) 
                                    for precinct3 in other_precinct_list
                                    if (precinct3.r_election_sum + precinct3.d_election_sum) != 0])
            precinct_exchanges_dict[(community_stdev_stat - precinct_stdev)] = precinct3 
        print(len(precinct_exchanges_dict.keys()))
        # for border precincts outside the highest stdev community, find stdev with that precinct
        for key in list(border_precincts.keys())[1:]:
            for precinct4 in border_precincts[key]:
                # check to make sure removing community from most_stdev_community
                # does not lead to non-contiguous communities
                # find community this precinct in precinct_list is from
                for community2 in communities_list:
                    if precinct4 in community2.precincts.values():
                        other_community_precinct_list1 = community2
                # if isinstance(clip([other_community_precinct_list1.coords, precinct4.coords], 2), MultiPolygon):
                #     print('stuff also eliminated')
                #     continue
                # else:
                added_precinct_list = list(most_stdev_community.precincts.values())[:]
                added_precinct_list.append(precinct4)
                precinct_stdev = stdev([(precinct4.r_election_sum * 100)/(precinct4.r_election_sum + precinct4.d_election_sum) 
                for precinct4 in added_precinct_list
                if (precinct4.r_election_sum + precinct4.d_election_sum) != 0])
                precinct_exchanges_dict[(community_stdev_stat - precinct_stdev)] = precinct4
        print('before duplicate removal, ', len(precinct_exchanges_dict.keys()))
        to_remove = []
        for sta_dev in precinct_exchanges_dict.keys():
            removed = list(precinct_exchanges_dict.keys())[:]
            removed.remove(sta_dev)
            if sta_dev in removed:
                to_remove.append(sta_dev)
        for sta_dev1 in to_remove:
            del precinct_exchanges_dict[sta_dev1]
        print(len(precinct_exchanges_dict.keys()))
        # add or remove precincts from border_precincts until there are no more beneficial exchanges,
        # or until the community's standard deviation is below the threshold
        precinct_count = 1
        while most_stdev_community.standard_deviation > threshold:
            print(most_stdev_id, most_stdev_community.standard_deviation, threshold)
            # if there is only one precinct left, just stop
            if len(most_stdev_community.precincts) == 1:
                break
            try:
                highest_precinct_exchange = max(precinct_exchanges_dict.keys())
                high_precinct = precinct_exchanges_dict[highest_precinct_exchange]
            # no more precinct exchanges left
            except ValueError:
                break
            # if there are no beneficial precinct exchanges left, stop
            if highest_precinct_exchange <= 0:
                break
            # find other community to add/subtract from
            # for community3, precinct_list2 in other_communities_dict.items():
            #     if high_precinct in precinct_list2:
            #         if community3 == most_stdev_community:
            #             for community4, precinct_list4 in border_precincts.items():
            #                 if high_precinct in precinct_list4:
            #                     other_community = communities_dict[community4]
            #         else:
            #             other_community = community3
            for community16, precinct_list10 in other_communities_dict.items():
                if high_precinct in precinct_list10:
                    if community16.id == most_stdev_community.id:
                        for community19 in communities_list:
                            if high_precinct in community19.precincts.values():
                                other_community = community19
                    else:
                        other_community = community16
            print('other community, ', other_community.id, 'highest stdev community, ', most_stdev_id)
            print(highest_precinct_exchange)
            # find precincts that can no longer be used now once a precinct has changed hands
            no_longer_applicable_precincts = []
            # if precinct is in biggest stdev community:
            if high_precinct in most_stdev_community.precincts.values():
                print(most_stdev_id, 'chosen')
                # give precinct from most_stdev to other community
                # double check for contiguity
                if isinstance(clip([most_stdev_community.coords, high_precinct.coords], 2), MultiPolygon):
                    del precinct_exchanges_dict[highest_precinct_exchange]
                    continue
                if isinstance(clip([other_community.coords, high_precinct.coords], 1), MultiPolygon):
                    del precinct_exchanges_dict[highest_precinct_exchange]
                    continue
                try:
                    most_stdev_community.give_precinct(other_community, high_precinct.vote_id)
                except CreatesMultiPolygonException:
                    del precinct_exchanges_dict[highest_precinct_exchange]
                    continue
                # del most_stdev_community.precincts[high_precinct.vote_id]
                # other_community.precincts[high_precinct.vote_id] = high_precinct
                # most_stdev_community.update_partisanship()
                # most_stdev_community.update_standard_deviation()
                # most_stdev_community.update_compactness()
                # other_community.update_partisanship()
                # other_community.update_standard_deviation()
                # other_community.update_compactness()
                # update number of precincts changed on this iteration
                num_of_changed_iteration += 1
                # most_stdev_community.give_precinct(other_community, high_precinct.vote_id)
                del precinct_exchanges_dict[highest_precinct_exchange]
                for precinct_list3 in border_precincts.values():
                    for precinct2 in precinct_list3:
                        try:
                            if not get_if_bordering(polygon_to_shapely(precinct2.coords), most_stdev_community.coords):
                                no_longer_applicable_precincts.append(precinct2)
                        except AttributeError:
                            # precinct has no coordinates, which happens sometimes for some reason
                            pass
            # precinct is not in biggest stdev community
            elif high_precinct in other_community.precincts.values():
                print(other_community.id, 'chosen')
                if isinstance(clip([other_community.coords, high_precinct.coords], 2), MultiPolygon):
                    del precinct_exchanges_dict[highest_precinct_exchange]
                    continue
                if isinstance(clip([most_stdev_community.coords, high_precinct.coords], 1), MultiPolygon):
                    del precinct_exchanges_dict[highest_precinct_exchange]
                    continue
                try:
                    other_community.give_precinct(most_stdev_community, high_precinct.vote_id)
                except CreatesMultiPolygonException:
                    del precinct_exchanges_dict[highest_precinct_exchange]
                    continue
                # del other_community.precincts[high_precinct.vote_id]
                # most_stdev_community.precincts[high_precinct.vote_id] = high_precinct
                # most_stdev_community.update_partisanship()
                # most_stdev_community.update_standard_deviation()
                # most_stdev_community.update_compactness()
                # most_stdev_community.update_coords()
                # other_community.update_partisanship()
                # other_community.update_standard_deviation()
                # other_community.update_compactness()
                # other_community.update_coords()
                # update number of precincts changed on this iteration
                num_of_changed_iteration += 1
                # other_community.give_precinct(polygon_to_shapely(most_stdev_community), high_precinct.vote_id)
                del precinct_exchanges_dict[highest_precinct_exchange]
                for precinct_list5 in border_precincts.values():
                    for precinct9 in precinct_list5:
                        try:
                            if not get_if_bordering(precinct9.coords, other_community.coords):
                                no_longer_applicable_precincts.append(precinct9)
                        except AttributeError:
                            # precinct has no coordinates, which happens sometimes for some reason
                            pass
            else:
                print(high_precinct.vote_id, high_precinct)
            # removes precincts that can't be added/removed now that a precinct has been added
            for id2, precinct_list6 in border_precincts.items():
                precinct_list_to_remove = []
                for precinct10 in precinct_list6:
                    if precinct10 not in no_longer_applicable_precincts:
                        precinct_list_to_remove.append(precinct10)
                border_precincts[id2] = precinct_list_to_remove
            not_involved_community_list = []
            for community13 in not_involved_community_list:
                if community13 != other_community:
                    if community13 != most_stdev_community:
                        not_involved_community_list.append(community13)
            community_change_snapshot = [other_community, most_stdev_community]
            for community14 in not_involved_community_list:
                community_change_snapshot.append(community14)
            # communities_to_json(community_change_snapshot, str('../../../../partisanship_after' + str(precinct_count) + 'community'  + '.json'))
            print('HELLO', precinct_count)
            precinct_count += 1
        stdev_stat = str([community10.standard_deviation for community10 in communities_list])[1:-1]
        standard_deviations.append(stdev_stat)
        for community11 in communities_list:
            print(community11.standard_deviation)
        # save number_of_changed_precincts_this_iteration to running list
        num_of_changed_precincts.append(num_of_changed_iteration)
        # create new communities list
        not_involved_community_list = []
        # find not involved community
        for community12 in communities_list:
            if community12 != other_community:
                if community12 != most_stdev_community:
                    not_involved_community_list.append(community12)
        community_change_snapshot = [other_community, most_stdev_community]
        for community15 in not_involved_community_list:
            community_change_snapshot.append(community15)
        communities_to_json(community_change_snapshot, str('../../../../partisanship_after boi' + str(count) + '.json'))
    communities_to_json(communities_list, '../../../../partisanship_after.json')
    print('completed!')
    return communities_list, count, standard_deviations, num_of_changed_precincts, average_stdev

# just for testing, will delete later
with open('../../../../test_vermont_initial_configuration.pickle', 'rb') as f:
    x = pickle.load(f)
communities_to_json(x[0], '../../../../new_test_communities.json')
b, count1, standard_deviations1, num_of_changed_precincts1, average_stdev1 = modify_for_partisanship(x[0], x[1], 5)


print('# of iterations:', count1)
print('standard deviations:', standard_deviations1)
print('# of changed precincts per iteration:', num_of_changed_precincts1)
print('average standard deviation across iterations:', average_stdev1)


with open('../../../../end_of_partisanship.pickle', 'wb') as f:
    pickle.dump(b, f)
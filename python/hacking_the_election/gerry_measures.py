"""
This program that let us calculate established gerrymandering measures as a reference point
for our quantification program. 

We plan to implement:
    - `Efficency Gap`
    - `Votes to Seats`
    - `Declination`
    and more.

In addition, there is also functionality for outputting hypothetical election results using the communities and districts
that we generate.


Usage:
python gerry_measures.py [generated communities/districts file.txt/district.json] [state.pickle] (-e/-d/-vts/-m) (-d)

Alternatively, [generated communities/districts file] can be switched out for district.json.

[state.pickle] is the serialized python pickle of the state in question.
(-e/-d/-vts/-m) is an optional flag for which gerrymandering measures and/or mock election is required. If ommited, it outputs all.
Combining flags like "-e-d" is legal.   

(-v) is an optional flag. If added, it signals that outputs on a district level should also be printed,
much like a standard verbose flag. This only applies if this can be applied (e.g, not on declination)
"""
import pickle
import json
import sys
from os.path import dirname, abspath
from math import ceil, pi
import matplotlib.pyplot as plt
from copy import deepcopy
sys.path.append(dirname(dirname(abspath(__file__))))

from hacking_the_election.utils.precinct import Precinct
from hacking_the_election.utils.community import Community
from hacking_the_election.utils.geometry import area, shapely_to_geojson, geojson_to_shapely

def _find_voter_data(geodata, state_graph, party_list):
    """
    Takes shapely coords from a district and returns election results.
    """
    precinct_list = []
    votes_list = [0 for party in party_list]
    total_party_list = ["total" + party[7:] for party in party_list]
    for node in state_graph:
        precinct_list.append(state_graph.node_attributes(node)[0])
    shapely_district = geojson_to_shapely(geodata)
    district_bounding_box = shapely_district.bounds
    considerable_precincts = []
    # Cull the list of precincts to only those worthy of consideration
    for precinct in precinct_list:
        bounding_box = precinct.coords.bounds
        if not (bounding_box[0] < district_bounding_box[0] or bounding_box[1] < district_bounding_box[1]):
            if not (bounding_box[2] > district_bounding_box[2] or bounding_box[3] > district_bounding_box[3]):
                considerable_precincts.append(precinct)
    for precinct in considerable_precincts:
        intersection = shapely_district.intersection(precinct.coords).area
        if intersection > 0:
            percent_precinct = intersection/precinct.coords.area
            if percent_precinct < 0:
                print(f'Precinct {precinct.id} has negative area. That\'s extremely bad.')
            for num, party in enumerate(total_party_list):
                exec('votes_list[num] += (percent_precinct * precinct.' + party + ')')
        elif intersection < 0:
            raise Exception(f'Precinct {precinct.id} has negative intersection area with district. How did this happen?????')
    # If total number of "other" votes is negative (possible), round to 0 as long as it's not too bad.
    if votes_list[0] < 0:
        if votes_list[0] > -1:
            votes_list[0] = 0
        else:
            print(f'Problematic number of negative other votes: {votes_list[0]}') 
    return votes_list

def gerry_measures_conversion(districts_file, state_graph):
    """
    Convert given arguments to arguments needed by gerrymandering measure functions.
    """
    with open(state_graph, 'rb') as f:
        state_graph = pickle.load(f)

    # Add node attribute to precincts in graph
    for node in state_graph:
        precinct = state_graph.node_attributes(node)[0]
        precinct.node = node
    if str(districts_file)[-5:] == ".json":
        with open(districts_file, 'r') as f:
            district_geodata = json.load(f)
        district_coords = [feature["geometry"]["coordinates"] for feature in district_geodata["features"]]
        district_ids = [feature["properties"]["District"] for feature in district_geodata["features"]]
        community_list = []
        state = state_graph.node_attributes(list(state_graph.nodes())[0])[0].state
        for num, district in enumerate(district_coords):
            community = Community(district_ids[num], state_graph)
            community.state = state
            community._update_parties()
            community.votes = _find_voter_data(district_coords[num], state_graph, community.parties)
            community_list.append(community)
    else:
        community_list = Community.from_text_file(args[0],  state_graph)
        for community in community_list:
            community.votes = _find_voter_data(shapely_to_geojson(community.coords), state_graph, community.parties)
    return community_list

def efficency_gap(community_list, districts=False):
    """
    Calculates efficency gap, based on
    https://chicagounbound.uchicago.edu/cgi/viewcontent.cgi?article=1946&context=public_law_and_legal_theory

    :param community_list: List of `hacking_the_election.utils.community.Community` objects
    :type community_list: list

    :param districts: Whether indiviual district results should be printed
    :type districts: bool
    """
    total_votes = 0
    wasted_votes = {}
    party_list = [party[8:].capitalize() for party in community_list[0].parties]
    for community in community_list:
        for precinct in community.precincts.values():
            if precinct.total_other < 0:
                precinct.total_other = 0
        party_dict = {party_list[num] : community.votes[num] for num in range(len(party_list))}
        # If first community, fill in wasted_votes
        if community == community_list[0]:
            for party in party_dict:
                wasted_votes[party] = 0 
        community_total_votes = 0
        other_parties = {}
        for party, votes in party_dict.items():
            community_total_votes += votes
            if votes == max(list(party_dict.values())):
                runner_up_dict = deepcopy(party_dict)
                del runner_up_dict[party]
                plurality = ceil(max(list(runner_up_dict.values())))
                majority_party = {party : (votes - plurality)}
                wasted_votes[party] += (votes - plurality)
            else:
                other_parties[party] = votes
                wasted_votes[party] += votes
        community_wasted_votes = list(majority_party.values())[0] + sum(list(other_parties.values()))
        community_efficency_gap = abs(list(majority_party.values())[0] - sum(list(other_parties.values())))/community_total_votes
        if districts:
            print(f'Community/District {community.id}: {int(community_wasted_votes)} Wasted votes, {round(100 * community_efficency_gap, 2)}% efficency gap, {round(100 * list(majority_party.values())[0]/community_total_votes, 2)}% of wasted votes are plurality party, {list(majority_party)[0]}'
            )
            print(f'{list(majority_party)[0]} wasted votes: {list(majority_party.values())[0]}')
            print(f'Other parties: {sum(list(other_parties.values()))}\n')
        total_votes += community_total_votes
    state_efficency_gap = abs((2 * max(list(wasted_votes.values()))) - sum(list(wasted_votes.values())))/total_votes
    print(f'State as whole: {int(sum(list(wasted_votes.values())))} Wasted votes, {round(100 * state_efficency_gap, 2)}% Efficency Gap, {round((100 * max(wasted_votes.values()) / sum(wasted_votes.values())), 2)}% of wasted votes are from {[i for i, votes in wasted_votes.items() if votes == max(wasted_votes.values())][0]}')
    print('State wasted_votes breakdown: ', wasted_votes)
    print('Total votes:', total_votes)
    return state_efficency_gap

def declination(community_list):
    """
    Calculates declination, based on https://arxiv.org/pdf/1803.04799.pdfhttps://arxiv.org/pdf/1803.04799.pdf

    :param community_list: List of `hacking_the_election.utils.community.Community` objects
    :type community_list: list
    """
    ratio_list = []
    for community in community_list:
        dem_votes = community.votes[1]
        rep_votes = community.votes[2]
        total = dem_votes + rep_votes
        ratio_list.append(dem_votes/total)
    rep_won = []
    dem_won = []
    mid_point = []
    for i, ratio in enumerate(ratio_list):
        if ratio == 0.5:
            mid_point = [0.5]
        if ratio < 0.5:
            rep_won.append(ratio)
        if ratio > 0.5:
            dem_won.append(ratio)
    if len(rep_won) == 0 or len(dem_won) == 0:
        print('Declination not applicable')
        return "None"
    rep_won = sorted(rep_won)
    dem_won = sorted(dem_won)
    rep_x = (len(rep_won)/2) - 0.5
    rep_point = [rep_x, sum(rep_won)/len(rep_won)]
    if mid_point == []:
        dem_x = (len(dem_won)/2) - 0.5 + len(rep_won)
        mid_x = len(rep_won) - 0.5
        mid_point = [mid_x, 0.5]
    else:
        dem_x = (len(dem_won)/2) + 0.5 + len(rep_won)
        mid_x = len(rep_won)
        mid_point = (mid_x, 0.5)
    dem_point = [dem_x, sum(dem_won)/len(dem_won)]

    rep_angle = np.arctan((mid_point[1]-rep_point[1])/(mid_point[0]-rep_point[0]))
    dem_angle = np.arctan((dem_point[1]-mid_point[1])/(dem_point[0]-mid_point[0]))
    declination = 2 * (rep_angle - dem_angle) / pi 
    print('\n')
    if declination > 0:
        print(f'Declination: {round(declination, 3)} towards Dems')
    elif declination == 0:
        print('Zero declination.')
    else:
        print(f'Declination: {round(abs(declination), 3)} towards Reps')
    print(f'Misallocated Seats: {round(abs(declination) * len(community_list)/2, 3)}')
    return declination

def votes_to_seats(community_list, districts=False):
    """
    Finds the votes_to_seats graph for a state

    :param community_list: List of `hacking_the_election.utils.community.Community` objects
    :type community_list: list   
     
    :param districts: Whether indiviual district results should be printed
    :type districts: bool  
    """

    for community in community_list:
        total_votes = sum(community.votes)
        community.partisanship = [votes/total_votes for votes in community.votes]
        community.partisanship.pop(0)
        community.partisanship = community.partisanship[:2]
    point_list = [[0,0]]
    community_list = sorted(community_list, key=lambda x: x.partisanship[0], reverse=True)
    for num, community in enumerate(community_list):
        point = []
        point_x = 1 - community.partisanship[0]
        point.append(point_x)
        point_y = (num + 1)/len(community_list)
        point.append(point_y)
        point_list.append([point_x, point_list[-1][1]])
        point_list.append(point)
    point_list.append([1,1])
    intersection_point_list = []
    below = True
    for point in point_list:
        if point[1] <= point[0]:
            if below == False:
                if not point == [point[1], point[1]]:
                    intersection_point_list.append([point[1], point[1]])
                below = True
            intersection_point_list.append(point)
        else:
            if below == True:
                if not point == [point[1], point[1]]:
                    intersection_point_list.append([point[0], point[0]])
            below = False
    intersection_point_list.append([1,0])
    intersection_point_list.pop(0)
    point_list.append([1,0])
    point_list.pop(0)
    intersection_area = area(intersection_point_list)
    map_area = area(point_list)
    overunderlap_area = (0.5 - intersection_area) + (map_area - intersection_area)
    print(f"Votes to Seats overunderlap: {overunderlap_area}")
    return overunderlap_area
    
def mock_election(community_list, districts=False):
    """
    Based on election results, run an election with inputted 
    districts/communities

    :param community_list: List of `hacking_the_election.utils.community.Community` objects
    :type community_list: list

    :param districts: Whether indiviual district results should be printed
    :type districts: bool
    """
    print('')
    if districts:
        print('Simulated Election Results\n')
    party_list = [party[8:].capitalize() for party in community_list[0].parties]
    print('\t\t', end='')
    for party in party_list:
        print('\t', end='')
        print(party, end='')
    if districts:
        print('\t| Winner', end='')
    else:
        print('\t| Result', end='')
    print('\n', end='')
    state_party_votes = [0 for party in party_list]
    state_total_votes = 0
    state_seats_won = {party : 0 for party in party_list}
    for community in community_list:
        total_votes = sum(community.votes)
        percentage_votes = {votes/total_votes : party_list[num] for num, votes in enumerate(community.votes)}
        for num, _ in enumerate(community.votes):
            state_party_votes[num] += community.votes[num]
        state_total_votes += total_votes
        max_percent = max(list(percentage_votes.keys()))
        state_seats_won[percentage_votes[max_percent]] += 1

        if districts:
            print(f'Community/District {community.id}', end='')
            for num, _ in enumerate(party_list):
                print('\t', end='')
                print(round(100 * list(percentage_votes.keys())[num], 3), end='')
                print('%', end='')
            print(' | ', end='')
            print(percentage_votes[max_percent], end='')
            print('')
    state_percentage_votes = {state_party_votes[num]/state_total_votes : party_list[num] for num, _ in enumerate(party_list)}
    print('State overall result', end='')
    for percentage in state_percentage_votes:
        print('\t', end='')
        print(round(100 * percentage, 3), end='')
        print('%', end='')
    print(' | ', end='')
    total_seats = sum(list(state_seats_won.values()))
    for party, seats_won in state_seats_won.items():
        if seats_won > 0:
            print(f'{seats_won} {party.lower()} seat(s) ', end='')
    print('(', end='')
    for seats_won in state_seats_won.values():
        if seats_won == list(state_seats_won.values())[-1]:
            if seats_won > 0:
                print(f'{round(100 * seats_won/total_seats)}%', end='')
        else:    
            if seats_won > 0:
                print(f'{round(100 * seats_won/total_seats)}%-', end='')
    print(' ratio)', end='')
    print('')
    
if __name__ == "__main__":
    args = sys.argv[1:]

    community_list = gerry_measures_conversion(args[0], args[1])

    if len(args) == 4:
        if "-e" in args[2]:
            efficency_gap(community_list, districts=True)
        if "-d" in args[2]:
            declination(community_list)
        if "-vts" in args[2]:
            votes_to_seats(community_list, districts=True)
        if "-m" in args[2]:
            mock_election(community_list, districts=True)
    elif len(args) == 3:
        verbose = False
        if "-e" in args[2]:
            verbose = True
            efficency_gap(community_list)
        if "-d" in args[2]:
            verbose = True
            declination(community_list)
        if "-vts" in args[2]:
            verbose = True
            votes_to_seats(community_list)
        if "-m" in args[2]:
            mock_election(community_list)
        elif verbose == False:
            efficency_gap(community_list, districts=True)
            declination(community_list)
            votes_to_seats(community_list, districts=True)
            mock_election(community_list, districts=True)
    elif len(args) == 2:
        efficency_gap(community_list)
        declination(community_list)
        votes_to_seats(community_list)
        mock_election(community_list)
    else:
        raise Exception('Wrong number of arguments.')
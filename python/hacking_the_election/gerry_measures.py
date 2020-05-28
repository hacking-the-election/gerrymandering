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
python gerry_measures.py [generated communities/districts file.txt] [state.pickle] (-e/-d/-vts/-m) (-d)

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
import numpy as np
from copy import deepcopy
sys.path.append(dirname(dirname(abspath(__file__))))

from hacking_the_election.utils.precinct import Precinct
from hacking_the_election.utils.community import Community


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
    for community in community_list:
        for i, precinct in community.precincts.items():
            if precinct.total_other < 0:
                print('total_other', i)
        party_list = ["total" + party[7:] for party in community.parties]
        party_dict = {}
        for party in party_list:
            party_sum = 0
            for precinct in community.precincts.values():
                party_sum += getattr(precinct, party)
            party_dict[party[6:].capitalize()] = party_sum
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


def declination(community_list):
    """
    Calculates declination, based on https://arxiv.org/pdf/1803.04799.pdfhttps://arxiv.org/pdf/1803.04799.pdf

    :param community_list: List of `hacking_the_election.utils.community.Community` objects
    :type community_list: list
    """
    ratio_list = []
    for community in community_list:
        dem_votes = sum([precinct.total_dem for precinct in community.precincts.values()])
        rep_votes = sum([precinct.total_rep for precinct in community.precincts.values()])
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
    if declination > 0:
        print(f'Declination: {round(declination, 2)} towards Dems')
    elif declination == 0:
        print('Zero declination.')
    else:
        print(f'Declination: {round(abs(declination), 2)} towards Reps')
        

def votes_to_seats(community_list, districts=False):
    """
    Finds the votes_to_seats graph for a state

    :param community_list: List of `hacking_the_election.utils.community.Community` objects
    :type community_list: list   
     
    :param districts: Whether indiviual district results should be printed
    :type districts: bool  
    """

def mock_election(community_list, districts=False):
    """
    Based on election results, run an election with inputted 
    districts/communities

    :param community_list: List of `hacking_the_election.utils.community.Community` objects
    :type community_list: list

    :param districts: Whether indiviual district results should be printed
    :type districts: bool
    """

if __name__ == "__main__":
    args = sys.argv[1:]

    with open(args[1], 'rb') as f:
        state_graph = pickle.load(f)

    # Add node attribute to precincts in graph
    for node in state_graph:
        precinct = state_graph.node_attributes(node)[0]
        precinct.node = node

    community_list = Community.from_text_file(args[0],  state_graph)
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
        if verbose == False:
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
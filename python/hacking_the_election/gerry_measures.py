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


def declination(community_list):
    """
    Calculates declination

    :param community_list: List of `hacking_the_election.utils.community.Community` objects
    :type community_list: list
    """


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

    with open(args[1], 'r') as f:
        state_graph = pickle.load(f)

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
    if len(args) == 3:
        verbose = False
        if "-e" in args[2]:
            verbose = True
            efficency_gap()
        if "-d" in args[2]:
            verbose = True
            declination()
        if "-vts" in args[2]:
            verbose = True
            votes_to_seats()
        if "-m" in args[2]:
            mock_election()
        if verbose == False:
            efficency_gap(districts=True)
            declination()
            votes_to_seats(districts=True)
            mock_election(districts=True)
    elif len(args) == 2:
        efficency_gap()
        declination()
        votes_to_seats()
        mock_election()
    else:
        raise Exception('Wrong number of arguments.')
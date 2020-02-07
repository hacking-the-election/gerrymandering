"""
usage: python3 load_precincts.py [objects_dir] [state]
"""


import pickle

from save_precincts import Precinct


def load(objects_dir, state):
    """
    Return the list of precincts that was saved to a state file
    """
    with open(f'{objects_dir}/{state}.pickle', 'rb') as f:
        state = pickle.load(f)
    return state


if __name__ == "__main__":
    
    import sys

    lst = load(sys.argv[1], state := sys.argv[2].capitalize())
    
    total_pop = 0
    total_dem = 0
    total_rep = 0
    n_missing = 0
    for precinct in lst[0]:
        total_pop += precinct.population
        total_dem += precinct.d_election_sum
        total_rep += precinct.r_election_sum
        if precinct.d_election_sum == -1 and precinct.r_election_sum == -1:
            n_missing += 1
    print(f"{state}:"
          f"Number of Precincts: {len(lst[0])}"
          f"Number of Districts: {len(lst[1]["features"])}"
          f"Population: {total_pop}"
          f"Democratic Votes: {total_dem}"
          f"Republican Votes: {total_rep}"
          f"Number of precincts without election data: {n_missing}")
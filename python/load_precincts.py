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

    lst = load(sys.argv[1], sys.argv[2])
    
    total_pop = 0
    total_dem = 0
    total_rep = 0
    for precinct in lst[0]:
        total_pop += precinct.population
        total_dem += precinct.d_election_sum
        total_rep += precinct.r_election_sum
    print(f"""
# Precincts: {len(lst[0])}
# Districts: {len(lst[1]["features"])}
Population: {total_pop}
Democratic Votes: {total_dem}
Republican Votes: {total_rep}""")
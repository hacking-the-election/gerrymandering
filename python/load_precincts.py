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
    print('\n'.join([str(p) for p in lst]))
    print(len(lst))
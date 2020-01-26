import pickle

from save_precincts import Precinct


def load(objects_dir, state):
    """
    Return the list of precincts that was saved to a state file
    """
    with open(f'{objects_dir}/{state}', 'rb') as f:
        state = pickle.load(f)
    return state


if __name__ == "__main__":
    
    import sys

    print('\n'.join([str(p) for p in load(sys.argv[1], sys.argv[2])]))
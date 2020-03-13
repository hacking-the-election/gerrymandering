"""
Changes a set of communities so that they are within a population range.

Usage:
python3 compactness_refinement.py [initial_configuration] [population_percentage] [json_path] [pickle_path] [animation_dir] [state_name]
"""


import pickle

import matplotlib.pyplot as plt

from hacking_the_election.test.funcs import (
    convert_to_json,
    polygon_to_list
)
from hacking_the_election.utils.exceptions import (
    ExitException
)


def signal_handler(sig, frame):
    raise ExitException


def refine_for_population(communities, population_percentage,
                          linked_precincts, output_json,
                          output_pickle, animation_dir,
                          state_name):
    """
    Returns communities that are within the population range.
    """

    try:
        X = [[] for _ in communities]
        Y = [[] for _ in communities]
    finally:
        with open(output_pickle, "wb+") as f:
            pickle.dump(communities, f)
        convert_to_json(
            [polygon_to_list(c.coords) for c in communities],
            output_json,
            [{"ID": c.id} for c in communities]
        )
        for x, y in zip(X, Y):
            plt.plot(x, y)
        plt.show()
        with open("test_compactness_graph.pickle", "wb+") as f:
            pickle.dump([X, Y], f)


if __name__ == "__main__":
    
    import signal
    import sys

    from hacking_the_election.utils.community import Community
    from hacking_the_election.serialization.save_precincts import Precinct

    signal.signal(signal.SIGINT, signal_handler)

    with open(sys.argv[1], "rb") as f:
        try:
            communities, linked_precinct_chains = pickle.load(f)
        except ModuleNotFoundError:
            from hacking_the_election.serialization import save_precincts
            sys.modules["save_precincts"] = save_precincts
    linked_precincts = {p for chain in linked_precinct_chains for p in chain}

    refine_for_population(communities, float(sys.argv[2]),
                          linked_precincts, *sys.argv[3:])
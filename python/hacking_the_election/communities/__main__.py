"""
The Communities Algorithm.

Usage:
python3 communities [state_data] [n_districts] [state_name] [animation_dir] [output_pickle] [output_json]
"""

from copy import copy
import signal
import sys
import pickle

from hacking_the_election.communities.initial_configuration import (
    create_initial_configuration
)
from hacking_the_election.serialization.save_precincts import Precinct
from hacking_the_election.test.funcs import convert_to_json, polygon_to_list
from hacking_the_election.utils.exceptions import (
    ExitException
)


# Parameters
PARTISANSHIP_STDEV = 8  # Minimum average standard deviation of
                        # partisanship within communities.
POPULATION = 5  # Allowed percent difference from ideal population
COMPACTNESS = 0.4  # Minimum compactness score.


def signal_handler(sig, frame):
    raise ExitException


def make_communities(island_precinct_groups, n_districts, state_name,
                     state_border, animation_dir, output_pickle, output_json):
    """
    Divides a state into ungerrymandered political communities.
    """

    # Start iterative method with random guess.
    initial_configuration, precinct_corridors = create_initial_configuration(
        island_precinct_groups,
        n_districts,
        state_border
    )
    
    # Community "snapshots" at different iterations.
    community_stages = [copy(initial_configuration)]
    # List of precincts that changed each iteration. (ids)
    changed_precincts = []

    # Save output to pickle and json
    with open(output_pickle, "wb+") as f:
        pickle.dump([community_stages, changed_precincts], f)
    convert_to_json(
        [polygon_to_list(c.coords) for c in community_stages[-1]],
        output_json
    )

    return community_stages, changed_precincts


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        island_precinct_groups, _, state_border = pickle.load(f)

    make_communities(island_precinct_groups, int(sys.argv[2]), sys.argv[3],
                     state_border, *sys.argv[4:])
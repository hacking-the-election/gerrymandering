"""
Refines a configuration of political communities in a state so that
the standard deviation of the partisanships of the precincts inside
of each of the communities is below a threshold.

Usage:
python3 -m hacking_the_election.communities.partisanship_refinement <serialized_state> <n_communities> (<animation_dir> | "none") <output_path>
"""

import os
import pickle
import sys

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration 


def optimize_partisanship_stdev(communities, graph, animation_dir=None):
    """Takes a set of communities and exchanges precincts such that the standard deviation of the partisanships of their precincts are as low as possible.

    :param communities: The current state of the communities within a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param graph: A graph containing precinct data within a state.
    :type graph: `pygraph.classes.graph.graph`

    :param animation_dir: Path to the directory where animation files should be saved, defaults to None
    :type animation_dir: str or NoneType
    """

    
if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)

    communities = create_initial_configuration(graph, int(sys.argv[2]))

    animation_dir = None if sys.argv[3] == "none" else sys.argv[3]
    try:
        os.mkdir(animation_dir)
    except FileExistsError:
        pass

    optimize_partisanship_stdev(communities, graph, animation_dir)

    with open(sys.argv[4], "wb+") as f:
        pickle.dump(communities, f)
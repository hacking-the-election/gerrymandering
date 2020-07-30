"""
Implementation of the simulated annealing and gradient ascent for
generating political communities.

Usage:
python3 -m hacking_the_election.communities <serialized_state_graph_path> <n_communities> <output_path> <algorithm> [animation_dir]

<algorithm> can either be "SA" or "GA"
"""

import pickle
import random
import sys

from pygraph.classes.graph import graph as Graph

from hacking_the_election.utils.community import (
    Community,
    create_initial_configuration
)
from hacking_the_election.utils.stats import average
from hacking_the_election.utils.graph import (
    contract,
    get_articulation_points,
    light_copy
)
from hacking_the_election.visualization.misc import draw_state


UPDATE_ATTRIBUTES = {"partisanship_stdev", "imprecise_compactness", "population"}


def _get_score(communities):
    """Returns the goodness score of a list of communities.
    
    :param communities: A list of communities.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :return: A score between 0 and 1 representing the goodness of the inputted communities.
    :rtype: float
    """
    
    return average([c.imprecise_compactness for c in communities])

def _get_all_exchanges(precinct_graph, communities):
    """Finds all the possible ways that a community map can be changed via a single precinct exchanges between two communities.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `pygraph.classes.graph.graph`

    :param communities: A list of the communities that divy up the precincts in precinct_graph
    :type communities: list of `hacking_the_election.utils.community.Community`

    :return: A set of tuples containing a precinct and a community it can be sent to.
    :rtype: set of tuple of `hacking_the_election.utils.precinct.Precinct` and int
    """

    exchanges = set()

    community_dict = {c.id: c for c in communities}
    community_articulation_points = {}
    for community in communities:
        community_articulation_points[community.id] = \
            get_articulation_points(community.induced_subgraph)

    for node in precinct_graph.nodes():

        precinct = precinct_graph.node_attributes(node)[0]
        node_community = precinct.community

        if node in community_articulation_points[node_community]:
            continue

        for neighbor in precinct_graph.neighbors(node):
            neighbor_community = \
                precinct_graph.node_attributes(neighbor)[0].community
            if neighbor_community != node_community:
                exchanges.add((precinct, neighbor_community))
    
    return exchanges


def generate_communities_gradient_ascent(precinct_graph, n_communities, animation_dir=None):
    """Generates a set of political communities using gradient ascent.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `pygraph.classes.graph.graph`

    :param n_communities: The number of communities to break the state into.
    :type n_communities: int

    :param animation_dir: Path to directory which will contain files for animation, defaults to None
    :type animation_dir: str or NoneType

    :return: A list of communities that represent the state.
    :rtype: list of `hacking_the_election.utils.community.Community`
    """

    if animation_dir is not None:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

    communities = create_initial_configuration(precinct_graph, n_communities)
    for community in communities:
        for attribute in UPDATE_ATTRIBUTES:
            eval(f"community.update_{attribute}()")
    community_dict = {community.id: community for community in communities}

    while True:
        best_exchange = None
        can_be_better = False
        largest_score = _get_score(communities)
        exchanges = _get_all_exchanges(precinct_graph, communities)

        for precinct, community_id in exchanges:
            init_community = community_dict[precinct.community]
            other_community = community_dict[community_id]
            if init_community.give_precinct(
                    other_community, precinct.id, UPDATE_ATTRIBUTES):

                score = _get_score(communities)
                if score > largest_score:
                    largest_score = score
                    best_exchange = (init_community, other_community, precinct.id)
                    can_be_better = True
                other_community.give_precinct(
                    init_community, precinct.id, UPDATE_ATTRIBUTES)
        
        if not can_be_better:
            break

        print(largest_score)
        best_exchange[0].give_precinct(
            best_exchange[1], best_exchange[2], UPDATE_ATTRIBUTES)

    return communities


def generate_communities_simulated_annealing(precinct_graph, n_communities, animation_dir=None):
    """Generates a set of political communities using simulated annealing.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `pygraph.classes.graph.graph`

    :param n_communities: The number of communities to break the state into.
    :type n_communities: int

    :param animation_dir: Path to directory which will contain files for animation, defaults to None
    :type animation_dir: str or NoneType

    :return: A list of communities that represent the state.
    :rtype: list of `hacking_the_election.utils.community.Community`
    """

    if animation_dir is not None:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

    communities = create_initial_configuration(precinct_graph, n_communities)

    return communities


if __name__ == "__main__":
    
    # Load input file and generate communities.
    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)

    args = [precinct_graph, int(sys.argv[2])]
    if len(sys.argv) > 5:
        args.append(sys.argv[5])

    if sys.argv[4] == "SA":
        communities = generate_communities_simulated_annealing(*args)
    elif sys.argv[4] == "GA":
        communities = generate_communities_gradient_ascent(*args)

    # draw_state(precinct_graph, None, fpath="test_gradient_ascent.png")

    # Write to file.
    with open(sys.argv[3], "wb+") as f:
        pickle.dump(communities, f)
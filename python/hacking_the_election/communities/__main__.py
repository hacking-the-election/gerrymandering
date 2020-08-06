"""
Implementation of the simulated annealing and greedy algorithms for
generating political communities.

Usage:
python3 -m hacking_the_election.communities <serialized_state_graph_path> <n_communities> <output_path> <algorithm> <image_path> [<text_file>]

<algorithm> can either be "SA" for simulated annealing or "GA" for greedy algorithm.
"""

import math
import pickle
import random
import sys
import time

import networkx as nx
# import numpy as np

from hacking_the_election.utils.community import (
    Community,
    create_initial_configuration
)
from hacking_the_election.utils.geometry import get_distance, get_imprecise_compactness
from hacking_the_election.utils.stats import average
from hacking_the_election.visualization.misc import draw_state


T_MAX = 5
EPOCHS = 40000
ALPHA = 0.99976  # Cooling factor

# C1 = 0.000000002  # Strechiness of sigmoid.
# C2 = 230000000  # Horizontal translation of sigmoid.

# UPDATE_ATTRIBUTES = {"area", "centroid", "population", "partisanship_stdev"}
UPDATE_ATTRIBUTES = {"area", "centroid"}
TEXT_FILE = False
VERBOSE = True

# ------- General private functions ------- #

def _logit(x):
    return - ((math.log(-x / (x - 2)) - 4) / 4)


# def _sigmoid(x):
#     return - (1 / (1 + math.exp(-C1 * (x - C2)))) + 1
# _sigmoid = np.vectorize(_sigmoid)


def _get_score(communities):
    """Returns the goodness score of a list of communities.
    
    :param communities: A list of communities.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :return: A score between 0 and 1 representing the goodness of the inputted communities.
    :rtype: float
    """
    # Population compactness and geometric compactness.
    # compactness_score_sum = 0
    # for community in communities:
    #     distances = []
    #     pop_weights = []

    #     for precinct in community.precincts.values():
    #         distances.append(get_distance(
    #             precinct.centroid, community.centroid)
    #         )
    #         pop_weights.append(precinct.pop / community.population)

    #     normalized_distances = _sigmoid(np.array(distances))
    #     weighted_values = np.multiply(normalized_distances, np.array(pop_weights))
    #     community_score = np.sum(weighted_values)
    #     compactness_score_sum += community_score

    # compactness_score = compactness_score_sum / len(communities)

    # pop_weighted_distances_sum = 0
    # for community in communities:
    #     precincts = community.precincts.values()
        
    #     precinct_distances = {}
    #     max_distance = 0

    #     for precinct in precincts:
    #         precinct_distances[precinct.id] = get_distance(
    #             precinct.centroid, community.centroid
    #         )
    #         if precinct_distances[precinct.id] > max_distance:
    #             max_distance = precinct_distances[precinct.id]
        
    #     for precinct in precincts:
    #         pop_weighted_distances_sum += (
    #             (precinct_distances[precinct.id] / max_distance)
    #           * (precinct.pop / community.population)
    #         )

    # compactness_score = 1 - (pop_weighted_distances_sum / len(communities))

    compactness_score = average([get_imprecise_compactness(c) for c in communities])

    # Population distribution.
    population_score = 1

    state_pop = sum([c.population for c in communities])
    ideal_pop = state_pop / len(communities)
    max_deviation = ideal_pop * 0.1
    deviations = [abs(c.population - ideal_pop) for c in communities]
    if not all([deviation <= max_deviation for deviation in deviations]):
        # If it doesn't fit the constraint, the score is less than one
        # because there is still room for improvement.
        population_score -= sum(deviations) / state_pop

    # Partisanship stdev.
    partisanship_score = 1 - average([c.partisanship_stdev for c in communities])

    # Get weights with logit function.
    weights = [
        _logit(compactness_score),
        _logit(population_score),
        _logit(partisanship_score)
    ]

    # print(compactness_score, weights[0])
    # print(population_score, weights[1])
    # print(partisanship_score, weights[2])

    return average([compactness_score, population_score, partisanship_score], weights=weights)


def _get_all_exchanges(precinct_graph, communities):
    """Finds all the possible ways that a community map can be changed via a single precinct exchanges between two communities.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `networkx.Graph`

    :param communities: A list of the communities that divvy up the precincts in precinct_graph
    :type communities: list of `hacking_the_election.utils.community.Community`

    :return: A set of tuples containing a precinct and a community it can be sent to.
    :rtype: set of tuple of `hacking_the_election.utils.precinct.Precinct` and int, also a dict of articulation points.
    """

    articulation_points = {}
    for c in communities:
        articulation_points[c.id] = \
            set(nx.articulation_points(c.induced_subgraph))

    exchanges = set()
    for node in precinct_graph.nodes:
        neighboring_communities = set()
        precinct = precinct_graph.nodes[node]['precinct']
        if precinct.node in articulation_points[precinct.community]:
            continue
        for neighbor in precinct_graph.neighbors(node):
            neighbor_community = \
                precinct_graph.nodes[neighbor]['precinct'].community
            if (
                        (neighbor_community != precinct.community)
                    and (neighbor_community not in neighboring_communities)):
                neighboring_communities.add(neighbor_community)
                exchanges.add((precinct, neighbor_community))
    
    return exchanges, articulation_points

# ------- simulated annealing-specific private functions ------- #

def _cool(t):
    """Temperature cooling function.
    """
    return t * ALPHA


def _probability(temp, current_energy, new_energy):
    """The probability of doing an exchange.
    """
    if new_energy > current_energy:
        # Always do exchanges that are good.
        return 1
    else:
        return math.exp((new_energy - current_energy) / temp)


def _get_random_exchange(precinct_graph, communities, articulation_points):
    """Finds a random precinct and a community it can be sent to without:
        - Making any communities distcontguous.
        - Creating any 0-precinct communities

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `networkx.Graph`

    :param communities: A list of the communities that divvy up the precincts in precinct_graph
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param articulation_points: The articulation points. Provide them if you already know them and don't want them to be recalculated.
    :type articulation points: dict mapping int to set, or NoneType

    :return: A tuple containing a precinct, the community it can be sent to, and a dict of articulation points by community.
    :rtype: tuple of `hacking_the_election.utils.precinct.Precinct`, int, and dict mapping int to set of int.
    """

    community_dict = {c.id: c for c in communities}

    if articulation_points is None:
        articulation_points = {}
        for c in communities:
            articulation_points[c.id] = \
                set(nx.articulation_points(c.induced_subgraph))

    nodes = precinct_graph.nodes
    for node in random.sample(nodes, len(nodes)):
        precinct = precinct_graph.nodes[node]['precinct']

        articulation_point = precinct.node in articulation_points[precinct.community]
        empty_community = len(community_dict[precinct.community].precincts) == 1
        if articulation_point or empty_community:
            continue

        for neighbor in precinct_graph.neighbors(node):
            neighbor_community = \
                precinct_graph.nodes[neighbor]['precinct'].community
            if neighbor_community != precinct.community:
                return precinct, neighbor_community, articulation_points

# ------- algorithm implementation functions ------- #

def generate_communities_greedy(precinct_graph, n_communities, animation_dir=None):
    """Generates a set of political communities using the greedy algorithm.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `networkx.Graph`

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

    # Create initial configuration and update attributes.
    if TEXT_FILE:
        communities = Community.from_text_file(TEXT_FILE, precinct_graph)
    else:
        communities = create_initial_configuration(precinct_graph, n_communities)
    for community in communities:
        for attribute in UPDATE_ATTRIBUTES:
            eval(f"community.update_{attribute}()")
    community_dict = {community.id: community for community in communities}

    i = 0

    if VERBOSE:
        print("Score      \tIteration")

    scores = []
    largest_score = _get_score(communities)

    start_time = time.time()
    while True:
        i += 1
        scores.append(largest_score)
        best_exchange = None
        exchanges, _ = _get_all_exchanges(precinct_graph, communities)

        for precinct, community_id in exchanges:
            # Communities involved in exchange.
            init_community = community_dict[precinct.community]
            other_community = community_dict[community_id]

            # Give the precinct and recalculate the score.
            init_community.give_precinct(
                other_community, precinct.id, UPDATE_ATTRIBUTES)
            score = _get_score(communities)
            if score > largest_score:
                # Mark this as the best exchange if it gives a score
                # higher than the best so far.
                largest_score = score
                best_exchange = (init_community, other_community, precinct.id)
            # Undo the exchange to try different ones.
            other_community.give_precinct(
                init_community, precinct.id, UPDATE_ATTRIBUTES)

        if VERBOSE:
            sys.stdout.write(f"\r" + "".join([" " for _ in range(20)]))
            sys.stdout.write(f"\r{round(largest_score, 8)}\t{i}")
            sys.stdout.flush()

        if best_exchange is None:
            # We have reached local maximum. There are no exchanges
            # that can immediately improve from this point.
            if VERBOSE:
                print()
            return communities, time.time() - start_time, scores

        # perform best exchange.
        best_exchange[0].give_precinct(
            best_exchange[1], best_exchange[2], UPDATE_ATTRIBUTES)


def generate_communities_simulated_annealing(precinct_graph, n_communities,
        prob, cool, animation_dir=None):
    """Generates a set of political communities using simulated annealing.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `networkx.Graph`

    :param n_communities: The number of communities to break the state into.
    :type n_communities: int

    :param prob: A function that returns the probability of doing an exchange given the temperature, current energy, and new energy.
    :type prob: function

    :param cool: A function that takes the current temperature and returns the temperature for the next epoch.
    :type cool: function

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

    if TEXT_FILE:
        communities = Community.from_text_file(TEXT_FILE, precinct_graph)
    else:
        communities = create_initial_configuration(precinct_graph, n_communities)
    for community in communities:
        for attribute in UPDATE_ATTRIBUTES:
            eval(f"community.update_{attribute}()")
    community_dict = {community.id: community for community in communities}

    current_energy = _get_score(communities)
    temp = T_MAX
    articulation_points = None

    # Save these things in case you don't reach them at the end when the temperature is low.
    best_state = []  # List of lists of precincts.
    highest_energy = 0.0
    last_epoch_change = True

    if VERBOSE:
        print("Score      \tTemperature  \tEpoch")
    
    energies = []

    start_time = time.time()
    for epoch in range(EPOCHS):

        energies.append(current_energy)

        # Choose random exchange.
        precinct, other_community_id, articulation_points = \
            _get_random_exchange(precinct_graph, communities, articulation_points)
        init_community = community_dict[precinct.community]
        other_community = community_dict[other_community_id]

        # Perform chosen exchange.
        init_community.give_precinct(
            other_community, precinct.id, update=UPDATE_ATTRIBUTES)
        # Calculate the new energy after this exchange.
        new_energy = _get_score(communities)

        # Go ahead with exchange with certain probability.
        if (prob(temp, current_energy, new_energy) > random.random()):
            current_energy = new_energy
            last_epoch_change = True
            # Reset articulation points because they need to be recalculated.
            articulation_points = None
            # Save new best state if necessary.
            if current_energy > highest_energy:
                best_state = [list(community.precincts.values()) for community in communities]
                highest_energy = current_energy
        else:
            # Exchange was not chosen by probability function.
            other_community.give_precinct(
                init_community, precinct.id, update=UPDATE_ATTRIBUTES)
            last_epoch_change = False

        if VERBOSE:
            sys.stdout.write(f"\r" + "".join([" " for _ in range(20)]))
            sys.stdout.write(f"\r{round(current_energy, 8)}\t{round(temp, 8)}\t{epoch + 1}")
            sys.stdout.flush()

        temp = cool(temp)

    run_time = time.time() - start_time

    if VERBOSE:
        print()

    # Re-generate the saved best communities.
    communities = [Community(i, precinct_graph) for i in range(n_communities)]
    for i, precincts in enumerate(best_state):
        for precinct in precincts:
            communities[i].take_precinct(precinct)
    return communities, run_time, energies


if __name__ == "__main__":

    import matplotlib.pyplot as plt
    
    # Load input file and generate communities.
    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)
    # Add area and points attributes to speed up imprecise compactness.
    for node in precinct_graph.nodes:
        precinct = precinct_graph.nodes[node]['precinct']
        precinct.area = precinct.coords.area
        precinct.points = [list(coord) for coord in list(precinct.coords.exterior.coords)]

    args = [precinct_graph, int(sys.argv[2])]
    if len(sys.argv) > 6:
        TEXT_FILE = sys.argv[6]

    if sys.argv[4] == "SA":
        communities, run_time, energies = generate_communities_simulated_annealing(
            *args, prob=_probability, cool=_cool)

        # Plot energy over epoch.
        plt.plot(list(range(1, EPOCHS + 1)), energies)
        plt.show()
    elif sys.argv[4] == "GA":
        communities, run_time, scores = generate_communities_greedy(*args)
        # Plot scores over epoch.
        plt.plot(list(range(len(scores))), scores)
        plt.show()
    print(f"RUN TIME: {run_time}")

    if len(sys.argv) > 5:
        draw_state(precinct_graph, None, fpath=sys.argv[5])

    # Write to file.
    with open(sys.argv[3], "wb+") as f:
        pickle.dump(communities, f)
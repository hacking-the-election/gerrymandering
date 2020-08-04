"""
Implementation of the simulated annealing and gradient ascent for
generating political communities.

Usage:
python3 -m hacking_the_election.communities <serialized_state_graph_path> <n_communities> <output_path> <algorithm> [animation_dir]

<algorithm> can either be "SA" or "GA"
grid search:
    - We just have to run gradient ascent a bunch of times.
    - for simulated annealing we must change:
        - starting temp.
        - cooling function.
        - number of epochs.
        - probability function.
    - For simulated annealing, not only do we have to consider how much
        it optimizes the score, but we also must consider how long it takes.
        Will that just be multiplying the two values, together?
"""

import pickle
import random
import sys
import time

import networkx as nx

from hacking_the_election.utils.community import create_initial_configuration
from hacking_the_election.utils.geometry import get_distance
from hacking_the_election.utils.stats import average
from hacking_the_election.visualization.misc import draw_state


UPDATE_ATTRIBUTES = {"population", "partisanship_stdev"}

T_MAX = 30
EPOCHS = 40000
COOL = 0.99976

VERBOSE = True

def _get_score(communities):
    """Returns the goodness score of a list of communities.
    
    :param communities: A list of communities.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :return: A score between 0 and 1 representing the goodness of the inputted communities.
    :rtype: float
    """

    # Population compactness and geometric compactness.
    compactness_score = 0
    for community in communities:
        precinct_populations = {}
        max_population = 0
        for precinct in community.precincts.values():
            precinct_populations[precinct.id] = precinct.pop
            if precinct.pop > max_population:
                max_population = precinct.pop

        for precinct in community.precincts.values():
            distance = get_distance(
                precinct.centroid, community.centroid)
            compactness_score += (distance
                * (precinct_populations[precinct.id] / max_population))
    
    # Population distribution.
    state_pop = sum([c.population for c in communities])
    ideal_pop = state_pop / len(communities)
    population_score = (1
        - sum([abs(c.population - ideal_pop) for c in communities]) / state_pop)

    # Partisanship stdev.
    partisanship_score = average([c.partisanship_stdev for c in communities])

    return average([compactness_score, population_score, partisanship_score])


def _get_all_exchanges(precinct_graph, communities):
    """Finds all the possible ways that a community map can be changed via a single precinct exchanges between two communities.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `networkx.Graph`

    :param communities: A list of the communities that divy up the precincts in precinct_graph
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


def generate_communities_gradient_ascent(precinct_graph, n_communities, animation_dir=None):
    """Generates a set of political communities using gradient ascent.

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

    communities = create_initial_configuration(precinct_graph, n_communities)
    for community in communities:
        for attribute in UPDATE_ATTRIBUTES:
            eval(f"community.update_{attribute}()")
    community_dict = {community.id: community for community in communities}

    while True:
        best_exchange = None
        can_be_better = False
        largest_score = _get_score(communities)
        exchanges, _ = _get_all_exchanges(precinct_graph, communities)

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

        if VERBOSE:
            print(largest_score)
        best_exchange[0].give_precinct(
            best_exchange[1], best_exchange[2], UPDATE_ATTRIBUTES)

    return communities


def generate_communities_simulated_annealing(precinct_graph, n_communities, animation_dir=None):
    """Generates a set of political communities using simulated annealing.

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

    communities = create_initial_configuration(precinct_graph, n_communities)
    community_dict = {c.id: c for c in communities}

    current_energy = _get_score(communities)
    temp = T_MAX

    for epoch in range(EPOCHS):
        
        exchanges, _ = _get_all_exchanges(precinct_graph, communities)
        choice = random.sample(exchanges, 1)[0]
        init_community = None
        other_community = None

        # Choose a random exchange.
        while True:
            choice = random.choice(
                [c for c in exchanges if c is not choice])
            
            init_community = community_dict[choice[0].community]
            other_community = community_dict[choice[1]]

            if init_community.give_precinct(other_community, choice[0].id):
                break
        
        new_energy = _get_score(communities)

        if new_energy < current_energy:
            # Go ahead with exchange because it lowers energy.
            current_energy = new_energy
        
        elif ((temp / T_MAX) > random.random()):
            # Go ahead with exchange because of probability function.
            current_energy = new_energy
        
        else:
            # Don't go ahead with exchange.
            other_community.give_precinct(init_community, choice[0].id)

        if VERBOSE:
            sys.stdout.write(f"\r" + "".join([" " for _ in range(20)]))
            sys.stdout.write(f"\r{round(current_energy, 8)}\t{round(temp, 8)}\t{epoch}")
            sys.stdout.flush()

        temp *= COOL
    
    if VERBOSE:
        print()

    return communities


if __name__ == "__main__":
    
    # Load input file and generate communities.
    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)

    args = [precinct_graph, int(sys.argv[2])]
    if len(sys.argv) > 5:
        args.append(sys.argv[5])

    if sys.argv[4] == 'speed':
        VERBOSE = False
        GA_times = []
        for _ in range(10):
            start_time = time.time()
            generate_communities_gradient_ascent(*args)
            GA_times.append(time.time() - start_time)
        # SA_times = []
        # for _ in range(100):
        #     start_time = time.time()
        #     generate_communities_simulated_annealing(*args)
        #     SA_times.append(time.time() - start_time)
        print("GA Average:", average(GA_times))
        # print("SA Average:", average(SA_times))
    else:
        start_time = time.time()
        if sys.argv[4] == "SA":
            communities = generate_communities_simulated_annealing(*args)
        elif sys.argv[4] == "GA":
            communities = generate_communities_gradient_ascent(*args)
        print(f"RUN TIME: {time.time() - start_time}")

        draw_state(precinct_graph, None, fpath="test_algorithm.png")

        # Write to file.
        with open(sys.argv[3], "wb+") as f:
            pickle.dump(communities, f)
"""
Script for generating and optimizing political communities within a state.

Usage (from the python directory):
python3 -m hacking_the_election.community_generation [path_to_community_list.pickle] [state_name]
"""
import pickle
import time
import random
import sys

import networkx as nx
from networkx.algorithms import community as nx_algorithms
from hacking_the_election.utils.community import Community

T_MAX = 5
EPOCHS = 40000
ALPHA = 0.99976

def _probability(temp, current_energy, new_energy):
    """The probability of doing an exchange.
    """
    if new_energy > current_energy:
        # Always do exchanges that are good.
        return 1
    else:
        return math.exp((new_energy - current_energy) / temp)

def create_block_graph(communities_list):
    block_graph = nx.Graph()
    for community in communities_list:
        block_graph.update(community.graph)
        block_graph.add_edges_from(community.border_edges)
    return block_graph

def border_evaluation(block_graph):
    pass

def update_community_evaluation(scores_dict, *args):
    pass

def _get_random_exchange(communities_list, id_to_community):
    community = random.choice(communities_list)
    while len(community.giveable_blocks) == 0:
        community = random.choice(communities_list)
    block_to_give = random.choice(community.giveable_blocks)
    community_to_give_to = random.choice(community.giveable_blocks[block_to_give])
    return (block_to_give, id_to_community[community_to_give_to])    
    

def _get_random_merge(communities_list, id_to_community):
    community = random.choice(communities_list)
    neighbor_community = id_to_community[random.choice(community.neighbors)]
    return (community, neighbor_community)

def _get_random_split(communities_list, id_to_block):
    community = random.choice(communities_list)
    parition = nx_algorithms.kernighan_lin_bisection(community.graph)
    community_1_blocks = [id_to_block[block_id] for block_id in parition[0]]
    community_2_blocks = [id_to_block[block_id] for block_id in parition[1]]
    community_1 = Community(community.state, 10*community.id+1, community_1_blocks)
    community_2 = Community(community.state, 10*community.id+2, community_1_blocks)
    community_1.initialize_graph()
    community_2.initialize_graph()
    community_1.find_neighbors_and_border()
    community_2.find_neighbors_and_border()
    return community, community_1, community_2


def generate_communities(initial_communities_path, evaluation="community"):
    """
    Given a initial set of communities, optimizes them with simulated annealing. 
    Returns a list of optimized communities. 
    """
    with open(initial_communities_path, "rb") as f:
        communities_list = pickle.load(f)

    communities_num = len(communities_list)
    id_to_community = {community.id : community for community in communities_list}

    block_graph = create_block_graph(communities_list)
    id_to_block = block_graph.nodes()
    if evaluation == "border":
        current_energy = border_evaluation(block_graph)
    if evaluation == "community":
        scores = {}
        for community in communities_list:
            scores[community.id] = community.calculate_score()
    
    temp = T_MAX

    if evaluation == "community":
        current_energy = sum(list(scores.values()))/communities_num

    # TODO: implement saving best state, not just end state
    energies = []
    start_time = time.time()
    for epoch in range(EPOCHS):
        energies.append(current_energy)
        rng = random.random()
        if random < 1/3:
            block_to_give, community_to_give_to = _get_random_block_exchange(communities_list)
            community_to_give_to.blocks.append(block_to_give)
            community_to_take_from = id_to_community[block_to_give.community]
            community_to_take_from.blocks.remove(block_to_give)
            if len(community_to_take_from.blocks) <= 1:
                # Ungive the block
                community_to_give_to.blocks.remove(block_to_give)
                community_to_take_from.blocks.append(block_to_give)
                continue    
            # See below for TODO!
            new_community_1_score = community_to_give_to.calculate_score()
            new_community_2_score = community_to_take_from.calculate_score()
            new_energy = (sum(list(scores.values()))-scores[community_to_give_to]+new_community_2_score-scores[community_to_take_from]+new_community_2_score)/(communities_num)
            if prob(temp, current_energy, new_energy) > random.random():
                # Keep the change!
                if evaluation == "community":
                    score[community_to_give_to.id] = new_community_1_score
                    score[community_to_give_to.id] = new_community_2_score
                    community_to_give_to.update_attributes(block_to_give, id_to_block, taking=True)
                    community_to_take_from.update_attributes(block_to_give, id_to_block, taking=False)
                    current_energy = sum(scores)/communities_num
                if evaluation == "border":
                    current_energy = border_evaluation(block_graph)
            else:
                # Ungive the block
                community_to_give_to.blocks.remove(block_to_give)
                community_to_take_from.blocks.append(block_to_give)
        elif random < 2/3:
            if len(communities) < 5:
                print("we definitely want more communities than that")
                continue
            communities_to_merge = _get_random_merge(communities_list, id_to_community)
            # Note: this is for community optimiziation only! 
            # TODO: add way to measure this for border optimization
            new_community_score = communities_to_merge[0].merge_community(communities_to_merge[1], id_to_block, id_to_community, False)
            new_energy = (sum(list(scores.values()))-new_community_score)/(communities_num-1)
            if prob(temp, current_energy, new_energy) > random.random():
                # Keep the change!
                if evaluation == "community":
                    del scores[communities_to_merge[1].id]
                    communities_list.remove(communities_to_merge[1])
                    communities_num -= 1
                    del id_to_community[communities_to_merge[1].id]
                    score[communities_to_merge[0].id] = new_community_score
                    communities_to_merge[0].merge_community(communities_to_merge[1], id_to_block, id_to_community)
                    current_energy = sum(scores)/communities_num
                if evaluation == "border":
                    current_energy = border_evaluation(block_graph)
        else:
            old_community, new_community_1, new_community_2 = _get_random_split(communities_list)
            # See above for TODO!
            new_community_1_score = new_community_1.calculate_score()
            new_community_2_score = new_community_2.calculate_score()
            new_energy = (sum(list(scores.values()))-scores[old_community]+new_community_1_score+new_community_2_score)/(communities_num+1)
            if prob(temp, current_energy, new_energy) > random.random():
                # Keep the change!
                if evaluation == "community":
                    del scores[old_community.id]
                    scores[new_community_1.id] = new_community_1_score
                    scores[new_community_2.id] = new_community_2_score
                    communities_list.remove(old_community.id)
                    communities_list.append(new_community_1)
                    communities_list.append(new_community_2)
                    communities_num += 1
                    del id_to_community[old_community.id]
                    id_to_community[new_community_1.id] = new_community_1
                    id_to_community[new_community_2.id] = new_community_2

                    current_energy = sum(scores)/communities_num
                if evaluation == "border":
                    current_energy = border_evaluation(block_graph)
        temp *= ALPHA
        print(f"\rEpoch: {epoch}, {round(100*epoch/EPOCHS, 1)}%", end="")
        sys.stdout.flush()
    print("\n", end="")
    return communities_list, energies

if __name__ == "__main__":
    optimized_communities_list, energies = generate_communities(sys.argv[1], evaluation="community")
    with open(sys.argv[2] + "_optimized_community_list", "wb") as f:
        pickle.dump(optimized_communities_list, f)

    visualize_map(optimized_community_list, "docs/images/random_optimized_community_visualization.jpg")

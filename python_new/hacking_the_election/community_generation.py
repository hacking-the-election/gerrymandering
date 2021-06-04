"""
Script for generating and optimizing political communities within a state.

Usage (from the python directory):
python3 -m hacking_the_election.community_generation [path_to_community_list.pickle] [state_name]
"""
import pickle
import time
import random
import sys
import math

random.seed(0)

import networkx as nx
from networkx.algorithms import community as nx_algorithms
from hacking_the_election.utils.community import Community
from hacking_the_election.visualization.community_visualization import visualize_map

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

def _get_random_block_exchange(communities_list, id_to_community):
    community = random.choice(communities_list)
    while len(community.giveable_blocks) == 0:
        community = random.choice(communities_list)
    block_to_give = random.choice(list(community.giveable_blocks.keys()))
    community_to_give_to = random.choice(community.giveable_blocks[block_to_give])
    # print(block_to_give, community_to_give_to)
    # print(community.id)
    # neighbors = set()
    # for choice, arr in community.giveable_blocks.items():
    #     for x in arr:
    #         neighbors.add(x)
    # print(neighbors, community_to_give_to)
    return (block_to_give, id_to_community[community_to_give_to])    
    

def _get_random_merge(communities_list, id_to_community):
    community = random.choice(communities_list)
    neighbor_community = id_to_community[random.choice(community.neighbors)]
    return (community, neighbor_community)

def _get_random_split(communities_list, id_to_block, id_to_community):
    community = random.choice(communities_list)
    partition = nx_algorithms.kernighan_lin_bisection(community.graph)
    community_1_blocks = [id_to_block[block_id] for block_id in partition[0]]
    # print(id_to_block)
    community_2_blocks = [id_to_block[block_id] for block_id in partition[1]]
    max_id = max(list(id_to_community.keys()))
    community_1 = Community(community.state, max_id+1, community_1_blocks)
    community_2 = Community(community.state, max_id+2, community_2_blocks)
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
    id_to_block = nx.get_node_attributes(block_graph, "block")
    blocks_list = list(id_to_block.values())
    if evaluation == "border":
        current_energy = border_evaluation(block_graph)
    if evaluation == "community":
        scores = {}
        for community in communities_list:
            scores[community.id] = community.calculate_score()
    
    temp = T_MAX

    if evaluation == "community":
        current_energy = sum(list(scores.values()))/communities_num
        print(current_energy)
    # TODO: implement saving best state, not just end state
    energies = []
    start_time = time.time()
    for epoch in range(EPOCHS):
        energies.append(current_energy)
        if epoch % 10 == 0:
            visualize_map(communities_list, "docs/images/tests/random_" + str(epoch) + "_optimized_community_visualization.jpg")
        # print(list(scores.keys()))
        rng = random.random()
        if rng < 1/3:
            print("block exchange")
            block_to_give, community_to_give_to = _get_random_block_exchange(communities_list, id_to_community)
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
            new_energy = (sum(list(scores.values()))-scores[community_to_give_to.id]+new_community_2_score-scores[community_to_take_from.id]+new_community_2_score)/(communities_num)
            if _probability(temp, current_energy, new_energy) > random.random():
                # print("probabilities do be added tho!")
                # Keep the change!
                if evaluation == "community":
                    scores[community_to_give_to.id] = new_community_1_score
                    scores[community_to_give_to.id] = new_community_2_score
                    community_to_give_to.update_attributes(block_to_give, id_to_block, taking=True)
                    community_to_take_from.update_attributes(block_to_give, id_to_block, taking=False)
                    current_energy = sum(list(scores.values()))/communities_num
                if evaluation == "border":
                    current_energy = border_evaluation(block_graph)
            else:
                # Ungive the block
                community_to_give_to.blocks.remove(block_to_give)
                community_to_take_from.blocks.append(block_to_give)
        elif rng < 2/3:
            print("community merge")
            if len(communities_list) < 5:
                print("we definitely want more communities than that")
                continue
            communities_to_merge = _get_random_merge(communities_list, id_to_community)
            # print(communities_to_merge[0].id, communities_to_merge[1].id)
            # Note: this is for community optimiziation only! 
            # TODO: add way to measure this for border optimization
            # for block in communities_to_merge[0].blocks:
            #     if block.id == "1000000US500110105004037":
            #         print(block.community, "this is the og, wayyyy before merge")
            new_community_score = communities_to_merge[0].merge_community(communities_to_merge[1], id_to_block, id_to_community, False)
            new_energy = (sum(list(scores.values()))-new_community_score)/(communities_num-1)
            if _probability(temp, current_energy, new_energy) > random.random():
                # print("probabilities do be added tho!")
                # print("THERE IS STUFF!")
                # Keep the change!
                if evaluation == "community":
                    # print("score being removed", communities_to_merge[1].id)
                    # for block in communities_to_merge[0].blocks:
                    #     if block.id == "1000000US500110105004037":
                    #         print(block.community, "this is the og, before merge")
                    communities_to_merge[0].merge_community(communities_to_merge[1], id_to_block, id_to_community)
                    del scores[communities_to_merge[1].id]
                    # print(list(scores.keys()))
                    communities_list.remove(communities_to_merge[1])
                    communities_num -= 1
                    del id_to_community[communities_to_merge[1].id]
                    scores[communities_to_merge[0].id] = new_community_score
                    # for other_community in communities_to_merge[0].neighbors:
                    #     neighbors = set()
                    #     for choice, arr in id_to_community[other_community].giveable_blocks.items():
                    #         for x in arr:
                    #             neighbors.add(x)
                    #     print(neighbors, "here are the possible communities to give")
                    #     print(other_community, id_to_community[other_community].neighbors, "final check")
                    # current_energy = sum(list(scores.values()))/communities_num
                if evaluation == "border":
                    current_energy = border_evaluation(block_graph)
                # print(list(scores.keys()))
        else:
            print("community split")
            old_community, new_community_1, new_community_2 = _get_random_split(communities_list, id_to_block, id_to_community)
            # print(old_community.id, "delete!")
            # for block in new_community_1.blocks:
            #     if block not in old_community.blocks:
            #         print("see this also doesn't work ")
            # for block in new_community_2.blocks:
            #     if block not in old_community.blocks:
            #         print("see this also doesn't work ")
            new_community_1.initialize_graph(id_to_block)
            new_community_2.initialize_graph(id_to_block)
            new_community_1.find_neighbors_and_border(id_to_block)
            new_community_2.find_neighbors_and_border(id_to_block)

            # See above for TODO!
            # print(old_community.id, new_community_1.id, new_community_2.id)
            # print(len(old_community.blocks), len(new_community_1.blocks), len(new_community_2.blocks), "lengths")
            new_community_1_score = new_community_1.calculate_score()
            new_community_2_score = new_community_2.calculate_score()
            new_energy = (sum(list(scores.values()))-scores[old_community.id]+new_community_1_score+new_community_2_score)/(communities_num+1)
            if _probability(temp, current_energy, new_energy) > random.random():
                # print("probabilities do be added tho!")
                # Keep the change!
                if evaluation == "community":
                    del scores[old_community.id]
                    scores[new_community_1.id] = new_community_1_score
                    scores[new_community_2.id] = new_community_2_score
                    communities_list.remove(old_community)
                    communities_list.append(new_community_1)
                    communities_list.append(new_community_2)
                    communities_num += 1
                    del id_to_community[old_community.id]
                    id_to_community[new_community_1.id] = new_community_1
                    id_to_community[new_community_2.id] = new_community_2


                    for block in new_community_1.blocks:
                        block.community = new_community_1.id
                        id_to_block[block.id] = block
                    #     # blocks_t.append(block.id)
                    for block in new_community_2.blocks:
                        block.community = new_community_2.id
                        id_to_block[block.id] = block
                    # for block in old_community.blocks:
                    #     if block.community == old_community.id:
                    #         print('shit, fuckers')
                    new_community_1.find_neighbors_and_border(id_to_block)
                    new_community_2.find_neighbors_and_border(id_to_block)
                    count = 0
                    for block in old_community.blocks:
                        try:
                            _ = id_to_block[block.id]
                        except:
                            if block.id not in list(new_community_1.block_ids.keys()) or block.id not in list(new_community_2.block_ids.keys()):
                                count += 1
                    # print(count, len(old_community.blocks))
                    # for block in new_community_1.blocks:
                    #     _ = id_to_block[block.id]
                    # for block in new_community_2.blocks:
                    #     _ = id_to_block[block.id]
                    # print(new_community_1.neighbors, new_community_2.neighbors)
                    for neighbor in new_community_1.neighbors:
                        if neighbor != old_community.id:
                            id_to_community[neighbor].find_neighbors_and_border(id_to_block, update=True)
                            # if old_community.id in id_to_community[neighbor].neighbors:
                            #     print("ohhh boi")
                    for neighbor in new_community_2.neighbors:
                        if neighbor != old_community.id:
                            id_to_community[neighbor].find_neighbors_and_border(id_to_block, update=True)
                    current_energy = sum(list(scores.values()))/communities_num
                    # for block in new_community_1.blocks:
                    #     if block not in old_community.blocks:
                    #         print("see this also doesn't work ")
                    # for block in new_community_2.blocks:
                    #     if block not in old_community.blocks:
                    #         print("see this also doesn't work ")
                    # for block in list(id_to_block.values()):
                    #     if block.community == old_community.id:
                    #         print("BIGGO PROBLEMS!")
                    # for block in old_community.blocks:
                    #     _ = id_to_block[block.id]
                if evaluation == "border":
                    current_energy = border_evaluation(block_graph)
        temp *= ALPHA
        print(f"\rEpoch: {epoch}, {round(100*epoch/EPOCHS, 1)}% Current Energy: {current_energy} ", end="")
        sys.stdout.flush()
    print("\n", end="")
    return communities_list, energies

if __name__ == "__main__":
    optimized_communities_list, energies = generate_communities(sys.argv[1], evaluation="community")
    with open(sys.argv[2] + "_optimized_community_list", "wb") as f:
        pickle.dump(optimized_communities_list, f)

    visualize_map(optimized_community_list, "docs/images/random_optimized_community_visualization.jpg")

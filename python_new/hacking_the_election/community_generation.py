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

random.seed(1)

import networkx as nx
from networkx.algorithms import community as nx_algorithms
from hacking_the_election.utils.community import Community
from hacking_the_election.initial_generation import random_generation
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

def _contract(G, t):
    new_contracted_nodes = []
    for v in t:
        try:
            v_contracted_nodes = G.nodes[v]['contracted nodes']
            new_contracted_nodes.extend(v_contracted_nodes)
            # new_contracted_nodes += v_contracted_nodes
        except KeyError:
            new_contracted_nodes.append(v)
    new_node_neighbors = list(set(G.neighbors(t[0]))
                            | set(G.neighbors(t[1])))
    new_node_neighbors.remove(t[0]); new_node_neighbors.remove(t[1])
    
    G.remove_edge(*t)
    G.remove_node(t[0]); G.remove_node(t[1])

    nodes = G.nodes
    if len(nodes) != 0:
        new_node = max(nodes) + 1
    else:
        new_node = 0
    G.add_node(new_node)
    for neighbor in new_node_neighbors:
        G.add_edge(new_node, neighbor)
    if t[0] == 64 or t[1] == 64:
        print("ok well this actually happens")
    # print(G.nodes)
    G.nodes[new_node]['contracted nodes'] = new_contracted_nodes
    # print(G.nodes[new_node]['contracted nodes'], new_node)

def karger_stein(block_graph):
    # G = _light_copy(precinct_graph)
    temporary_ids_to_block_ids = {}
    temporary_block_ids_to_ids = {}
    G2 = nx.Graph()
    for i, v in enumerate(block_graph.nodes):
        G2.add_node(i)
        temporary_block_ids_to_ids[v] = i
        temporary_ids_to_block_ids[i] = v
    print(len(block_graph.edges))
    for e in block_graph.edges:
        if not G2.has_edge(temporary_block_ids_to_ids[e[0]], temporary_block_ids_to_ids[e[1]]):
            G2.add_edge(temporary_block_ids_to_ids[e[0]], temporary_block_ids_to_ids[e[1]])
    # print(G2.nodes)
    # print(G2.edges(64))
    while len(G2.nodes) > 2:
        # print(len(G2.nodes))
        attr_lengths = {}  # Links edges to the number of nodes they contain.
        edges = list(set(G2.edges))
        for i in range(min(20, len(edges))):
            e = random.choice(edges)
        # e = random.choice(edges)
            # print(e)
            attr_lengths[e] = (len(G2.nodes[e[0]])
                             + len(G2.nodes[e[1]]))
        # print(e)
        _contract(G2, min(attr_lengths))
    # print(G2.nodes)
    # Create community objects from nodes.
    # communities = [Community(i, precinct_graph) for i in range(n_communities)]
    partitioned_blocks = []
    for i, node in enumerate(G2.nodes):
        # print(G2.nodes[node], node)
        contracted_nodes = G2.nodes[node]['contracted nodes']
        actual_nodes = []
        for node in contracted_nodes:
            actual_nodes.append(temporary_ids_to_block_ids[node])
        # partitioned_blocks.append(G.nodes[node]['contracted nodes'])
        partitioned_blocks.append(actual_nodes)
            

    return partitioned_blocks

def border_evaluation(block_graph):
    pass

def update_community_evaluation(scores_dict, *args):
    pass

def _get_random_block_exchange(communities_list, id_to_community):
    possible_community_list = [community for community in communities_list if len(community.blocks) > 30]
    community = random.choice(possible_community_list)
    # while len(community.giveable_blocks) == 0:
    #     community = random.choice(communities_list)
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
    # print("split created!")
    possible_community_list = [community for community in communities_list if len(community.blocks) > 30]
    if len(possible_community_list) == 0:
        return "no moves", "ok", "boomer"
    else:
        chosen_community = random.choice(possible_community_list)
    # print(chosen_community.neighbors, "old community neighbors")
    # print(chosen_community.block_ids)
    actual_block_ids = chosen_community.block_ids
    # partition = nx_algorithms.kernighan_lin_bisection(community.graph)
    # partition = karger_stein(community.graph)
    # print(community.neighbors, "neighbors of the community")
    # block_list = []
    community_id = max(list(id_to_community.keys()))
    block_ids = dict(chosen_community.block_ids)
    collections_of_blocks = []
    created_communities = []
    keys = list(block_ids.keys())
    # actual_block_ids = chosen_community.block_ids
    # print(random.choice(keys))
    # print(len(block_ids), "when the blocks are ids" )
    while len(block_ids) > 0:
        community_id += 1
        try:
            starting_index = random.choice(list(block_ids.keys()))
        except:
            # print("nooo")
            break
        starting_block = block_ids[starting_index]
        del block_ids[starting_index]
        neighbor_ids = [id for id in starting_block.neighbors if id in block_ids]
        blocks = [starting_block]
        community_population = starting_block.pop
        # print(community_population, community.pop/2)
        # print(chosen_community.pop/2)
        while community_population < chosen_community.pop/2:
            # print(community_population)
            # print(len(blocks))
            if len(blocks) > len(chosen_community.blocks)/2:
                break
            try:
                # Choose neighbor randomly, or go in order?
                # neighbor_id = choice(neighbor_ids)
                neighbor_id = neighbor_ids[0]
            except:
                break
            neighbor_ids.remove(neighbor_id)
            try:
                neighbor_block = block_ids[neighbor_id]
            except:
                continue
            else:
                del block_ids[neighbor_id]
            blocks.append(neighbor_block)
            community_population += neighbor_block.pop
            for neighbor in neighbor_block.neighbors:
                if neighbor in block_ids:
                    neighbor_ids.append(neighbor)
        created_community = Community(chosen_community.state, community_id, blocks)
        for block in blocks:
            block.community = community_id
        # community_id += 1
        created_communities.append(created_community)
    # print(len(chosen_community.blocks))
    # print(created_communities)
        # collections_of_blocks.append(blocks)
    # print(actual_block_ids)
    # print(list(actual_block_ids.keys()))
    # print(keys)
    # for block in created_communities[0].blocks:
        # for neighbor in block.neighbors:
            # print(neighbor)
            # print(chosen_community.block_ids)
            # print(list(chosen_community.block_ids.keys()))
            # if neighbor in list(actual_block_ids.keys()):
            # if neighbor in keys:
                # print(neighbor.community, "therea re neighbors!")
    # ids_to_blocks = {block.id: block for block in block_dict.values()}
    for community in created_communities:
        community.initialize_graph(chosen_community.block_ids)
        # community.initialize_graph(id_to_block)
        community.find_neighbors_and_border(chosen_community.block_ids)
        # community.find_neighbors_and_border(id_to_block)
    id_to_community = {community.id:community for community in created_communities}
    # to_remove = [community for community in created_communities if community.pop < 5000]
    created_communities = sorted(created_communities, key=lambda x : x.pop, reverse=True)
    to_remove = created_communities[2:]
    # print(to_remove)
    for i, community in enumerate(to_remove):
        neighboring_community_id = random.choice(community.neighbors)
        id_to_community[neighboring_community_id].merge_community(community, chosen_community.block_ids, id_to_community)
        created_communities.remove(community)
    # partition = random_generation("", community.state, community.graph, id_to_block, id_to_community)
    for community in created_communities:
        community.initialize_graph(chosen_community.block_ids)
        # community.initialize_graph(id_to_block)
        community.find_neighbors_and_border(chosen_community.block_ids)
    # print(created_communities)
    if "1000000US500019604002000" in list(chosen_community.block_ids.keys()):
        print(chosen_community.block_ids["1000000US500019604002000"].community, "it's missing from other stuff")
    for block in created_communities[0].blocks:
        block.community = created_communities[0].id
        if block.id == "1000000US500019604002000":
            print("ayoooo?", created_communities[0].id)
    for block in created_communities[1].blocks:
        if block.id == "1000000US500019604002000":
            print("ayoooo?", created_communities[1].id)
        block.community = created_communities[1].id
    # community_1_blocks = [id_to_block[block_id] for block_id in partition[0]]
    # community_1_blocks = partition[0]
    # community_2_blocks = partition[1]
    # community_1_blocks = collections_of_blocks[0]
    # community_2_blocks = collections_of_blocks[1]
    # # print(id_to_block)
    # # community_2_blocks = [id_to_block[block_id] for block_id in partition[1]]
    # # print(community_1_blocks)
    # community_1 = Community(community.state, max_id+1, community_1_blocks)
    # community_2 = Community(community.state, max_id+2, community_2_blocks)
    # return community, community_1, community_2
    # try:
    # exce
    # print(len(created_communities), "hello")
    return chosen_community, created_communities[0], created_communities[1]


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
        # try:
        #     print(id_to_block["1000000US500019604002000"].community, "offender's community")
        # except:
        #     pass
        energies.append(current_energy ** 0.25)
        if epoch % 200 == 0 and epoch != 0:
            visualize_map(communities_list, "docs/images/tests/random_" + str(epoch) + "_optimized_community_visualization.jpg")
        print(len(list(scores.keys())))
        print(list(scores.keys()))
        # try:
        #     print(id_to_community[64].neighbors, "id 64 com neighbors")
        # except:
            # pass
        rng = random.random()
        if rng < 1/3:
            print("block exchange")
            block_to_give, community_to_give_to = _get_random_block_exchange(communities_list, id_to_community)
            community_to_give_to.blocks.append(block_to_give)
            community_to_take_from = id_to_community[block_to_give.community]
            # print(community_to_give_to.id, "give to", community_to_take_from.id, "take from")
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
            print(communities_to_merge[0].id, communities_to_merge[1].id)
            # Note: this is for community optimiziation only! 
            # TODO: add way to measure this for border optimization
            # for block in communities_to_merge[0].blocks:
            #     if block.id == "1000000US500110105004037":
            #         print(block.community, "this is the og, wayyyy before merge")
            # print(len(communities_to_merge[0].blocks), len(communities_to_merge[1].blocks))
            new_community_score = communities_to_merge[0].merge_community(communities_to_merge[1], id_to_block, id_to_community, False)
            # print(len(communities_to_merge[0].blocks), len(communities_to_merge[1].blocks))
            new_energy = (sum(list(scores.values()))-new_community_score)/(communities_num-1)
            if _probability(temp, current_energy, new_energy) > random.random():
                # print("probabilities do be added tho!")
                # print("THERE IS STUFF!")
                # Keep the change!
                if evaluation == "community":
                    # print("score being removed", communities_to_merge[1].id)
                    # for block in communities_to_merge[0].blocks:
                    #     if block.id == "1000000US500019604002000":
                    #         print(block.community, "this is the og, before merge")
                    # for block in communities_to_merge[1].blocks:
                    #     if block.id == "1000000US500019604002000":
                    #         print(block.community, "this is the og, before merge, and in the first one")
                    communities_to_merge[0].merge_community(communities_to_merge[1], id_to_block, id_to_community)
                    del scores[communities_to_merge[1].id]
                    # print(list(scores.keys()))
                    communities_list.remove(communities_to_merge[1])
                    communities_num -= 1
                    del id_to_community[communities_to_merge[1].id]
                    scores[communities_to_merge[0].id] = new_community_score
                    current_energy = sum(list(scores.values()))/communities_num
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
            if old_community == "no moves":
                break
            # if "1000000US500019604002000" in list(old_community.block_ids.keys()):
            #     print("WAIT SO A BLOCK IS NOT GETTING ADDED????", old_community.block_ids["1000000US500019604002000"].neighbors)
            # print(old_community.id, "delete!")
            # for block in new_community_1.blocks:
            #     if block.community == old_community.id:
            #         print("see this also doesn't work ")
            # for block in new_community_2.blocks:
            #     if block.community == old_community.id:
            #         print("see this also doesn't work ")
            new_community_1.initialize_graph(id_to_block)
            new_community_2.initialize_graph(id_to_block)
            new_community_1.find_neighbors_and_border(id_to_block)
            new_community_2.find_neighbors_and_border(id_to_block)
            print(old_community.id, len(old_community.graph.edges()))
            print(new_community_1.id, len(new_community_1.graph.edges()))
            print(new_community_2.id, len(new_community_2.graph.edges()))
            print(old_community.id, len(old_community.blocks))
            print(new_community_1.id, len(new_community_1.blocks))
            print(new_community_2.id, len(new_community_2.blocks))
            # See above for TODO!
            # print(old_community.id, new_community_1.id, new_community_2.id)
            # print(len(old_community.blocks), len(new_community_1.blocks), len(new_community_2.blocks), "lengths")
            new_community_1_score = new_community_1.calculate_score()
            new_community_2_score = new_community_2.calculate_score()
            # print(scores[old_community.id])
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
                    new_community_1.initialize_graph(id_to_block)
                    new_community_2.initialize_graph(id_to_block)
                    new_community_1.find_neighbors_and_border(id_to_block)
                    new_community_2.find_neighbors_and_border(id_to_block)
                    # print(new_community_1.neighbors, "new community 1 neighbors")
                    # print(new_community_2.neighbors, "new community 2 neighbors")
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
                        # if "1000000US500019604002000" in list(id_to_community[neighbor].block_ids.keys()):
                        #     print("YOOOOOO THIS IS BAD", neighbor, "new com 1")
                        if neighbor != old_community.id:
                            id_to_community[neighbor].find_neighbors_and_border(id_to_block, update=True)
                            # print(id_to_community[neighbor].neighbors, "neighbor neighbors 1")
                    for neighbor in new_community_2.neighbors:
                        # if "1000000US500019604002000" in list(id_to_community[neighbor].block_ids.keys()):
                        #     print("YOOOOOO THIS IS BAD", neighbor, "new com 2")
                        if neighbor != old_community.id:
                            id_to_community[neighbor].find_neighbors_and_border(id_to_block, update=True)
                            # print(id_to_community[neighbor].neighbors, "neighbor neighbors")
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
            else:
                for block in old_community.blocks:
                    block.community = old_community.id
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

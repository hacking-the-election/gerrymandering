"""
Script for generating and optimizing political communities within a state.

Usage (from the python directory):
python3 -m hacking_the_election.community_generation [path_to_community_list.pickle]
"""
import pickle
import time
import random
import sys
import math
# import heap

random.seed(70)

import networkx as nx
import cProfile
from networkx.algorithms import community as nx_algorithms
from networkx.algorithms import edge_betweenness_centrality
from networkx.algorithms.distance_measures import center
from hacking_the_election.utils.community import Community
from hacking_the_election.utils.split import apartition
# from hacking_the_election.utils.split import test_partition
from hacking_the_election.initial_generation import random_generation
from hacking_the_election.visualization.community_visualization import visualize_map
from hacking_the_election.visualization.graph_visualization import visualize_graph


T_MAX = 1
EPOCHS = 40000
ALPHA = 0.9985
THRESHOLD = 0.225

METRIC = 3

def _probability(temp, current_energy, new_energy):
    """The probability of doing an exchange.
    """
    if new_energy > current_energy:
        # Always do exchanges that are good.
        print(new_energy, current_energy, new_energy-current_energy, "sussy improvements")
        return 1
    else:
        # return temp
        # return 0
        print(new_energy-current_energy, (new_energy-current_energy)/temp, math.exp((new_energy - current_energy) / temp), "prob calcs")
        return math.exp((new_energy - current_energy) / temp)

def create_block_graph(communities_list):
    block_graph = nx.Graph()
    for community in communities_list:
        block_graph.update(community.graph)
        # block_graph.add_edges_from(zip(community.border_edges.keys(),community.border_edges.values()))
        block_graph.add_edges_from(community.border_edges)
    return block_graph

# def _contract(G, edges, edge):
def _contract(G, edge):
    yes = time.time()
    new_contracted_nodes = []
    for v in edge:
        try:
            v_contracted_nodes = G.nodes[v]['contracted nodes']
            new_contracted_nodes.extend(v_contracted_nodes)
            # new_contracted_nodes += v_contracted_nodes
        except KeyError:
            new_contracted_nodes.append(v)
    print(f"{time.time()-yes}, a")
    try:
        _ = set(G.neighbors(edge[0]))
        _ = set(G.neighbors(edge[1]))
    except:
        raise Exception(edge[0], edge[1])
    new_node_neighbors = list(set(G.neighbors(edge[0]))
                            | set(G.neighbors(edge[1])))
    new_node_neighbors.remove(edge[0]); new_node_neighbors.remove(edge[1])
    print(f"{time.time()-yes}, b")
    G.remove_edge(*edge)
    print(f"{time.time()-yes}, I")
    # edges.remove((edge[0],edge[1]))
    print(f"{time.time()-yes}, II")
    new_node_neighbors_0 = list(set(G.neighbors(edge[0])))
    print(f"{time.time()-yes}, III")
    # new_node_neighbors_0.remove(edge[1]); 
    new_node_neighbors_1 = list(set(G.neighbors(edge[1])))
    print(f"{time.time()-yes}, IV")
    # new_node_neighbors_1.remove(edge[0]); 
    # for neighbor in new_node_neighbors_0:
    #     try:
    #         edges.remove((neighbor, edge[0]))
    #     except:
    #         pass
    #         # print("ok 1 m")
    #     try:
    #         edges.remove((edge[0], neighbor))
    #     except:
    #         pass
            # print("ok 2 m")
    # print(f"{time.time()-yes}, V")
    # for neighbor in new_node_neighbors_1:
    #     try:
    #         edges.remove((neighbor, edge[1]))
    #     except:
    #         pass
    #     try:
    #         edges.remove((edge[1], neighbor))
    #     except:
    #         pass
    # print(f"{time.time()-yes}, VI")
    G.remove_node(edge[0]); G.remove_node(edge[1])
    print(f"{time.time()-yes}, c")

    nodes = G.nodes
    if len(nodes) != 0:
        new_node = max(nodes) + 1
    else:
        new_node = 0
    # print(f"{time.time()-yes}, 2")
    G.add_node(new_node)
    for neighbor in new_node_neighbors:
        G.add_edge(new_node, neighbor)
        # edges.append((new_node, neighbor))
        # edges.append((new_node,neighbor))
    # if edge[0] == 64 or edge[1] == 64:
    #     print("ok well this actually happens")
    # print(G.nodes)
    print(f"{time.time()-yes}, d")
    G.nodes[new_node]['contracted nodes'] = new_contracted_nodes
    print(f"{time.time()-yes}, e")
    # print(G.nodes[new_node]['contracted nodes'], new_node)
    # return edges
    # print(f"{time.time()-yes}, 3")

def karger_stein(block_graph):
    k_start_time = time.time()
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
    print(f"Time elapsed: {time.time()-k_start_time}")
    # print(G2.nodes)
    # print(G2.edges(64))
    # i = len(G2.nodes)
    haha = time.time()
    edges = list(set(G2.edges))
    # print(edges)
    i = 0
    sum_of_times = 0
    # while i > 2:
    node_attr = []
    # for node in G2.nodes():

    while len(G2.nodes) > 2:
        ok = time.time()
        print(f"\r {i}   ", end="")
        # print(len(G2.nodes))
        # attr_lengths = {}  # Links edges to the number of nodes they contain.
        # edges = list(set(G2.edges))
        print(f"{time.time()-ok}, 1")
        # for i in range(min(20, len(edges))):
            # e = random.choice(edges)
        # edge = random.choice(G2.edges)
        u, v = random.choice(list(set(G2.edges())))
        print(u, v)
        # print(G2.edges(v, data=True))
        # print(set(G2.edges(v, data=True)))
        # print(list(set(G2.edges(v, data=True))))
        # new_edges = ((u, w, d) for x, w, d in G2.edges(v, data=True) if False or w != u)
        # new_edges = list((u, w, d) for x, w, d in G2.edges(v, data=True) if w != u)
        new_edges = [(u,w) for w in list(set(G2.neighbors(v))) if w != u]
        # for j, edge in enumerate(new_edges):
        #     if edge[1] == u:
        #         new_edges.remove(edge)
        #         break
        # raise Exception(list(new_edges))
        v_data = G2.nodes[v]
        G2.remove_node(v)
        G2.add_edges_from(new_edges)
        if 'contraction' in G2.nodes[u]:
            G2.nodes[u]['contraction'][v] = v_data
        else:
            G2.nodes[u]['contraction'] = {v: v_data}
        # edge = random.choice(edges)
        # e = random.choice(list(G2.edges))
            # print(e)
            # attr_lengths[e] = (len(G2.nodes[e[0]])
                            #  + len(G2.nodes[e[1]]))
        # print(e)
        print(f"{time.time()-ok}, 2")
        # edges = _contract(G2, edges, edge)
        # _contract(G2, edge)
        sum_of_times += time.time()-ok
        print(f"{time.time()-ok}, 3")
        # print(sum_of_times/i)
        i += 1
        print("ok, ", i)

    print(G2.nodes)
    print(len(block_graph.nodes), sum_of_times, sum_of_times/len(block_graph.nodes))
    print(sum_of_times, sum_of_times/len(block_graph.nodes))
    print(f"{(time.time()-haha)/len(block_graph.nodes)}")
        # _contract(G2, min(attr_lengths))
    # print(G2.nodes)
    # Create community objects from nodes.
    # communities = [Community(i, precinct_graph) for i in range(n_communities)]
    print(f"Time elapsed: {time.time()-k_start_time}")
    partitioned_blocks = []
    for i, node in enumerate(G2.nodes):
        print(G2.nodes[node], node)
    for i, node in enumerate(G2.nodes):
        # continue
        contracted_nodes = G2.nodes[node]['contracted nodes']
        actual_nodes = []
        for node in contracted_nodes:
            actual_nodes.append(temporary_ids_to_block_ids[node])
        # partitioned_blocks.append(G.nodes[node]['contracted nodes'])
        partitioned_blocks.append(actual_nodes)
    print(f"Time elapsed: {time.time()-k_start_time}")
            

    return partitioned_blocks

def border_evaluation(block_graph):
    pass

def update_community_evaluation(scores_dict, *args):
    pass

def calculate_edge_difference(block_1, block_2):
    try:
        # political_difference = (stats.stdev([block_1.percent_dem, block_2.percent_dem])+stats.stdev([block_1.percent_rep, block_2.percent_rep])+stats.stdev([block_1.percent_other, block_2.percent_other]))/3
        political_difference = (abs(block_1.percent_dem_votes-block_2.percent_dem_votes)+abs(block_1.percent_rep_votes-block_2.percent_rep_votes)+abs(block_1.percent_other_votes-block_2.percent_other_votes))
    except TypeError:
        political_difference = 0
    # print([block_1.percent_white,block_2.percent_white], [block_1.percent_black, block_2.percent_black], [block_1.percent_hispanic, block_2.percent_hispanic], [block_1.percent_aapi, block_2.percent_aapi], [block_1.percent_aian, block_2.percent_aian], [block_1.percent_other, block_2.percent_other])
    try:
        # racial_difference = ((stats.stdev([block_1.percent_white, block_2.percent_white])+stats.stdev([block_1.percent_black, block_2.percent_black])+stats.stdev([block_1.percent_hispanic, block_2.percent_hispanic])+stats.stdev([block_1.percent_aapi, block_2.percent_aapi])+stats.stdev([block_1.percent_aian, block_2.percent_aian])+stats.stdev([block_1.percent_other, block_2.percent_other]))/6)
        racial_difference = (abs(block_1.percent_white-block_2.percent_white)+abs(block_1.percent_black-block_2.percent_black)+abs(block_1.percent_hispanic-block_2.percent_hispanic)+abs(block_1.percent_aapi-block_2.percent_aapi)+abs(block_1.percent_aian-block_2.percent_aian)+abs(block_1.percent_other-block_2.percent_other))
    except TypeError:
        racial_difference = 0
    # if block_1.density > 1:
        # raise Exception(block_1.id, block_1.pop, block_1.area, block_1.density)
    # if block_2.density > 1:
        # raise Exception(block_2.id, block_2.pop, block_2.area, block_2.density)
    # print(block_1.density, block_2.density, abs(block_1.density-block_2.density)/max(block_1.density, block_2.density))
    try: 
        # density_difference = stats.stdev([block_1.density, block_2.density])
        if block_1.density != 0 or block_2.density != 0:
            density_difference = abs(block_1.density-block_2.density)/(max(block_1.density, block_2.density))
        else:
            density_difference = 0
    except TypeError:
        density_difference = 0
    return (political_difference+racial_difference+density_difference)/3

def _get_random_block_exchange(communities_list, id_to_community):
    # max_blocks = max([len(community.blocks) for community in communities_list])
    # possible_community_list = [community for community in communities_list if len(community.blocks) > max_blocks/2]
    possible_community_list = [community for community in communities_list if len(community.blocks) > 30]
    community_to_take_from = random.choice(possible_community_list)
    # while len(community.giveable_blocks) == 0:
    #     community = random.choice(communities_list)
    # print(community.articulation_points, 'articulation points')
    # community_to_take_from.articulation_points = set(nx.articulation_points(community_to_take_from.graph))
    community_giveable_blocks = [block for block in community_to_take_from.giveable_blocks if block.id not in community_to_take_from.articulation_points]
    # for block in community_to_take_from.giveable_blocks:
    #     print(block.id, block.id in community_to_take_from.articulation_points)
    # print(community_to_take_from.articulation_points)
    try:
        block_to_give = random.choice(list(community_giveable_blocks))
    except IndexError:
        raise Exception(community_to_take_from.giveable_blocks, community_to_take_from.blocks, community_to_take_from.border, community_to_take_from.pop, "there were no giveable blocks here??")
    community_to_give_to = random.choice(community_to_take_from.giveable_blocks[block_to_give])
    # print(block_to_give, community_to_give_to)
    # print(community.id)
    # neighbors = set()
    # for choice, arr in community.giveable_blocks.items():
    #     for x in arr:
    #         neighbors.add(x)
    # print(neighbors, community_to_give_to)
    return (block_to_give, id_to_community[community_to_give_to])    
    

def _get_random_merge(communities_list, id_to_block, id_to_community):
    max_blocks = max([len(community.blocks) for community in communities_list])
    # possible_community_list = [community for community in communities_list if len(community.blocks) < max_blocks/2]
    # community = random.choice(possible_community_list)
    community = random.choice(communities_list)
    community.find_neighbors_and_border(id_to_block)
    neighbor_community = id_to_community[random.choice(community.neighbors)]
    print(community.neighbors)
    return (community, neighbor_community)

def _get_random_split(communities_list, id_to_block, id_to_community):
    graph_start_time = time.time()
    # print("split created!")
    max_blocks = max([len(community.blocks) for community in communities_list])
    possible_community_list = [community for community in communities_list if len(community.blocks) > max_blocks/2]
    # if len(possible_community_list) == 0:
    #     return "no moves", "ok", "boomer"
    # else:
    chosen_community = random.choice(possible_community_list)
    # print(len(chosen_community.graph.nodes))
    # chosen_community.graph.update(id_to_community[1].graph)
    # print(len(chosen_community.graph.nodes))
    # chosen_community.graph.update(id_to_community[3].graph)
    # print(len(chosen_community.graph.nodes))
    # chosen_community.graph.update(id_to_community[10].graph)
    # print(len(chosen_community.graph.nodes))
    # chosen_community.graph.update(id_to_community[11].graph)
    # print(len(chosen_community.graph.nodes))
    # chosen_community.graph.update(id_to_community[8].graph)
    # print(len(chosen_community.graph.nodes))
    # print(id_to_community[8].)
    # for community in communities_list:
    #     if community.id != chosen_community.id:
    #         chosen_community.graph.update(community.graph)
    # print(len(chosen_community.graph.nodes))
    print(chosen_community.id, "community being split")
    # print(chosen_community.neighbors)
    # print(chosen_community.neighbors, "chosen_community neighbors")
    return apartition(chosen_community, id_to_block, id_to_community)
    # print(chosen_community.neighbors, "old community neighbors")
    # print(chosen_community.block_ids)
    # partition = nx_algorithms.kernighan_lin_bisection(chosen_community.graph)

    # cutset = nx.minimum_edge_cut(chosen_community.graph)
    # print(cutset)
    # new_graph = nx.Graph()
    # new_graph.add_nodes_from(chosen_community.graph)
    # new_graph.add_edges_from(chosen_community.graph.edges)
    # new_graph.remove_edges_from(cutset)
    # subgraphs = list(new_graph.subgraph(c) for c in nx.connected_components(new_graph))
    # print(subgraphs)
    # cProfile.run('karger_stein(chosen_community.graph)')
    # partition = karger_stein(chosen_community.graph)
    # raise Exception("sussy!")
    # # community_1_blocks = partition[0]
    # # community_2_blocks = partition[1]
    # community_1_blocks = [id_to_block[block_id] for block_id in partition[0]]
    # community_2_blocks = [id_to_block[block_id] for block_id in partition[1]]
    # max_id = max(id_to_community)
    # community_1 = Community(chosen_community.state, max_id+1, community_1_blocks)
    # community_2 = Community(chosen_community.state, max_id+2, community_2_blocks)
    # return chosen_community, community_1, community_2
    # print(partition)
    # print(community.neighbors, "neighbors of the community")
    # block_list = []

    # starting_1_block, starting_2_block = random.sample(chosen_community.blocks, 2)
    # community_1_blocks = [starting_1_block]
    # community_2_blocks = [starting_2_block]
    # neighbors_1 = [id for id in starting_1_block.neighbors if id_to_block[id].community == chosen_community.id]
    # neighbors_2 = [id for id in starting_2_block.neighbors if id_to_block[id].community == chosen_community.id]
    # i = 0
    # while len(community_1_blocks) + len(community_2_blocks) < len(chosen_community.blocks):
    #     if len(neighbors_1)+len(neighbors_2) == 0:
    #         for block in chosen_community.blocks:
    #             if block not in community_1_blocks and block not in community_2_blocks:
    #                 print(block.id, block.neighbors, "ok!")
    #                 for neighbor in block.neighbors:
    #                     print(neighbor, id_to_block[neighbor].community, id_to_block[neighbor] in community_1_blocks, id_to_block[neighbor] in community_2_blocks)
    #         visualize_graph(
    #             chosen_community.graph,
    #             f'./chosen_community_graph.jpg',
    #             lambda n : chosen_community.graph.nodes[n]['block'].centroid,
    #             # colors=(lambda n c: spanning_tree.nodes[n]['color']),
    #             # sizes=(lambda n : spanning_tree.nodes[n]['size']),
    #             # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
    #             show=False
    #         )
    #         print(len(chosen_community.graph.subgraph(c)) for c in list(nx.connected_components(chosen_community.graph)))
    #         print(len(community_1_blocks),len(community_2_blocks),len(community_1_blocks)+len(community_2_blocks),len(chosen_community.blocks))
    #         raise Exception("SUSSY")
    #     if i % 2 == 0:
    #     # if random.random() < 1/2:
    #         if len(neighbors_1) == 0:
    #             i += 1
    #             continue
    #         # Add to community_1
    #         next_neighbor_id = neighbors_1.pop(0)
    #         next_neighbor_block = id_to_block[next_neighbor_id]
    #         # either the while loop or the line 200 if function needs to go
    #         no_neighbors_left = False
    #         while next_neighbor_block in community_1_blocks or next_neighbor_block in community_2_blocks:
    #             try:
    #                 next_neighbor_id = neighbors_1.pop(0)
    #             except:
    #                 no_neighbors_left = True
    #                 break
    #             next_neighbor_block = id_to_block[next_neighbor_id]
    #         if no_neighbors_left:
    #             # print("there were no n 1")
    #             i += 1
    #             continue
    #         community_1_blocks.append(next_neighbor_block)
    #         for neighbor in next_neighbor_block.neighbors:
    #             if id_to_block[neighbor] in community_1_blocks or id_to_block[neighbor] in community_2_blocks:
    #                 continue
    #             else:
    #                 if id_to_block[neighbor].community == chosen_community.id:
    #                     neighbors_1.append(neighbor)
    #     else:
    #         # print("ok it started")
    #         if len(neighbors_2) == 0:
    #             i += 1
    #             continue
    #         # Add to community_2
    #         next_neighbor_id = neighbors_2.pop(0)
    #         next_neighbor_block = id_to_block[next_neighbor_id]
    #         # either the while loop or the line 200 if function needs to go
    #         no_neighbors_left = False
    #         while next_neighbor_block in community_1_blocks or next_neighbor_block in community_2_blocks:
    #             try:
    #                 next_neighbor_id = neighbors_2.pop(0)
    #             except:
    #                 no_neighbors_left = True
    #                 break
    #             next_neighbor_block = id_to_block[next_neighbor_id]
    #         if no_neighbors_left:
    #             # print("there were no n 2")
    #             i += 1
    #             # if (len(community_1_blocks) + len(community_2_blocks) + 1 == len(chosen_community.blocks)):
    #             #     for block in chosen_community.blocks:
    #             #         if block not in community_2_blocks or block not in community_1_blocks:
    #             #             print(block.id, block.neighbors)
    #             continue
    #             # break
    #         community_2_blocks.append(next_neighbor_block)
    #         for neighbor in next_neighbor_block.neighbors:
    #             if id_to_block[neighbor] in community_1_blocks or id_to_block[neighbor] in community_2_blocks:
    #                 continue
    #             else:
    #                 if id_to_block[neighbor].community == chosen_community.id:
    #                     neighbors_2.append(neighbor)
    #     i += 1
    #     # print(len(community_1_blocks), len(community_2_blocks), len(chosen_community.blocks))
    #     # print(len(neighbors_1), len(neighbors_2))
    #     # print(i, "i")

    # print(f"Time needed 1: {time.time()-graph_start_time}")

    # max_id = max(id_to_community)
    # # print(community_2_blocks)
    # community_1 = Community(chosen_community.state, max_id+1, community_1_blocks)
    # community_2 = Community(chosen_community.state, max_id+2, community_2_blocks)
    # # for block in community_1_blocks:
    # #     block.community = max_id+1
    # # for block in community_2_blocks:
    # #     block.community = max_id+2
    # # community_1.initialize_graph(id_to_block)
    # # # community_1.find_neighbors_and_border(chosen_community.block_ids)
    # # community_1.find_neighbors_and_border(id_to_block)
    # # community_2.initialize_graph(id_to_block)
    # # # community_2.find_neighbors_and_border(chosen_community.block_ids)
    # # community_2.find_neighbors_and_border(id_to_block)

    # print(f"Time needed 2: {time.time()-graph_start_time}")

    # visualize_graph(
    #     chosen_community.graph,
    #     f'./chosen_community_graph.jpg',
    #     lambda n : chosen_community.graph.nodes[n]['block'].centroid,
    #     # colors=(lambda n : spanning_tree.nodes[n]['color']),
    #     # sizes=(lambda n : spanning_tree.nodes[n]['size']),
    #     # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
    #     show=False
    # )
    # # print(list(community_1.graph.nodes)[0])
    # for node in community_1.graph.nodes:
    #     try:
    #         _ = community_1.graph.nodes[node]['block']
    #     except:
    #         print(node)
    #         if node not in community_1.block_ids:
    #             print("wait up, 1")
    # for node in community_2.graph.nodes:
    #     try:
    #         _ = community_2.graph.nodes[node]['block']
    #     except:
    #         print(node)
    #         if node not in community_2.block_ids:
    #             print("wait up, 2")
    # print(community_1.graph.nodes[list(community_1.graph.nodes)[0]])
    # visualize_graph(
    #     community_1.graph,
    #     f'./community_1_graph.jpg',
    #     lambda n : community_1.graph.nodes[n]['block'].centroid,
    #     # colors=(lambda n : spanning_tree.nodes[n]['color']),
    #     # sizes=(lambda n : spanning_tree.nodes[n]['size']),
    #     # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
    #     show=False
    # )
    # visualize_graph(
    #     community_2.graph,
    #     f'./community_2_graph.jpg',
    #     lambda n : community_2.graph.nodes[n]['block'].centroid,
    #     # colors=(lambda n : spanning_tree.nodes[n]['color']),
    #     # sizes=(lambda n : spanning_tree.nodes[n]['size']),
    #     # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
    #     show=False
    # )

    # print(len(chosen_community.blocks))
    # print(len(community_2_blocks), len(community_1_blocks), len(community_2_blocks)+len(community_1_blocks), "blocks")
    # print(chosen_community.id)
    # print(chosen_community.neighbors, "chosen_community neighbors")
    # print(community_1.neighbors)
    # print(community_2.neighbors)
    # return chosen_community, community_1, community_2


    # spanning_tree = nx.Graph()
    # starting_block = random.choice(chosen_community.blocks).id
    # spanning_tree.add_node(starting_block, block=id_to_block[starting_block], color=(0,0,255), size=1)
    # neighbors = {id : starting_block for id in id_to_block[starting_block].neighbors}
    # print(starting_block, "starting block")
    # # first_neighbor = id_to_block[starting_block].neighbors[0]
    # # second_neighbor = id_to_block[starting_block].neighbors[1]
    # total_block_num = len(chosen_community.graph.nodes)
    # while (len(spanning_tree.nodes) < total_block_num):
    #     # print(len(spanning_tree.nodes), total_block_num)
    #     next_neighbor = list(neighbors.keys())[0]
    #     # next_neighbor = random.choice(list(neighbors.keys()))
    #     # print(next_neighbor)
    #     if next_neighbor not in spanning_tree.nodes:
    #         # if next_neighbor == first_neighbor:
    #             # print(neighbors)
    #             # print("oh cool!", first_neighbor, neighbors[first_neighbor])
    #         # if next_neighbor == second_neighbor:
    #             # print(neighbors)
    #             # print("oh cool!", second_neighbor, neighbors[second_neighbor])
    #         # print("haha yes")
    #         spanning_tree.add_node(next_neighbor, block=id_to_block[next_neighbor], color=(255,0,0), size=1)
    #         spanning_tree.add_edge(neighbors[next_neighbor], next_neighbor)
    #         added_neighbors_dict = {neighbor : next_neighbor for neighbor in id_to_block[next_neighbor].neighbors}
    #         # print(neighbors, added_neighbors_dict, "p")
    #         # neighbors = {**neighbors, **added_neighbors_dict}
    #         neighbors = {**added_neighbors_dict, **neighbors}
    #         del neighbors[next_neighbor]
    #     else:
    #         del neighbors[next_neighbor]
    # for edge in chosen_community.graph.edges:
    #     edge[weight] = 1
    for edge in chosen_community.graph.edges():
        chosen_community.graph[edge[0]][edge[1]]['weight'] = random.random()
    # # nx.set_edge_attributes(chosen_community.graph, 'weight', 1)
    spanning_tree = nx.minimum_spanning_tree(chosen_community.graph, weight='weight')
    # edge_to_remove = random.choice(list(spanning_tree.edges))
    print(f"Time needed: {time.time()-graph_start_time}")
    # c = center(spanning_tree)
    # central_node = random.choice(list(c))
    # edge_to_remove = random.choice(list(spanning_tree.edges(central_node)))
    edge_to_remove = random.choice(list(spanning_tree.edges(random.choice(list(set(spanning_tree.nodes))))))
    print(f"Time needed: {time.time()-graph_start_time}")

    # edges_to_centrality = edge_betweenness_centrality(spanning_tree)
    # centrality_to_edges = {}
    # for edge, centrality in edges_to_centrality.items():
        # centrality_to_edges[centrality] = edge
    # edge_to_remove = (starting_block, first_neighbor)
    # edge_to_remove = centrality_to_edges[max(centrality_to_edges.keys())]
    # del edges[edge_to_remove]
    spanning_tree.nodes[edge_to_remove[0]]['color'] = (0,0,255)
    spanning_tree.nodes[edge_to_remove[0]]['size'] = 5
    spanning_tree.nodes[edge_to_remove[1]]['color'] = (0,0,255)
    spanning_tree.nodes[edge_to_remove[1]]['size'] = 5
    visualize_graph(
        spanning_tree,
        f'./spanning_tree_graph.jpg',
        lambda n : spanning_tree.nodes[n]['block'].centroid,
        # colors=(lambda n : spanning_tree.nodes[n]['color']),
        # sizes=(lambda n : spanning_tree.nodes[n]['size']),
        # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
        show=False
    )
    # print(edge_to_remove)
    spanning_tree.remove_edge(edge_to_remove[0], edge_to_remove[1])
    partition = list(spanning_tree.subgraph(c) for c in nx.connected_components(spanning_tree))
    print([p.nodes for p in partition])
    for node in list(partition[0].nodes):
        spanning_tree.nodes[node]['color'] = (255, 0, 0)
    for node in list(partition[1].nodes):
        spanning_tree.nodes[node]['color'] = (0, 255, 0)
    visualize_graph(
        spanning_tree,
        f'./spanning_tree_after_graph.jpg',
        lambda n : spanning_tree.nodes[n]['block'].centroid,
        colors=(lambda n : spanning_tree.nodes[n]['color']),
        # sizes=(lambda n : spanning_tree.nodes[n]['size']),
        # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
        show=False
    )

    # max_id = max(id_to_community)
    # community_1_blocks = [id_to_block[block_id] for block_id in list(partition[0].nodes)]
    # community_2_blocks = [id_to_block[block_id] for block_id in list(partition[1].nodes)]
    # # print(community_2_blocks)
    # for block in community_1_blocks:
    #     block.community = max_id+1
    # for block in community_2_blocks:
    #     block.community = max_id+2
    # community_1 = Community(chosen_community.state, max_id+1, community_1_blocks)
    # community_2 = Community(chosen_community.state, max_id+2, community_2_blocks)
    # community_1.initialize_graph(id_to_block)

    # # community_1.find_neighbors_and_border(chosen_community.block_ids)
    # community_1.find_neighbors_and_border(id_to_block)
    # community_2.initialize_graph(id_to_block)
    # # community_2.find_neighbors_and_border(chosen_community.block_ids)
    # community_2.find_neighbors_and_border(id_to_block)
    # print(len(chosen_community.blocks))
    # print(len(community_2_blocks), len(community_2_blocks)+len(community_1_blocks), "blocks")
    # print(chosen_community.id)
    # print(chosen_community.neighbors, "chosen_community neighbors")
    # print(community_1.neighbors)
    # print(community_2.neighbors)
    # return chosen_community, community_1, community_2

    # actual_block_ids = chosen_community.block_ids
    # community_id = max(list(id_to_community.keys()))
    # block_ids = dict(chosen_community.block_ids)
    # collections_of_blocks = []
    # created_communities = []
    # keys = list(block_ids.keys())
    # # actual_block_ids = chosen_community.block_ids
    # # print(random.choice(keys))
    # # print(len(block_ids), "when the blocks are ids" )
    # while len(block_ids) > 0:
    #     community_id += 1
    #     try:
    #         starting_index = random.choice(list(block_ids.keys()))
    #     except:
    #         # print("nooo")
    #         break
    #     starting_block = block_ids[starting_index]
    #     del block_ids[starting_index]
    #     neighbor_ids = [id for id in starting_block.neighbors if id in block_ids]
    #     blocks = [starting_block]
    #     community_population = starting_block.pop
    #     # print(community_population, community.pop/2)
    #     # print(chosen_community.pop/2)
    #     while community_population < chosen_community.pop/2:
    #         # print(community_population)
    #         # print(len(blocks))
    #         if len(blocks) > len(chosen_community.blocks)/2:
    #             break
    #         try:
    #             # Choose neighbor randomly, or go in order?
    #             # neighbor_id = choice(neighbor_ids)
    #             neighbor_id = neighbor_ids[0]
    #         except:
    #             break
    #         neighbor_ids.remove(neighbor_id)
    #         try:
    #             neighbor_block = block_ids[neighbor_id]
    #         except:
    #             continue
    #         else:
    #             del block_ids[neighbor_id]
    #         blocks.append(neighbor_block)
    #         community_population += neighbor_block.pop
    #         for neighbor in neighbor_block.neighbors:
    #             if neighbor in block_ids:
    #                 neighbor_ids.append(neighbor)
    #     created_community = Community(chosen_community.state, community_id, blocks)
    #     for block in blocks:
    #         block.community = community_id
    #     # community_id += 1
    #     created_communities.append(created_community)
    # # print(len(chosen_community.blocks))
    # # print(created_communities)
    #     # collections_of_blocks.append(blocks)
    # # print(actual_block_ids)
    # # print(list(actual_block_ids.keys()))
    # # print(keys)
    # # for block in created_communities[0].blocks:
    #     # for neighbor in block.neighbors:
    #         # print(neighbor)
    #         # print(chosen_community.block_ids)
    #         # print(list(chosen_community.block_ids.keys()))
    #         # if neighbor in list(actual_block_ids.keys()):
    #         # if neighbor in keys:
    #             # print(neighbor.community, "therea re neighbors!")
    # # ids_to_blocks = {block.id: block for block in block_dict.values()}
    # print(f"Time needed: {time.time()-graph_start_time}")
    # print(len(created_communities))
    # for community in created_communities:
    #     community.initialize_graph(chosen_community.block_ids)
    #     # community.initialize_graph(id_to_block)
    #     community.find_neighbors_and_border(chosen_community.block_ids)
    #     # community.find_neighbors_and_border(id_to_block)
    # print(f"Time needed: {time.time()-graph_start_time}")
    # id_to_community = {community.id:community for community in created_communities}
    # # to_remove = [community for community in created_communities if community.pop < 5000]
    # created_communities = sorted(created_communities, key=lambda x : x.pop, reverse=True)
    # to_remove = created_communities[2:]
    # # print(to_remove)
    # print(len(to_remove))
    # for i, community in enumerate(to_remove):
    #     neighboring_community_id = random.choice(community.neighbors)
    #     id_to_community[neighboring_community_id].merge_community(community, chosen_community.block_ids, id_to_community)
    #     created_communities.remove(community)
    # print(f"Time needed: {time.time()-graph_start_time}")
    # # partition = random_generation("", community.state, community.graph, id_to_block, id_to_community)
    # for community in created_communities:
    #     community.initialize_graph(chosen_community.block_ids)
    #     # community.initialize_graph(id_to_block)
    #     community.find_neighbors_and_border(chosen_community.block_ids)

    print(f"Time needed: {time.time()-graph_start_time}")

    # print(created_communities)
    # if "1000000US500019604002000" in list(chosen_community.block_ids.keys()):
    #     print(chosen_community.block_ids["1000000US500019604002000"].community, "it's missing from other stuff")
    # for block in created_communities[0].blocks:
    #     block.community = created_communities[0].id
    #     if block.id == "1000000US500019604002000":
    #         print("ayoooo?", created_communities[0].id)
    # for block in created_communities[1].blocks:
    #     if block.id == "1000000US500019604002000":
    #         print("ayoooo?", created_communities[1].id)
    #     block.community = created_communities[1].id
    # community_1_blocks = partition[0]
    # community_2_blocks = partition[1]
    # community_2_blocks = list(partition[0].nodes)
    # print(type(community_2_blocks[0]))
    # for block_id in list(partition[0].nodes):
    #     print(block_id)
    #     _ = id_to_block[block_id]
    #     print("ok")
    # community_1_blocks = [id_to_block[block_id] for block_id in list(partition[0].nodes)]
    # community_2_blocks = [id_to_block[block_id] for block_id in list(partition[1].nodes)]
    # print(community_1_blocks)
    # print("hello!")
    # print(partition)
    # # community_1_blocks = collections_of_blocks[0]
    # # community_2_blocks = collections_of_blocks[1]
    # # # print(id_to_block)
    # max_id = max(id_to_community)
    # # community_1_blocks = [id_to_block[block_id] for block_id in partition[0]]
    # # community_2_blocks = [id_to_block[block_id] for block_id in partition[1]]
    # for block in community_1_blocks:
    #     block.community = max_id+1
    # for block in community_2_blocks:
    #     block.community = max_id+2
    # community_1 = Community(chosen_community.state, max_id+1, community_1_blocks)
    # community_2 = Community(chosen_community.state, max_id+2, community_2_blocks)
    # community_1.initialize_graph(chosen_community.block_ids)
    # community_1.find_neighbors_and_border(chosen_community.block_ids)
    # community_2.initialize_graph(id_to_block)
    # community_2.find_neighbors_and_border(chosen_community.block_ids)
    # print(len(chosen_community.blocks))
    # print(len(community_2_blocks), len(community_2_blocks)+len(community_1_blocks), "blocks")
    # print(chosen_community.id)
    # print(chosen_community.neighbors, "chosen_community neighbors")
    # print(community_1.neighbors)
    # print(community_2.neighbors)
    # return chosen_community, community_1, community_2
    # print(len(created_communities), "hello")
    # return chosen_community, created_communities[0], created_communities[1]


def generate_communities(initial_communities_path, evaluation="community"):
    """
    Given a initial set of communities, optimizes them with simulated annealing. 
    Returns a list of optimized communities. 
    """
    very_start_time = time.time()
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
        if METRIC == 0:
            for community in communities_list:
                community.initialize_graph(id_to_block)
                community.find_neighbors_and_border(id_to_block)
                scores[community.id] = community.calculate_score(id_to_block, THRESHOLD)
        elif METRIC == 1:
            for community in communities_list:
                community.initialize_graph(id_to_block)
                community.find_neighbors_and_border(id_to_block)
                ids_to_scores = {}
                for neighbor in community.neighbors:
                    ids_to_scores[neighbor] = community.calculate_border_score(id_to_block, neighbor)
                scores[community.id] = [community.calculate_community_score(), ids_to_scores]
        elif METRIC == 3:
            edges_num = len(block_graph.edges())
            for i, edge in enumerate(block_graph.edges()):
                print(f"\r{i}/{edges_num}, {round(100*i/edges_num, 1)}% edge scores calculated", end="")
                difference = calculate_edge_difference(id_to_block[edge[0]], id_to_block[edge[1]])
                if id_to_block[edge[0]].community == id_to_block[edge[1]].community:
                    scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = THRESHOLD - difference
                    scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = THRESHOLD - difference
                else:
                    scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = difference - THRESHOLD
                    scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = difference - THRESHOLD
            print("")
            # scores[community.id] = community.calculate_political_similarity()*community.calculate_race_similarity()
            # scores[community.id] = community.calculate_graphical_compactness()
    # scores_colors = {}
    # max_value = max(list(scores.values()))
    # for key, value in scores.items():
    #     if value > 0:
    #         scores_colors[key] = (0,round(255*value/max_value),0)
    #     else:
    #         scores_colors[key] = (round(255*value/max_value),0,0)
    # visualize_graph(
    #     block_graph,
    #     './vermont_0_score_graph.png',
    #     coords=lambda n : block_graph.nodes[n]['block'].centroid,
    #     edge_colors=lambda n: scores_colors[n],
    # )
    
    temp = T_MAX

    if evaluation == "community":
        if METRIC == 0:
            print(scores, len(scores))
            current_energy = sum(list(scores.values()))/len(scores)
        elif METRIC == 1:
            # print(scores)
            current_energy = sum([(community[0]+sum(border_energy for id, border_energy in community[1].items()))/(len(community[1])+1) for community in list(scores.values())])/len(scores)
        # print(sum(list(scores.values())))
        elif METRIC == 3:
            current_energy = sum(list(scores.values()))/len(scores)
    print(f"Number of communities: {communities_num}")
    print(f"Initial Energy: {current_energy}")

    energies = []
    temperatures = []
    number_of_communities_list = []
    ear = []
    mar = []
    sar = []
    start_time = time.time()
    merge_num = 0
    attempted_merge_num = 0
    split_num = 0
    attempted_split_num = 0
    exchange_num = 0
    attempted_exchange_num = 0

    iterations_without_movement = 0
    epoch = 1

    best_energy = -1000000
    best_communities = None

    try:
        # for epoch in range(EPOCHS):
        while True:
            epoch += 1
            if iterations_without_movement > 200:
                break
            # try:
            #     print(id_to_block["1000000US500019604002000"].community, "offender's community")
            # except:
            #     pass
            # if epoch % 5000 == 0 and epoch != 0:
                # visualize_map(communities_list, "docs/images/tests/random_" + str(epoch) + "_optimized_community_visualization.jpg")
            # print(list(scores.keys()))
            # print(id_to_community)
            # print(scores[6], "score of community 6")
            # print(id_to_community[6].neighbors, "neighbors of community 6")
            # try:
            #     print(id_to_community[64].neighbors, "id 64 com neighbors")
            # except:
                # pass

            rng = random.random()
            if rng < 1/3:
            # if rng:
                exchange_start_time = time.time()
                print("block exchange", end="")
                block_to_give, community_to_give_to = _get_random_block_exchange(communities_list, id_to_community)
                community_to_take_from = id_to_community[block_to_give.community]
                # print(community_to_take_from.articulation_points, "take_from articulation points!!")
                community_to_give_to.find_neighbors_and_border(id_to_block)
                community_to_take_from.find_neighbors_and_border(id_to_block)
                
                community_to_give_to.blocks.append(block_to_give)
                community_to_take_from.blocks.remove(block_to_give)
                # print(community_to_give_to.id, "give to", community_to_take_from.id, "take from")
                # print(f"1 exchange_time elapsed: {time.time()-exchange_start_time}")
                print(community_to_give_to.id, community_to_take_from.id, "communities exchanged", block_to_give.id, "block given id")
                # print(id_to_block["1000000US500199519001110"].community, id_to_block["1000000US500199513001120"].community, "big pogs")
                if len(community_to_take_from.blocks) <= 1:
                    # Ungive the block
                    community_to_give_to.blocks.remove(block_to_give)
                    community_to_take_from.blocks.append(block_to_give)
                    continue
                

                if METRIC == 0:
                    block_to_give.community = community_to_give_to.id
                    community_to_give_to.find_neighbors_and_border(id_to_block)
                    community_to_take_from.find_neighbors_and_border(id_to_block)
                    # Temporarily extend border edges
                    # community_to_give_to.border_edges.extend([border_edge for border_edge in community_to_take_from.border_edges if id_to_block[border_edge[1]].community != community_to_give_to.id])
                    # # # community_to_take_from.border_edges.extend([border_edge for border_edge in community_to_give_to.border_edges if id_to_block[border_edge[1]].community != community_to_take_from.id])
                    new_community_1_score = community_to_give_to.calculate_score(id_to_block, THRESHOLD)
                    # new_community_1_score = community_to_give_to.calculate_political_similarity()*community_to_give_to.calculate_race_similarity()
                    # new_community_1_score = community_to_give_to.calculate_graphical_compactness()
                    new_community_2_score = community_to_take_from.calculate_score(id_to_block, THRESHOLD)
                    # new_community_2_score = community_to_take_from.calculate_political_similarity()*community_to_take_from.calculate_race_similarity()
                    # new_community_2_score = community_to_take_from.calculate_graphical_compactness()
                    new_energy = (sum(list(scores.values()))-scores[community_to_give_to.id]+new_community_2_score-scores[community_to_take_from.id]+new_community_2_score)/(communities_num)
                    
                    current_energy = sum(list(scores.values()))/communities_num
                elif METRIC == 1:
                    block_to_give.community = community_to_give_to.id
                    community_to_give_to.find_neighbors_and_border(id_to_block)
                    community_to_take_from.find_neighbors_and_border(id_to_block)
                    # ok = time.time()
                    new_community_1_similarity = community_to_give_to.calculate_community_score()
                    # print("1", time.time()-ok)
                    new_community_2_similarity = community_to_take_from.calculate_community_score()
                    # print("2", time.time()-ok)
                    new_border_difference = community_to_give_to.calculate_border_score(id_to_block, community_to_take_from.id)
                    # print("3", time.time()-ok)
                    print(new_community_1.id, new_community_2.id, "communities exchanged")
                    print(new_community_1_similarity, new_community_2_similarity, new_border_difference, "scores be like")
                    new_energy = new_community_1_similarity+new_community_2_similarity+new_border_difference-1

                    current_energy = (scores[community_to_give_to.id][0]+scores[community_to_take_from.id][0]+scores[community_to_give_to.id][1][community_to_take_from.id]-1)
                elif METRIC == 3:
                    current_border_edges = []
                    current_border_score = 0
                    for edge in community_to_give_to.border_edges:
                        if id_to_block[edge[1]].community == community_to_take_from.id:
                            current_border_score += scores[(edge[0], edge[1])]
                            current_border_edges.append(edge)
                    current_border_score /= len(scores)
                    block_to_give.community = community_to_give_to.id
                    community_to_give_to.find_neighbors_and_border(id_to_block)
                    community_to_take_from.find_neighbors_and_border(id_to_block)
                    new_border_edges = []
                    new_border_score = 0
                    for edge in community_to_give_to.border_edges:
                        if id_to_block[edge[1]].community == community_to_take_from.id:
                            new_border_score += scores[(edge[0], edge[1])]
                            new_border_edges.append(edge)
                    new_border_score /= len(scores)
                    
                    new_energy = current_energy -2*new_border_score-2*current_border_score
                # print(f"2 exchange_time elapsed: {time.time()-exchange_start_time}")
                print(current_energy, "current energy", new_energy, "new energy")
                # print((sum(list(scores.values()))+new_community_1_score+new_community_2_score-scores[community_to_give_to.id]-scores[community_to_take_from.id])/(communities_num))

                if _probability(temp, current_energy, new_energy) > random.random():
                    
                    iterations_without_movement = 0
                    # print("probabilities do be added tho!")
                    # Keep the change!
                    if evaluation == "community":
                        if METRIC == 0:
                            # print(new_community_1_score, community_to_take_from.calculate_score(id_to_block, 0))
                            # print(new_community_2_score, community_to_take_from.calculate_score(id_to_block,0))
                            if new_community_1_score != community_to_give_to.calculate_score(id_to_block, THRESHOLD):
                                print(new_community_1_score, community_to_give_to.calculate_score(id_to_block, THRESHOLD))
                                raise Exception("NOOO0")
                            # else:
                                # if new_community_2_score != community_to_take_from.calculate_score(id_to_block, 0):
                                    # raise Exception("pain")
                                # else:
                                    # print("OK THIS WAS CHECKED", new_community_1_score, community_to_give_to.calculate_score(id_to_block, 0), community_to_give_to.id)
                            scores[community_to_give_to.id] = new_community_1_score
                            scores[community_to_take_from.id] = new_community_2_score
                        elif METRIC == 1:
                            scores[community_to_give_to.id][0] = new_community_1_similarity
                            scores[community_to_take_from.id][0] = new_community_2_similarity
                            scores[community_to_give_to.id][1][community_to_take_from.id] = new_border_difference
                            scores[community_to_take_from.id][1][community_to_give_to.id] = new_border_difference
                        elif METRIC == 3:
                            for edge in current_border_edges:
                                scores[(edge[0],edge[1])] = -scores[(edge[0],edge[1])]
                                scores[(edge[1],edge[0])] = -scores[(edge[1],edge[0])]
                            for edge in new_border_edges:
                                scores[(edge[0],edge[1])] = -scores[(edge[0],edge[1])]
                                scores[(edge[1],edge[0])] = -scores[(edge[1],edge[0])]
                            current_energy = new_energy
                        community_to_give_to.take_block(block_to_give, id_to_block)
                        community_to_take_from.give_block(block_to_give, id_to_block)
                        if len([c for c in list(nx.connected_components(community_to_give_to.graph))]) > 1 or len([c for c in list(nx.connected_components(community_to_take_from.graph))]) > 1:
                            print(len([c for c in list(nx.connected_components(community_to_give_to.graph))]), len([c for c in list(nx.connected_components(community_to_take_from.graph))]), community_to_take_from.articulation_points)
                            raise Exception("alright, this is where the baka is sussy, line 997")
                    if evaluation == "border":
                        current_energy = border_evaluation(block_graph)
                    exchange_num += 1
                else:
                    # Ungive the block
                    community_to_give_to.blocks.remove(block_to_give)
                    community_to_take_from.blocks.append(block_to_give)
                    # restore original border edgesF
                    block_to_give.community = community_to_take_from.id
                    community_to_give_to.find_neighbors_and_border(id_to_block)
                    community_to_take_from.find_neighbors_and_border(id_to_block)
                attempted_exchange_num += 1
                print(f"Exchange time elapsed: {time.time()-exchange_start_time}")
            elif rng < 2/3:
                merge_start_time = time.time()
                print("community merge", end="")
                if len(communities_list) < 5:
                    print("we definitely want more communities than that")
                    continue
                community_to_add_to, community_to_delete = _get_random_merge(communities_list, id_to_block, id_to_community)
                original_community_to_add_to_blocks = list(community_to_add_to.blocks)
                print(community_to_add_to.id, community_to_delete.id, "communities being merged")
                # print(len(community_to_add_to.border_edges))
                # print(community_to_add_to.neighbors)
                # print(community_to_delete.blocks[0].community, community_to_add_to.id)
                # print(id_to_block[community_to_add_to.blocks[0].id].community, community_to_delete.id)
                # print(community_to_add_to.id, community_to_delete.id)
                # new_community_score = community_to_add_to.merge_community(community_to_delete, id_to_block, id_to_community, METRIC, THRESHOLD, False)
                community_to_add_to.blocks.extend(community_to_delete.blocks)
                if METRIC == 0:
                    for block in community_to_delete.blocks:
                        block.community = community_to_add_to.id
                    community_to_add_to.find_neighbors_and_border(id_to_block)
                    # print(len(community_to_add_to.border_edges))
                    # print(community_to_add_to.neighbors)
                    # print(community_to_add_to.border_edges)  
                    # # # community_to_add_to.border_edges.extend([border_edge for border_edge in community_to_delete.border_edges if id_to_block[border_edge[1]].community != community_to_add_to.id])
                    new_community_score = community_to_add_to.calculate_score(id_to_block, THRESHOLD)
                    current_energy = sum(list(scores.values()))/communities_num
                    new_energy = (sum(list(scores.values()))+2*new_community_score-scores[community_to_add_to.id]-scores[community_to_delete.id])/(communities_num)
                if METRIC == 1:
                    for block in community_to_delete.blocks:
                        block.community = community_to_add_to.id
                    print(scores[community_to_add_to.id], community_to_add_to.id, community_to_delete.id, "merging ids")
                    current_energy = (scores[community_to_add_to.id][0]+scores[community_to_delete.id][0]+scores[community_to_add_to.id][1][community_to_delete.id]-1)
                    new_energy = 2 * new_community_score
                if METRIC == 3:
                    current_border_edges = []
                    current_border_score = 0
                    for edge in community_to_add_to.border_edges:
                        if id_to_block[edge[1]].community == community_to_delete.id:
                            current_border_score += scores[(edge[0], edge[1])]
                            current_border_edges.append(edge)
                    current_border_score /= len(scores)
                    new_energy = current_energy - 2 * current_border_score
                print(current_energy, "current energy", new_energy, "new energy")
                # print((sum(list(scores.values()))+new_community_score-scores[community_to_add_to.id]-scores[community_to_delete.id])/(communities_num-1))


                if _probability(temp, current_energy, new_energy) > random.random():
                    print(current_border_score, "current border score")                    
                    # for edge in community_to_add_to.border_edges:
                    #     if id_to_block[edge[1]].community == community_to_delete.id:
                    #         print(scores[(edge[0], edge[1])], calculate_edge_difference(id_to_block[edge[0]],id_to_block[edge[1]]))
                    iterations_without_movement = 0
                    # Keep the change!
                    if evaluation == "community":
                        community_to_add_to.merge_community(community_to_delete, id_to_block, id_to_community)
                        # print(len(community_to_add_to.border_edges))
                        # print(community_to_add_to.neighbors)
                        # print(community_to_add_to.border_edges)
                        # print(community_to_add_to.calculate_political_similarity(), "<- polisim", community_to_add_to.calculate_race_similarity(), "<- racesim", community_to_add_to.calculate_graphical_compactness(), "<-- grapsim")
                        communities_list.remove(community_to_delete)
                        communities_num -= 1
                        del id_to_community[community_to_delete.id]
                        if METRIC == 0:
                            if new_community_score != community_to_add_to.calculate_score(id_to_block, THRESHOLD):
                                print(new_community_score, community_to_add_to.calculate_score(id_to_block, THRESHOLD))
                                raise Exception("NOOO1")
                            scores[community_to_add_to.id] = new_community_score
                            del scores[community_to_delete.id]
                        elif METRIC == 1:
                            ids_to_scores = {}
                            for neighbor in community_to_delete.neighbors:
                                ids_to_scores[neighbor] = community_to_add_to.calculate_border_score(id_to_block, neighbor)
                            scores[community_to_add_to.id] = [new_community_score, ids_to_scores]
                            del scores[community_to_delete.id]
                        elif METRIC == 3:
                            for edge in current_border_edges:
                                scores[(edge[0],edge[1])] = -scores[(edge[0],edge[1])]
                                scores[(edge[1],edge[0])] = -scores[(edge[1],edge[0])]
                            current_energy = new_energy
                        merge_num += 1
                    if evaluation == "border":
                        current_energy = border_evaluation(block_graph)
                else:
                    # Reset blocks to original
                    community_to_add_to.blocks = original_community_to_add_to_blocks
                    for block in community_to_delete.blocks:
                        block.community = community_to_delete.id
                    community_to_add_to.find_neighbors_and_border(id_to_block)
                attempted_merge_num += 1
                print(f"Merge time elapsed: {time.time()-merge_start_time}")
            else:
                split_start_time = time.time()
                # graph_start_time = time.time()
                print("community split", end="")
                old_community, new_community_1, new_community_2 = _get_random_split(communities_list, id_to_block, id_to_community)
                print(old_community.id, "community being split", time.time()-split_start_time)
                if old_community == "no moves":
                    break
                for block in new_community_1.blocks:
                    block.community = new_community_1.id
                for block in new_community_2.blocks:
                    block.community = new_community_2.id
                new_community_1.initialize_graph(id_to_block)
                # community_1.find_neighbors_and_border(chosen_community.block_ids)
                new_community_1.find_neighbors_and_border(id_to_block)
                new_community_2.initialize_graph(id_to_block)
                # community_2.find_neighbors_and_border(chosen_community.block_ids)
                new_community_2.find_neighbors_and_border(id_to_block)
                if METRIC == 0:
                    new_community_1_score = new_community_1.calculate_score(id_to_block, THRESHOLD)
                    new_community_2_score = new_community_2.calculate_score(id_to_block, THRESHOLD)
                    current_energy = sum(list(scores.values()))/communities_num
                    new_energy = (sum(list(scores.values()))-2*scores[old_community.id]+new_community_1_score+new_community_2_score)/(communities_num)
                elif METRIC == 1:
                    print(old_community.id)
                    new_community_1_score = new_community_1.calculate_community_score()
                    new_community_2_score = new_community_2.calculate_community_score()
                    new_border_difference = new_community_1.calculate_border_score(id_to_block, new_community_2.id)
                    current_energy = 2*scores[old_community.id][0]
                    new_energy = new_community_1_score+new_community_2_score+new_border_difference-1
                elif METRIC == 3:
                    new_border_score = 0
                    new_border_edges = []
                    for edge in new_community_1.border_edges:
                        if id_to_block[edge[1]].community == new_community_2.id:
                            new_border_score += scores[(edge[0],edge[1])]
                            new_border_edges.append(edge)
                    new_border_score /= len(scores)
                    new_energy = current_energy - 2 *new_border_score
                # new_community_1_score = new_community_1.calculate_political_similarity()*new_community_1.calculate_race_similarity()
                # new_community_1_score = new_community_1.calculate_graphical_compactness()
                # new_community_2_score = new_community_2.calculate_political_similarity()*new_community_2.calculate_race_similarity()
                # new_community_2_score = new_community_2.calculate_graphical_compactness()
                # print(old_community.calculate_political_similarity(), "<- polisim", old_community.calculate_race_similarity(), "<- racesim", old_community.calculate_graphical_compactness(), "<-- grapsim")
                # print(new_community_1.calculate_political_similarity(), "<- polisim", new_community_1.calculate_race_similarity(), "<- racesim", new_community_1.calculate_graphical_compactness(), "<-- grapsim")
                # print(new_community_2.calculate_political_similarity(), "<- polisim", new_community_2.calculate_race_similarity(), "<- racesim", new_community_2.calculate_graphical_compactness(), "<-- grapsim")

                # print(new_community_1_score, new_community_2_score, new_border_difference, scores[old_community.id][0], "ook!")
                # print(scores[old_community.id])
                print(current_energy, "current energy", new_energy, "new energy")
                # print((sum(list(scores.values()))+new_community_1_score+new_community_2_score-scores[old_community.id])/(communities_num+1))
                # print(new_community_1_score, new_community_2_score)
                # print(scores[old_community.id], new_community_1_score+new_community_2_score/2)
                if _probability(temp, current_energy, new_energy) > random.random():

                    iterations_without_movement = 0
                    # print("probabilities do be added tho!")
                    # Keep the change!
                    if evaluation == "community":
                        # for block in new_community_1.blocks:
                        #     block.community = new_community_1.id 
                        #     id_to_block[block.id] = block
                        # #     # blocks_t.append(block.id)
                        # for block in new_community_2.blocks:
                        #     block.community = new_community_2.id
                        #     id_to_block[block.id] = block
                        # # for block in old_community.blocks:
                        # #     if block.community == old_community.id:
                        # #         print('shit, fuckers')
                        # new_community_1.initialize_graph(id_to_block)
                        # new_community_2.initialize_graph(id_to_block)
                        # new_community_1.find_neighbors_and_border(id_to_block)
                        # new_community_2.find_neighbors_and_border(id_to_block)

                        del id_to_community[old_community.id]
                        id_to_community[new_community_1.id] = new_community_1
                        id_to_community[new_community_2.id] = new_community_2
                        for neighbor in new_community_1.neighbors:
                            # if "1000000US500019604002000" in list(id_to_community[neighbor].block_ids.keys()):
                            #     print("YOOOOOO THIS IS BAD", neighbor, "new com 1")
                            if neighbor != old_community.id:
                                id_to_community[neighbor].find_neighbors_and_border(id_to_block)
                                # print(id_to_community[neighbor].neighbors, "neighbor neighbors 1")
                        for neighbor in new_community_2.neighbors:
                            # if "1000000US500019604002000" in list(id_to_community[neighbor].block_ids.keys()):
                            #     print("YOOOOOO THIS IS BAD", neighbor, "new com 2")
                            if neighbor != old_community.id:
                                id_to_community[neighbor].find_neighbors_and_border(id_to_block)

                        if METRIC == 0:
                            if new_community_1_score != new_community_1.calculate_score(id_to_block, THRESHOLD):
                                print(new_community_1_score, new_community_1.calculate_score(id_to_block, THRESHOLD), new_community_1.id)
                                raise Exception("NOOO2")
                            scores[new_community_1.id] = new_community_1_score
                            scores[new_community_2.id] = new_community_2_score
                            del scores[old_community.id]
                        elif METRIC == 1:
                            ids_to_scores = {}
                            for neighbor in new_community_1.neighbors:
                                ids_to_scores[neighbor] = new_community_1.calculate_border_score(id_to_block, neighbor)
                            scores[new_community_1.id] = [new_community_1_score, ids_to_scores]
                            ids_to_scores = {}
                            for neighbor in new_community_2.neighbors:
                                ids_to_scores[neighbor] = new_community_2.calculate_border_score(id_to_block, neighbor)
                            scores[new_community_2.id] = [new_community_2_score, ids_to_scores]
                            del scores[old_community.id]
                        elif METRIC == 3:
                            for edge in new_border_edges:
                                scores[(edge[0],edge[1])] = -scores[(edge[0],edge[1])]
                                scores[(edge[1],edge[0])] = -scores[(edge[1],edge[0])]
                            current_energy = new_energy
                        communities_list.remove(old_community)
                        communities_list.append(new_community_1)
                        communities_list.append(new_community_2)
                        communities_num += 1


                        # print(new_community_1.neighbors, "new community 1 neighbors")
                        # print(new_community_2.neighbors, "new community 2 neighbors")
                        # count = 0
                        # for block in old_community.blocks:
                        #     try:
                        #         _ = id_to_block[block.id]
                        #     except:
                        #         if block.id not in list(new_community_1.block_ids.keys()) or block.id not in list(new_community_2.block_ids.keys()):
                        #             count += 1
                        # print(count, len(old_community.blocks))
                        # for block in new_community_1.blocks:
                        #     _ = id_to_block[block.id]
                        # for block in new_community_2.blocks:
                        #     _ = id_to_block[block.id]
                        # print(new_community_1.neighbors, new_community_2.neighbors)
                                # print(id_to_community[neighbor].neighbors, "neighbor neighbors")
                        # print(f"Now current energy: {current_energy}")
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
                        split_num += 1
                    if evaluation == "border":
                        current_energy = border_evaluation(block_graph)
                else:
                    for block in old_community.blocks:
                        block.community = old_community.id
                attempted_split_num += 1
                print(f"Split time elapsed: {time.time()-split_start_time}")

            iterations_without_movement += 1
            energies.append(100 * current_energy)
            temperatures.append(100 * temp)
            number_of_communities_list.append(len(list(communities_list)))
            ear.append(round(100*exchange_num/max(attempted_exchange_num,1), 1))
            mar.append(round(100*merge_num/max(attempted_merge_num,1), 1))
            sar.append(round(100*split_num/max(attempted_split_num,1), 1))
            if METRIC == 0:
                # current_scores = []
                # _scores = {}
                # for community in communities_list:
                #     yes = community.calculate_score(id_to_block, THRESHOLD)
                #     _scores[community.id] = yes
                #     current_scores.append(yes)
                # for i in list(scores.keys()):
                #     if scores[i] != _scores[i]:
                #         raise Exception(scores[i], _scores[i], i)
                current_energy = sum(list(scores.values()))/len(scores)
                # print(current_energy, sum(current_scores)/len(current_scores))
            elif METRIC == 1:
                current_energy = sum([(community[0]+sum(border_energy for id, border_energy in community[1].items()))/(len(community[1])+1) for community in list(scores.values())])/len(scores)
            # for key, value in scores.items():
            #     if value < 0 and id_to_block[key[0]].community != id_to_block[key[1]].community:
            #         calc = calculate_edge_difference(id_to_block[key[0]],id_to_block[key[1]])
            #         raise Exception("OK THIS IS HELLA SUSS", calc, value)
            #     elif value > 0 and id_to_block[key[0]].community == id_to_block[key[1]].community:
            #         calc = calculate_edge_difference(id_to_block[key[0]],id_to_block[key[1]])
            #         raise Exception("OK THIS IS HELLA SUSS1", calc, value, key[0], key[1])
            # if epoch > 9000 and epoch < 10000:
            #     try:
            #         visualize_graph(id_to_community[6300].graph, "unconnected_graph" + str(epoch)+".png", lambda n : id_to_community[6300].graph.nodes[n]['block'].centroid)
            #     except:
            #         pass
            # for community in communities_list:
                # if len(list(c for c in nx.connected_components(community.graph))) > 1:
                    # raise Exception("OK, THIS IS THE EARLIEST!!!", community.id, len(list(c for c in nx.connected_components(community.graph))))
            print(f"TEMPERATURE: {temp}")
            print(f"\rEpoch: {epoch}, {round(100*epoch/EPOCHS, 1)}% Current Energy: {current_energy} Communities #: {len(list(communities_list))} Ex: {exchange_num}, {round(100*exchange_num/max(attempted_exchange_num,1), 1)}% M: {merge_num}, {round(100*merge_num/max(attempted_merge_num,1), 1)}% S: {split_num} {round(100*split_num/max(attempted_split_num,1), 1)}% ny, 0.225 threshold", end="")

            temp *= ALPHA
            # print(f"\rEpoch: {epoch}, {round(100*epoch/EPOCHS, 1)}% Current Energy: {current_energy} ", end="")
            sys.stdout.flush()
            if current_energy > best_energy:
                best_energy = current_energy
                # best communities list needs to be a copy, not just a reference!
                best_communities = communities_list[:]
                block_list = []
                for community in best_communities:
                    block_list.extend(community.blocks)
            print(len(best_communities))
            print(len(block_list))
            print(len(set(block_list)), "set of blocks")
            print("number of blocks as it is supposed to be", 350828)
        print("\n", end="")
        print(f"{iterations_without_movement} iterations without change in communities.")
        print(f"Best energy found: {best_energy}")
        print(f"Run ended at epoch {epoch}. {round(time.time()-very_start_time)} seconds elapsed for an average of {epoch/(time.time()-very_start_time)} epochs per second.")
        print(len(best_communities))
        print(len(block_list))
        print(len(set(block_list)), "set of blocks")
        return best_communities, energies, temperatures, number_of_communities_list, mar, sar, ear
    except KeyboardInterrupt:
        print("\n", end="")
        print(f"Run ended at epoch {epoch}. {round(time.time()-very_start_time)} seconds elapsed for an average of {epoch/(time.time()-very_start_time)} epochs per second.")
        return None, energies, temperatures, number_of_communities_list, mar, sar, ear
    except Exception as e:
    # except:
        raise Exception(str(e))
        print("\n", end="")
        # if e.__class__.__name__ != "KeyboardInterrupt":
        print(str(e))
        print(f"Run ended at epoch {epoch}. {round(time.time()-very_start_time)} seconds elapsed for an average of {epoch/(time.time()-very_start_time)} epochs per second.")
        return None, energies, temperatures, number_of_communities_list, mar, sar, ear

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise Exception("There are either too few or too many arguments. ")
    file = sys.argv[1]
    try:
        state_name = file[:file.find("community")-1]
    except:
        raise Exception("File must be a _community_list.pickle")
    # print(state_name)
    optimized_communities_list, energies, temperatures, number_of_communities_list, mar, sar, ear = generate_communities(file, evaluation="community")
    print(len(optimized_communities_list))
    block_list = []
    for community in optimized_communities_list:
        block_list.extend(community.blocks)
    print(len(block_list))
    print(len(set(block_list)))
    print("\nSaving metrics to 'community_generation_metrics.txt' ")
    # print("Save metrics?")
    text_to_write = "Energies\n"
    text_to_write += str(energies) + "\n"
    text_to_write += "Temperatures\n"
    text_to_write += str(temperatures) + "\n"
    text_to_write += "Number of Communities\n"
    text_to_write += str(number_of_communities_list) + "\n"
    text_to_write += "Merge Acceptance Rate\n"
    text_to_write += str(mar) + "\n"
    text_to_write += "Split Acceptance Rate\n"
    text_to_write += str(sar) + "\n"
    text_to_write += "Exchange Acceptance Rate\n"
    text_to_write += str(ear) + "\n"
    with open("community_generation_metrics.txt", "w") as f:
        f.write(text_to_write)

    if optimized_communities_list != None:

        print("Saving optimized communities to pickle")
        with open(state_name + "_optimized_community_list.pickle", "wb") as f:
            pickle.dump(optimized_communities_list, f)
        print("Visualizing map")
        visualize_map(optimized_communities_list, "docs/images/optimized_random_community_visualization.jpg")

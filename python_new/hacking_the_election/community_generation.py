"""
Script for generating and optimizing political communities within a state.

Usage (from the python directory):
python3 -m hacking_the_election.community_generation [path_to_community_list.pickle]
"""
import pickle
import time
import random
# import copy
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
from hacking_the_election.utils.split import partition
# from hacking_the_election.utils.split import test_partition
from hacking_the_election.initial_generation import random_generation
from hacking_the_election.visualization.community_visualization import visualize_map
from hacking_the_election.visualization.graph_visualization import visualize_graph

global ALPHA, THRESHOLD, METRIC, temp
T_MAX = 1
EPOCHS = 40000
ALPHA = 0.999
THRESHOLD = 0.5
# THRESHOLD = 1

METRIC = 3

political_differences = [0]*101
racial_differences = [0]*101
density_differences = [0]*101
density_differences1 = [0]*101

def _probability(temp, current_energy, new_energy):
    """The probability of doing an exchange.
    """
    if new_energy > current_energy:
        # Always do exchanges that are good.
        print(new_energy, current_energy, new_energy-current_energy, "sussy improvements")
        return 1
    elif new_energy == current_energy:
        return temp
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
        political_difference = (abs(block_1.percent_dem_votes-block_2.percent_dem_votes)+abs(block_1.percent_rep_votes-block_2.percent_rep_votes)+abs(block_1.percent_other_votes-block_2.percent_other_votes))/2
        # political_differences[math.floor(100*political_difference)] += 1
    except TypeError:
        political_difference = 0
    # print([block_1.percent_white,block_2.percent_white], [block_1.percent_black, block_2.percent_black], [block_1.percent_hispanic, block_2.percent_hispanic], [block_1.percent_aapi, block_2.percent_aapi], [block_1.percent_aian, block_2.percent_aian], [block_1.percent_other, block_2.percent_other])
    try:
        # racial_difference = ((stats.stdev([block_1.percent_white, block_2.percent_white])+stats.stdev([block_1.percent_black, block_2.percent_black])+stats.stdev([block_1.percent_hispanic, block_2.percent_hispanic])+stats.stdev([block_1.percent_aapi, block_2.percent_aapi])+stats.stdev([block_1.percent_aian, block_2.percent_aian])+stats.stdev([block_1.percent_other, block_2.percent_other]))/6)
        racial_difference = (abs(block_1.percent_white-block_2.percent_white)+abs(block_1.percent_black-block_2.percent_black)+abs(block_1.percent_hispanic-block_2.percent_hispanic)+abs(block_1.percent_aapi-block_2.percent_aapi)+abs(block_1.percent_aian-block_2.percent_aian)+abs(block_1.percent_other-block_2.percent_other))/2
        # racial_differences[math.floor(100*racial_difference)] += 1
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
            density_differences1[math.floor(100*density_difference)] += 1
        else:
            density_difference = 0
        # density_differences[math.floor(100*density_difference)] += 1
    except TypeError:
        density_difference = 0
    if (political_difference+racial_difference+density_difference)/3-1/3 == 0:
        print(block_1.percent_white, block_1.percent_black, block_1.percent_hispanic, block_1.percent_aapi, block_1.percent_aian, block_1.percent_other)
        print(block_2.percent_white, block_2.percent_black, block_2.percent_hispanic, block_2.percent_aapi, block_2.percent_aian, block_2.percent_other)
        print(political_difference, racial_difference, density_difference)
    # return (political_difference+racial_difference+density_difference)/3
    return political_difference, racial_difference, density_difference

def _get_random_block_exchange(communities_list, id_to_block, id_to_community):
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
    while len(community_giveable_blocks) == 0:
        community_to_take_from = random.choice(possible_community_list)
        community_giveable_blocks = [block for block in community_to_take_from.giveable_blocks if block.id not in community_to_take_from.articulation_points]

    # try:
    block_to_give = random.choice(list(community_giveable_blocks))
    # except IndexError:
        # visualize_map([community_to_take_from], "sussy_community_visualization.jpg", mode="block_random")
        # print(len([c for c in list(nx.connected_components(community_to_take_from.graph))]), "number of components")
        # print(community_to_take_from.articulation_points)
        # print(community_to_take_from.border)
        # community_to_take_from.find_neighbors_and_border(id_to_block)
        # with open("sussy.pickle", "wb") as f:
        #     pickle.dump(community_to_take_from, f)
        # try:
        #     block_to_give = random.choice(list(community_giveable_blocks))
        # except:
        #     raise Exception(community_to_take_from.giveable_blocks, list(community_to_take_from.block_ids.keys()), [block.id for block in community_to_take_from.border], community_to_take_from.pop, "there were no giveable blocks here??")
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

    # print(id_to_community[8].)
    # for community in communities_list:
    #     if community.id != chosen_community.id:
    #         chosen_community.graph.update(community.graph)
    # print(len(chosen_community.graph.nodes))
    print(chosen_community.id, "community being split")
    # print(chosen_community.neighbors)
    # print(chosen_community.neighbors, "chosen_community neighbors")
    return partition(chosen_community, id_to_block, id_to_community)


def generate_communities(initial_communities_path, evaluation="community"):
    """
    Given a initial set of communities, optimizes them with simulated annealing. 
    Returns a list of optimized communities. 
    """
    very_start_time = time.time()
    with open(initial_communities_path, "rb") as f:
        communities_list = pickle.load(f)
    print("Communities Loaded.")
    state = communities_list[0].state
    communities_num = len(communities_list)
    id_to_community = {community.id : community for community in communities_list}

    block_graph = create_block_graph(communities_list)
    print("Block graph created.")
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
            differences = [0]*101
            # for i, edge in enumerate(block_graph.edges()):
            #     print(f"\r{i}/{edges_num}, {round(100*i/edges_num, 1)}% edge scores calculated", end="")
            #     difference = calculate_edge_difference(id_to_block[edge[0]], id_to_block[edge[1]])
            #     if id_to_block[edge[0]].community == id_to_block[edge[1]].community:
            #         scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = THRESHOLD - difference
            #         scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = THRESHOLD - difference
            #     else:
            #         scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = difference - THRESHOLD
            #         scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = difference - THRESHOLD

            political_differences_dict = {}
            racial_differences_dict = {}
            density_differences_dict = {}
            differences_by_edge = {}
            for i, edge in enumerate(block_graph.edges()):
                print(f"\r{i}/{edges_num}, {round(100*i/edges_num, 1)}% edge scores calculated", end="")
                if id_to_block[edge[0]].pop == 0 or id_to_block[edge[1]].pop == 0:
                    scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = 0
                    scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = 0
                    continue
                political_difference, racial_difference, density_difference = calculate_edge_difference(id_to_block[edge[0]], id_to_block[edge[1]])
                try:
                    political_differences_dict[political_difference].append(edge)
                except:
                    political_differences_dict[political_difference] = [edge]
                try:
                    racial_differences_dict[racial_difference].append(edge)
                except:
                    racial_differences_dict[racial_difference] = [edge]
                try:
                    density_differences_dict[density_difference].append(edge)
                except:
                    density_differences_dict[density_difference] = [edge]
                differences_by_edge[edge] = []
            print(len(political_differences_dict))
            print(len(racial_differences_dict))
            print(len(density_differences_dict))

            increment = 1/(len(political_differences_dict)+1)
            for i, difference in enumerate(sorted(political_differences_dict)):
                political_differences[math.floor(100*(i+1)*increment)] += 1
                for edge in political_differences_dict[difference]:
                    differences_by_edge[edge].append((i+1)*increment)
            increment = 1/(len(racial_differences_dict)+1)
            for i, difference in enumerate(sorted(racial_differences_dict)):
                racial_differences[math.floor(100*(i+1)*increment)] += 1
                for edge in racial_differences_dict[difference]:
                    differences_by_edge[edge].append((i+1)*increment)
            increment = 1/(len(density_differences_dict)+1)
            for i, difference in enumerate(sorted(density_differences_dict)):
                density_differences[math.floor(100*(i+1)*increment)] += 1
                for edge in density_differences_dict[difference]:
                    differences_by_edge[edge].append((i+1)*increment)

            for edge in differences_by_edge:
                difference = sum(differences_by_edge[edge])/3
                try:
                    differences[math.floor(100*difference)] += 1
                except:
                    print(difference, differences_by_edge[edge])
                if id_to_block[edge[0]].community == id_to_block[edge[1]].community:
                    scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = THRESHOLD - difference
                    scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = THRESHOLD - difference
                else:
                    scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = difference - THRESHOLD
                    scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = difference - THRESHOLD

            # differences_dict = {}
            # differences_by_edge = {}
            # for i, edge in enumerate(block_graph.edges()):
            #     print(f"\r{i}/{edges_num}, {round(100*i/edges_num, 1)}% edge scores calculated", end="")
            #     if id_to_block[edge[0]].pop == 0 or id_to_block[edge[1]].pop == 0:
            #         scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = 0
            #         scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = 0
            #         continue
            #     difference = calculate_edge_difference(id_to_block[edge[0]], id_to_block[edge[1]])
            #     try:
            #         differences_dict[difference].append(edge)
            #     except:
            #         differences_dict[difference] = [edge]
            # increment = 1/(len(differences_dict)+1)
            # for i, difference in enumerate(sorted(differences_dict)):
            #     for edge in differences_dict[difference]:
            #         differences_by_edge[edge] = (i+1)*increment
            # for edge in differences_by_edge:
            #     difference = differences_by_edge[edge]
            #     try:
            #         differences[math.floor(100*difference)] += 1
            #     except:
            #         print(difference, differences_by_edge[edge])
            #     if id_to_block[edge[0]].community == id_to_block[edge[1]].community:
            #         scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = THRESHOLD - difference
            #         scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = THRESHOLD - difference
            #     else:
            #         scores[(id_to_block[edge[1]].id,id_to_block[edge[0]].id)] = difference - THRESHOLD
            #         scores[(id_to_block[edge[0]].id,id_to_block[edge[1]].id)] = difference - THRESHOLD

            print("")
            # print(len(political_differences_dict))
            # print(len(racial_differences_dict))
            # print(len(density_differences_dict))
            with open("differences.txt", "w") as f:
                f.write(str(differences))
            # with open("differences_political.txt", "w") as f:
            #     f.write(str(political_differences))
            # with open("differences_racial.txt", "w") as f:
            #     f.write(str(racial_differences))
            # with open("differences_density.txt", "w") as f:
            #     f.write(str(density_differences))
            # with open("differences_density1.txt", "w") as f:
            #     f.write(str(density_differences1))
            # scores[community.id] = community.calculate_political_similarity()*community.calculate_race_similarity()
            # scores[community.id] = community.calculate_graphical_compactness()
    del political_differences_dict
    del racial_differences_dict
    del density_differences_dict
    # print("stuff happened 2.8")
    # scores_colors = {}
    # print("stuff happened 2.81")
    # max_value = max(list(scores.values()))
    # print("stuff happened 2.82")
    # # print("s")
    # for i, key in enumerate(scores):
    #     value = scores[key]
    #     print(key, value, max_value, i)
    #     if value > 0:
    #         scores_colors[key] = (0,round(255*value/max_value),0)
    #     else:
    #         scores_colors[key] = (round(255*value/max_value),0,0)
    # print("stuffhappened3")
    # visualize_graph(
    #     block_graph,
    #     './new_york_0_score_graph.png',
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
            score_sum = sum([abs(value) for value in scores.values()])
            current_energy = sum(list(scores.values()))/score_sum
            # current_energy = sum(list(scores.values()))/len(scores)

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

    while True:
        try:
            epoch += 1
            if iterations_without_movement > 300:
                break
            # try:
            #     print(id_to_block["1000000US500019604002000"].community, "offender's community")
            # except:
            #     pass
            # if epoch % 2500 == 0 and epoch != 0:
            #     scores_colors = {}
            #     max_value = max(list(scores.values()))
            #     for key, value in scores.items():
            #         if value > 0:
            #             scores_colors[key] = (0,round(255*value/max_value),0)
            #         else:
            #             scores_colors[key] = (round(255*value/max_value),0,0)
            #     visualize_graph(
            #         block_graph,
            #         './new_york_0_score_graph.png',
            #         coords=lambda n : block_graph.nodes[n]['block'].centroid,
            #         edge_colors=lambda n: scores_colors[n],
            #     )
            # if epoch % 5000 == 0 and epoch != 0:
                # visualize_map(communities_list, "docs/images/tests/random_" + str(epoch) + "_optimized_community_visualization.jpg")
            # print(list(scores.keys()))
            # print(id_to_community)

            rng = random.random()
            if rng < 1/3:
                if len(communities_list) < 2:
                    print("we definitely want more communities than that")
                    continue
            # if rng:
                exchange_start_time = time.time()
                print("block exchange", end="")
                block_to_give, community_to_give_to = _get_random_block_exchange(communities_list, id_to_block, id_to_community)
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
                            current_border_score += 2 * scores[(edge[0], edge[1])]
                            current_border_edges.append(edge)
                    # current_border_score /= len(scores)
                    current_border_score /= score_sum
                    block_to_give.community = community_to_give_to.id
                    community_to_give_to.find_neighbors_and_border(id_to_block)
                    community_to_take_from.find_neighbors_and_border(id_to_block)
                    new_border_edges = []
                    new_border_score = 0
                    for edge in community_to_give_to.border_edges:
                        if id_to_block[edge[1]].community == community_to_take_from.id:
                            new_border_score += 2 * scores[(edge[0], edge[1])]
                            new_border_edges.append(edge)
                    # new_border_score /= len(scores)
                    new_border_score /= score_sum
                    
                    new_energy = current_energy -2*new_border_score-2*current_border_score
                # print(f"2 exchange_time elapsed: {time.time()-exchange_start_time}")
                print(current_energy, "current energy", new_energy, "new energy")
                # print((sum(list(scores.values()))+new_community_1_score+new_community_2_score-scores[community_to_give_to.id]-scores[community_to_take_from.id])/(communities_num))

                if _probability(temp, current_energy, new_energy) > random.random():
                    
                    iterations_without_movement = 0
                    # print("probabilities do be added tho!")
                    # Keep the change!
                    if evaluation == "community":
                        community_to_give_to.take_block(block_to_give, id_to_block)
                        community_to_take_from.give_block(block_to_give, id_to_block)

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
                            current_energy = sum(list(scores.values()))/score_sum
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
                    # restore original border edges
                    block_to_give.community = community_to_take_from.id
                    community_to_give_to.find_neighbors_and_border(id_to_block)
                    community_to_take_from.find_neighbors_and_border(id_to_block)
                attempted_exchange_num += 1
                print(f"Exchange time elapsed: {time.time()-exchange_start_time}")
            elif rng < 2/3:
                merge_start_time = time.time()
                print("community merge", end="")
                if len(communities_list) < 2:
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
                            current_border_score += 2 * scores[(edge[0], edge[1])]
                            current_border_edges.append(edge)
                    # current_border_score /= len(scores)
                    current_border_score /= score_sum
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
                            current_energy = sum(list(scores.values()))/score_sum
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
                begin_time = time.time()
                for block in new_community_1.blocks:
                    block.community = new_community_1.id
                for block in new_community_2.blocks:
                    block.community = new_community_2.id
                print(time.time()-begin_time, '1')
                new_community_1.initialize_graph(id_to_block)
                # community_1.find_neighbors_and_border(chosen_community.block_ids)
                new_community_1.find_neighbors_and_border(id_to_block)
                new_community_2.initialize_graph(id_to_block)
                # community_2.find_neighbors_and_border(chosen_community.block_ids)
                new_community_2.find_neighbors_and_border(id_to_block)
                print(time.time()-begin_time, '2')
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
                    # new_border_score1 = 0
                    new_border_edges = []
                    for edge in new_community_1.border_edges:
                        if id_to_block[edge[1]].community == new_community_2.id:
                            new_border_score += 2 * scores[(edge[0],edge[1])]
                            # new_border_score1 += 2 * scores[edge]
                            # new_border_score1 += scores[(edge[0],edge[1])]
                            # new_border_score1 += scores[(edge[1],edge[0])]
                            new_border_edges.append(edge)
                    # new_border_score /= len(scores)
                    new_border_score /= score_sum
                    # print("NEW BORDER SCORE", new_border_score, "NEW BORDER SCORE1", new_border_score1)
                    new_energy = current_energy - 2 *new_border_score
                print(time.time()-begin_time, '3')
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
                        communities_list.remove(old_community)
                        communities_list.append(new_community_1)
                        communities_list.append(new_community_2)
                        communities_num += 1
                        print(time.time()-begin_time, '4')
                        for neighbor in new_community_1.neighbors:
                            # if "1000000US500019604002000" in list(id_to_community[neighbor].block_ids.keys()):
                            #     print("YOOOOOO THIS IS BAD", neighbor, "new com 1")
                            if neighbor != old_community.id:
                                id_to_community[neighbor].find_neighbors_and_border(id_to_block)
                                # print(id_to_community[neighbor].neighbors, "neighbor neighbors 1")
                        print(time.time()-begin_time, '5')
                        for neighbor in new_community_2.neighbors:
                            # if "1000000US500019604002000" in list(id_to_community[neighbor].block_ids.keys()):
                            #     print("YOOOOOO THIS IS BAD", neighbor, "new com 2")
                            if neighbor != old_community.id:
                                id_to_community[neighbor].find_neighbors_and_border(id_to_block)

                        print(time.time()-begin_time, '6')
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
                            current_energy = sum(list(scores.values()))/score_sum

                        print(time.time()-begin_time, '7')

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
            # print(sum(list(scores.values()))/len(scores), sum(list(scores.values()))/score_sum, current_energy, "CURRENT ENERGY")
            # print(new_energy, "NEW ENERGY")
            # for key, value in scores.items():
            #     if value < 0 and id_to_block[key[0]].community != id_to_block[key[1]].community:
            #         calc = calculate_edge_difference(id_to_block[key[0]],id_to_block[key[1]])
            #         raise Exception("OK THIS IS HELLA SUSS", calc, value)
            #     elif value > 0 and id_to_block[key[0]].community == id_to_block[key[1]].community:
            #         calc = calculate_edge_difference(id_to_block[key[0]],id_to_block[key[1]])
            #         raise Exception("OK THIS IS HELLA SUSS1", calc, value, key[0], key[1])
            # for community in communities_list:
                # if len(list(c for c in nx.connected_components(community.graph))) > 1:
                    # raise Exception("OK, THIS IS THE EARLIEST!!!", community.id, len(list(c for c in nx.connected_components(community.graph))))
            # for community in communities_list:
            #     blocks_community_list = set([block.community for block in community.blocks])
            #     if len(blocks_community_list) > 1 or list(blocks_community_list)[0] != community.id:
            #         raise Exception(blocks_community_list, community.id)
            if current_energy > best_energy:
                best_energy = current_energy
                best_communities_dict = {}
                for id, block in id_to_block.items():
                    best_communities_dict[id] = block.community
                print("best communities updated")
            print(f"TEMPERATURE: {temp}")
            print(f"\rEpoch: {epoch}, {round(100*epoch/EPOCHS, 1)}% ", end="")
            print(f"Current Energy: {current_energy} Best Energy: {best_energy} Communities #: {len(list(communities_list))} ", end="")
            print(f"Ex: {exchange_num}, {round(100*exchange_num/max(attempted_exchange_num,1), 1)}%  M: {merge_num}, {round(100*merge_num/max(attempted_merge_num,1), 1)}%  S: {split_num} {round(100*split_num/max(attempted_split_num,1), 1)}% ", end="")
            print(f"Iterations w/out change: {iterations_without_movement} ", end="")
            print(f"{state}, metric {METRIC}, {THRESHOLD} threshold", end="")
            print("")
            temp *= ALPHA
            # print(f"\rEpoch: {epoch}, {round(100*epoch/EPOCHS, 1)}% Current Energy: {current_energy} ", end="")
            sys.stdout.flush()
            # print(community_set, [community.id for community in communities_list])
            # block_list = []
            # for community in best_communities:
            #     block_list.extend(community.blocks)
            # print(len(best_communities))
            # print(len(block_list))
            # print(len(set(block_list)), "set of blocks")
            # print("number of blocks as it is supposed to be", 350828)
        except KeyboardInterrupt:
            text_input = input()
            if text_input == "end":
                print("\n", end="")
                print(f"Run ended at epoch {epoch}. {round(time.time()-very_start_time)} seconds elapsed for an average of {epoch/(time.time()-very_start_time)} epochs per second.")
                print(set(best_communities_dict.values()))
                for block_id in best_communities_dict:
                    id_to_block[block_id].community = best_communities_dict[block_id]
                best_communities_block_list = {}
                for community in set(best_communities_dict.values()):
                    best_communities_block_list[community] = []
                for id, community in best_communities_dict.items():
                    best_communities_block_list[community].append(id_to_block[id])
                best_communities = []
                state = communities_list[0].state
                for community_id, block_list in best_communities_block_list.items():
                    best_communities.append(Community(state, community_id, block_list))
                return best_communities, energies, temperatures, number_of_communities_list, mar, sar, ear
            elif text_input == "visualize_communities":
                visualize_map(communities_list, "docs/images/current_random_community_visualization.jpg")
            elif text_input == "visualize_graph":
                scores_colors = {}
                max_value = max(list(scores.values()))
                for key, value in scores.items():
                    if value > 0:
                        scores_colors[key] = (0,round(255*value/max_value),0)
                    else:
                        scores_colors[key] = (round(255*value/max_value),0,0)
                visualize_graph(
                    block_graph,
                    './current_score_graph.png',
                    coords=lambda n : block_graph.nodes[n]['block'].centroid,
                    edge_colors=lambda n: scores_colors[n],
                )
            elif text_input == "save":
                with open("current_community_list.pickle", "wb") as f:
                    pickle.dump(communities_list, f)
            elif len(text_input.split(" ")) > 1:
                if text_input.split(" ")[0] == "change":
                    print(text_input.split(" ")[1] + " = " + text_input.split(" ")[2])
                    exec(text_input.split(" ")[1] + " = " + text_input.split(" ")[2])
                elif text_input.split(" ")[0] == "print":
                    try:
                        exec("print("+text_input.split(" ")[1]+")")
                    except NameError:
                        print(text_input.split(" ")[1], "is not defined.")
                        
        except Exception as e:
        # except:
            raise Exception(str(e))
            print("\n", end="")
            # if e.__class__.__name__ != "KeyboardInterrupt":
            print(str(e))
            print(f"Run ended at epoch {epoch}. {round(time.time()-very_start_time)} seconds elapsed for an average of {epoch/(time.time()-very_start_time)} epochs per second.")
            return None, energies, temperatures, number_of_communities_list, mar, sar, ear
    print("\n", end="")
    print(f"{iterations_without_movement} iterations without change in communities.")
    print(f"Best energy found: {best_energy}")
    print(f"Run ended at epoch {epoch}. {round(time.time()-very_start_time)} seconds elapsed for an average of {epoch/(time.time()-very_start_time)} epochs per second.")
    # print(len(best_communities))
    # print(len(block_list))
    # print(len(set(block_list)), "set of blocks")
    print(set(best_communities_dict.values()))
    for block_id in best_communities_dict:
        id_to_block[block_id].community = best_communities_dict[block_id]
    best_communities_block_list = {}
    for community in set(best_communities_dict.values()):
        best_communities_block_list[community] = []
    for id, community in best_communities_dict.items():
        best_communities_block_list[community].append(id_to_block[id])
    best_communities = []
    state = communities_list[0].state
    for community_id, block_list in best_communities_block_list.items():
        best_communities.append(Community(state, community_id, block_list))
    return best_communities, energies, temperatures, number_of_communities_list, mar, sar, ear

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

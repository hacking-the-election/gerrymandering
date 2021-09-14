"""
Contains partition function used in community generation algorithm
"""

# cimport random as random
import random
import time

# cimport networkx as nx
import networkx as nx

from hacking_the_election.visualization.graph_visualization import visualize_graph
from hacking_the_election.utils.community import Community

def partition(chosen_community, id_to_block, id_to_community):
    """
    Takes in Community object, returns two Community objects which partition the object
    """
    hh = time.time()
    starting_1_block, starting_2_block = random.sample(chosen_community.blocks, 2)
    community_1_blocks = [starting_1_block]
    starting_1_block.mark = 1
    community_2_blocks = [starting_2_block]
    starting_2_block.mark = 2
    neighbors_1 = [id for id in starting_1_block.neighbors if id_to_block[id].community == chosen_community.id and id_to_block[id] not in community_2_blocks]
    neighbors_2 = [id for id in starting_2_block.neighbors if id_to_block[id].community == chosen_community.id and id_to_block[id] not in community_2_blocks]
    i = 0
    # print(starting_1_block.id, starting_2_block.id)
    while len(community_1_blocks) + len(community_2_blocks) < len(chosen_community.blocks):
        # print(len(community_1_blocks),len(community_2_blocks),len(community_1_blocks)+len(community_2_blocks),len(chosen_community.blocks))
        # set1 = set(community_1_blocks)
        # set2 = set(community_2_blocks)
        # print(len(set1), len(set2), len(set1 | set2))
        # print(neighbors_1, "neighbors1")
        # print(neighbors_2, "neighbors2")
        # for neighbor in neighbors_1:
        #     if hasattr(id_to_block[neighbor], "mark"):
        #         raise Exception("OK SUSSY", "1", neighbor)
        # for neighbor in neighbors_2:
        #     if hasattr(id_to_block[neighbor], "mark"):
        #         raise Exception("OK SUSSY", "2", neighbor)
        if len(neighbors_1)+len(neighbors_2) == 0:
            print(len(community_1_blocks), len(community_2_blocks), len(chosen_community.blocks))
            for block in chosen_community.blocks:
                if block not in community_1_blocks and block not in community_2_blocks:
                    print(block.id, block.neighbors, "ok!")
                    # for neighbor in block.neighbors:
                        # print(neighbor, id_to_block[neighbor].community, id_to_block[neighbor] in community_1_blocks, id_to_block[neighbor] in community_2_blocks)
            visualize_graph(
                chosen_community.graph,
                f'./chosen_community_graph.jpg',
                lambda n : chosen_community.graph.nodes[n]['block'].centroid,
                # colors=(lambda n c: spanning_tree.nodes[n]['color']),
                # sizes=(lambda n : spanning_tree.nodes[n]['size']),
                # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
                show=False
            )
            print(len([chosen_community.graph.subgraph(c) for c in list(nx.connected_components(chosen_community.graph))]))
            print(len(community_1_blocks),len(community_2_blocks),len(community_1_blocks)+len(community_2_blocks),len(chosen_community.blocks))
            set1 = set(community_1_blocks)
            set2 = set(community_2_blocks)
            print(len(set1), len(set2), len(set1 | set2))
            raise Exception("SUSSY")
        if i % 2 == 0:
        # if random.random() < 1/2:
            # Add to community_1
            # while next_neighbor_block in community_1_blocks or next_neighbor_block in community_2_blocks:
            # while hasattr(next_neighbor_block, "mark"):
            try:
                next_neighbor_id = neighbors_1.pop(0)
                while hasattr(id_to_block[next_neighbor_id], "mark"):
                    next_neighbor_id = neighbors_1.pop(0)
            except:
                i += 1
                continue
            next_neighbor_block = id_to_block[next_neighbor_id]

            community_1_blocks.append(next_neighbor_block)
            next_neighbor_block.mark = 1
            for neighbor in next_neighbor_block.neighbors:
                # if id_to_block[neighbor] in community_1_blocks or id_to_block[neighbor] in community_2_blocks:
                # if hasattr(id_to_block[neighbor], "mark"):
                #     continue
                # else:
                if id_to_block[neighbor].community == chosen_community.id:
                    neighbors_1.append(neighbor)
        else:
            # print("ok it started")
            # Add to community_2
            # either the while loop or the line 200 if function needs to go
            # while next_neighbor_block in community_1_blocks or next_neighbor_block in community_2_blocks:
            # while hasattr(next_neighbor_block, "mark"):
            try:
                next_neighbor_id = neighbors_2.pop(0)
                while hasattr(id_to_block[next_neighbor_id], "mark"):
                    next_neighbor_id = neighbors_2.pop(0)
            except:
                i += 1
                continue
            next_neighbor_block = id_to_block[next_neighbor_id]
            community_2_blocks.append(next_neighbor_block)
            next_neighbor_block.mark = 2
            for neighbor in next_neighbor_block.neighbors:
                # if id_to_block[neighbor] in community_1_blocks or id_to_block[neighbor] in community_2_blocks:
                # if hasattr(id_to_block[neighbor], "mark"):
                #     continue
                # else:
                if id_to_block[neighbor].community == chosen_community.id:
                    if neighbor == "1000000US500179594003012":
                        print(hasattr(id_to_block[neighbor], "mark"))
                    neighbors_2.append(neighbor)
        i += 1

    # for block in chosen_community.blocks:
    #     if block not in community_1_blocks and block not in community_2_blocks:
    #         print("double sussy!!!", block.id)
    # raise Exception("Sdf")
    for block in chosen_community.blocks:
        del block.mark
    max_id = max(id_to_community)
    # print(community_2_blocks)
    community_1 = Community(chosen_community.state, max_id+1, community_1_blocks)
    community_2 = Community(chosen_community.state, max_id+2, community_2_blocks)

    print(time.time()-hh, "adsf")
    return chosen_community, community_1, community_2

def test_partition(chosen_community, id_to_block, id_to_community):
    x = time.time()
    for edge in chosen_community.graph.edges():
            chosen_community.graph[edge[0]][edge[1]]['weight'] = random.random()
    # # nx.set_edge_attributes(chosen_community.graph, 'weight', 1)
    spanning_tree = nx.minimum_spanning_tree(chosen_community.graph, weight='weight')
    # visualize_graph(
    #     spanning_tree,
    #     f'./spanning_tree_graph.jpg',
    #     lambda n : spanning_tree.nodes[n]['block'].centroid,
    #     # colors=(lambda n : spanning_tree.nodes[n]['color']),
    #     # sizes=(lambda n : spanning_tree.nodes[n]['size']),
    #     # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
    #     show=False
    # )
    # max_size = round(len(chosen_community.graph.nodes)/2)
    # partition = nx.algorithms.community.lukes.lukes_partitioning(spanning_tree, max_size)
    # print(partition)
    # for edge in chosen_community.graph.edges():
            # chosen_community.graph[edge[0]][edge[1]]['weight'] = random.random()
    # print(chosen_community.graph.edges[0]['weight'])
    # second_spanning_tree = nx.minimum_spanning_tree(chosen_community.graph, weight='weight')
    # visualize_graph(
    #     second_spanning_tree,
    #     f'./second_spanning_tree_graph.jpg',
    #     lambda n : spanning_tree.nodes[n]['block'].centroid,
    #     # colors=(lambda n : spanning_tree.nodes[n]['color']),
    #     # sizes=(lambda n : spanning_tree.nodes[n]['size']),
    #     # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
    #     show=False
    # )
    spl = dict(nx.all_pairs_shortest_path_length(spanning_tree))
    print("spanning tree created!!", time.time()-x)
    # spl_1 = dict(nx.all_pairs_shortest_path_length(spanning_tree))
    # spl_2 = dict(nx.all_pairs_shortest_path_length(second_spanning_tree))
    # ok = time.time()
    # print("haha")
    # spl = dict(nx.all_pairs_shortest_path_length(chosen_community.graph))
    # print(time.time()-ok)
    block_1 = random.choice(chosen_community.blocks)
    block_2 = id_to_block[random.choice(block_1.neighbors)]
    # print(time.time()-ok)
    for node in chosen_community.graph.nodes():
        # if spl_1[block_1.id][node] < spl_2[block_2.id][node]:
        if spl[block_1.id][node] < spl[block_2.id][node]:
            chosen_community.graph.nodes[node]['color'] = (255,0,0)
        else:
            chosen_community.graph.nodes[node]['color'] = (0,0,255)
    # print("ok!", time.time()-ok, len(chosen_community.graph.nodes))
    visualize_graph(
        chosen_community.graph,
        f'./chosen_community_graph.jpg',
        lambda n : chosen_community.graph.nodes[n]['block'].centroid,
        colors=(lambda n : chosen_community.graph.nodes[n]['color']),
        # sizes=(lambda n : spanning_tree.nodes[n]['size']),
        # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
        show=False
    )
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
    # # for edge in chosen_community.graph.edges:
    # #     edge[weight] = 1
    # # spanning_tree = nx.minimum_spanning_tree(chosen_community.graph, weight='weight')
    # # edge_to_remove = random.choice(list(spanning_tree.edges))
    # print(f"Time needed: {time.time()-graph_start_time}")
    # c = center(spanning_tree)
    # central_node = random.choice(list(c))
    # edge_to_remove = random.choice(list(spanning_tree.edges(central_node)))
    # print(f"Time needed: {time.time()-graph_start_time}")

    # # edges_to_centrality = edge_betweenness_centrality(spanning_tree)
    # # centrality_to_edges = {}
    # # for edge, centrality in edges_to_centrality.items():
    #     # centrality_to_edges[centrality] = edge
    # # edge_to_remove = (starting_block, first_neighbor)
    # # edge_to_remove = centrality_to_edges[max(centrality_to_edges.keys())]
    # # del edges[edge_to_remove]
    # spanning_tree.nodes[edge_to_remove[0]]['color'] = (0,0,255)
    # spanning_tree.nodes[edge_to_remove[0]]['size'] = 5
    # spanning_tree.nodes[edge_to_remove[1]]['color'] = (0,0,255)
    # spanning_tree.nodes[edge_to_remove[1]]['size'] = 5
    # visualize_graph(
    #     spanning_tree,
    #     f'./spanning_tree_graph.jpg',
    #     lambda n : spanning_tree.nodes[n]['block'].centroid,
    #     colors=(lambda n : spanning_tree.nodes[n]['color']),
    #     sizes=(lambda n : spanning_tree.nodes[n]['size']),
    #     # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
    #     show=False
    # )
    # # print(edge_to_remove)
    # spanning_tree.remove_edge(edge_to_remove[0], edge_to_remove[1])
    # partition = list(spanning_tree.subgraph(c) for c in nx.connected_components(spanning_tree))
    # for node in list(partition[0].nodes):
    #     spanning_tree.nodes[node]['color'] = (255, 0, 0)
    # for node in list(partition[1].nodes):
    #     spanning_tree.nodes[node]['color'] = (0, 255, 0)
    # visualize_graph(
    #     spanning_tree,
    #     f'./spanning_tree_after_graph.jpg',
    #     lambda n : spanning_tree.nodes[n]['block'].centroid,
    #     colors=(lambda n : spanning_tree.nodes[n]['color']),
    #     sizes=(lambda n : spanning_tree.nodes[n]['size']),
    #     # sizes=(lambda n : block_graph.nodes[n]['precinct'].pop/500),
    #     show=False
    # )





def unnecessary_splitting_methods():
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
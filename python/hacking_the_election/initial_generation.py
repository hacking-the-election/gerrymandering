"""
Contains various functions and algorithms to generate the initial configuration for the community creation algorithm
Each function takes in a path to a .json or .pickle file and returns a list of Commmunity objects
"""

import sys
import json
import pickle
from random import choice

import networkx as nx
# from shapely.geometry import Polygon

from hacking_the_election.utils.block import Block
from hacking_the_election.utils.new_community import Community
from hacking_the_election.utils.geometry import geojson_to_shapely
from hacking_the_election.visualization.new_visualization import visualize_map
def _deserialize(path):
    if path.endswith("json"):
        block_to_index = {}
        block_graph = nx.Graph()
        block_list = []
        with open(path, "r") as f:
            loaded = json.load(f)
        for i, block in enumerate(loaded["features"]):
            # try:
            coordinates = geojson_to_shapely(block["geometry"]["coordinates"])
            # except:
                # print(block_id, i)
                # continue
            state = block["properties"]["STATE"]
            block_id = block["properties"]["ID"]
            neighbors = eval(block["properties"]["NEIGHBORS"])
            total_votes = float(block["properties"]["TOTAL_VOTES"])
            rep_votes = float(block["properties"]["REP_VOTES"])
            dem_votes = float(block["properties"]["DEM_VOTES"])
            racial_data = {}
            racial_data["white"] = float(block["properties"]["WHITE"])
            racial_data["black"] = float(block["properties"]["BLACK"])
            racial_data["hispanic"] = float(block["properties"]["HISPANIC"])
            racial_data["aapi"] = float(block["properties"]["AAPI"])
            racial_data["aian"] = float(block["properties"]["AIAN"])
            racial_data["other"] = float(block["properties"]["OTHER"])

            pop = float(block["properties"]["POP"])
            created_block = Block(pop, coordinates, state, block_id, racial_data, total_votes, rep_votes, dem_votes)
            created_block.neighbors = neighbors
            block_graph.add_node(i, block=created_block)
            block_list.append(created_block)
            block_to_index[created_block.id] = i
            print(f"\rBlocks added to graph: {i}", end="")
            sys.stdout.flush()
        for block in block_list:
            for neighbor_id in block.neighbors:
                block_graph.add_edge(block.id, block_to_index[neighbor_id])
    elif path.endswith("pickle"):
        with open(path, "rb") as f:
            block_graph = pickle.load(f)
    else:
        raise Exception("Incorrect file type, please use a .json or .pickle file")
    print("\n", end="")
    return block_graph

def random_generation(path, state):
    """
    Creates a random configuration of communities based on the "custom nation" model of EU4, starts with
    random starting block and adds neighbors. Continues until all blocks are used
    """
    block_graph = _deserialize(path)
    print("Block graph deserialized. ")
    block_dict = nx.get_node_attributes(block_graph, 'block')
    # print(block_dict)
    indexes = {i:block_dict[i] for i in range(len(block_dict))}
    ids_to_indexes = {block.id : i for i, block in indexes.items()}
    for index in ids_to_indexes.values():
        try:
            _ = indexes[index]
        except:
            print(index)
    community_list = []
    community_id = 0
    while len(block_dict) > 0:
        community_id += 1
        try:
            starting_index = choice(list(indexes.keys()))
        except:
            break
        starting_block = indexes[starting_index]
        del indexes[starting_index]
        neighbor_indexes = [id for id in starting_block.neighbors if ids_to_indexes[id] in indexes]
        blocks = [starting_block]
        community_population = starting_block.pop
        while community_population <= 20000:
            try:
                # Choose neighbor randomly, or go in order?
                # neighbor_id = choice(neighbor_indexes)
                neighbor_id = neighbor_indexes[0]
            except:
                break
            neighbor_indexes.remove(neighbor_id)
            try:
                neighbor_block = indexes[ids_to_indexes[neighbor_id]]
            except:
                continue
            else:
                del indexes[ids_to_indexes[neighbor_id]]
            blocks.append(neighbor_block)
            community_population += neighbor_block.pop
            for neighbor in neighbor_block.neighbors:
                if ids_to_indexes[neighbor] in indexes:
                    neighbor_indexes.append(neighbor)

        created_community = Community(state, community_id, blocks)
        for block in blocks:
            block.community = community_id
        community_list.append(created_community)
        print(f"\rCommunities created: {community_id}", end="")
        sys.stdout.flush()
    print("\n", end="")
    # print(block_dict)
    # Calcualate borders and neighbors for all communities
    ids_to_blocks = {block.id: block for block in block_dict.values()}
    for community in community_list:
        community.find_neighbors_and_border(ids_to_blocks)

    # Remove small communities
    id_to_community = {community.id:community for community in community_list}
    to_remove = [community for community in community_list if community.pop < 5000]
    for i, community in enumerate(to_remove):
        neighboring_community_id = choice(community.neighbors)
        id_to_community[neighboring_community_id].merge_community(community)
        print(f"\rCommunities merged: {i+1}/{len(to_remove)}, {round(100*(i+1)/len(to_remove), 1)}%", end="")
        sys.stdout.flush()
        community_list.remove(community)
    print("\n", end="")

    # Calcualate borders and neighbors for all communities
    ids_to_blocks = {block.id: block for block in block_dict.values()}
    for i, community in enumerate(community_list):
        community.find_neighbors_and_border(ids_to_blocks)
        # Renumber communities
        community.id = i
        for block in community.blocks:
            block.community = i
    print(f"Final number of communities: {len(community_list)}")
    return community_list

if __name__ == "__main__":
    community_list = random_generation("serialized.json", "vermont")
    with open("community_list.pickle", "wb") as f:
        pickle.dump(community_list, f)
    
    visualize_map(community_list, "./community_visualization.jpg")
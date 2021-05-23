"""
Contains various functions and algorithms to generate the initial configuration for the community creation algorithm
Each function takes in a path to a .json or .pickle file and returns a list of Commmunity objects
"""

import json
import pickle
import networkx as nx

from hacking_the_election.utils.block import Block
from hacking_the_election.utils.community import Community

def _deserialize(path):
    if path.endswith("json"):
        block_to_index = {}
        block_graph = nx.Graph()
        block_list = []
        with open(path, "r") as f:
            loaded = json.load(f)
        for i, block in enumerate(loaded["features"]):
            coordinates = block["geometry"]["coordinates"]
            state = block["properties"]["STATE"]
            block_id = block["properties"]["ID"]
            neighbors = block["properties"]["NEIGHBORS"]
            rep_votes = block["properties"]["REP_VOTES"]
            dem_votes = block["properties"]["DEM_VOTES"]
            racial_data = {}
            racial_data["white"] = block["properties"]["WHITE"]
            racial_data["black"] = block["properties"]["BLACK"]
            racial_data["hispanic"] = block["properties"]["HISPANIC"]
            racial_data["aapi"] = block["properties"]["AAPI"]
            racial_data["aian"] = block["properties"]["AIAN"]
            racial_data["other"] = block["properties"]["OTHER"]
            # this below should be changed as soon as the .jsonf files have a POP property
            pop = sum(list(racial_data.values()))
            created_block = Block(pop, coordinates, state, block_id, racial_data, rep_votes, dem_votes)
            created_block.neighbors = neighbors
            block_graph.add_node(i, block=created_block)
            block_list.append(created_block)
            block_to_index[created_block.id] = i
        for block in block_list:
            for neighbor_id in block.neighbors:
                block_graph.add_edge(block.id, block_to_index[neighbor_id])
    elif path.endswith("pickle"):
        with open(path, "rb") as f:
            block_graph = pickle.load(f)
    else:
        raise Exception("Incorrect file type, please use a .json or .pickle file")
    return block_graph

def random_generation(path):
    block_graph = _deserialize(path)
    block_list = [node.block for node in block_graph.nodes()]
    community_list = []
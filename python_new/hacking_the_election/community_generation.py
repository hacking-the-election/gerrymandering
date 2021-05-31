"""
Script for generating and optimizing political communities within a state.

Usage (from the python directory):
python3 -m hacking_the_election.community_generation [path_to_serialized.json] [state_name]
"""
import pickle
import time
import random

T_MAX = 5
EPOCHS = 40000
ALPHA = 0.99976

def create_block_graph():
    pass

def border_evaluation(block_graph):
    pass

def update_community_evaluation(scores_dict, *args):
    pass

def generate_communities(initial_communities_path, evaluation="community"):
    """
    Given a initial set of communities, optimizes them with simulated annealing. 
    Returns a list of optimized communities. 
    """
    with open(initial_communities_path, "rb") as f:
        communities_list = pickle.load(f)

    communities_num = len(communities_list)
    id_to_community = {community.id : community for community in communities_list}

    if evaluation == "border":
        block_graph = create_block_graph()
        current_energy = border_evaluation(block_graph)
    if evaluation == "community":
        scores = {}
        for community in communities_list:
            political_similarity = community.calculate_political_similarity()
            community.political_similarity = political_similarity
            racial_similarity = community.calculate_racial_similarity()
            community.racial_similarity = racial_similarity
            graphical_compactness = community.calculate_graphical_compactness()
            community.graphical_compactness = graphical_compactness
            scores[community.id] = political_similarity*racial_similarity*graphical_compactness
    
    temp = T_MAX
    
    if evaluation == "community":
        current_energy = sum(scores)/communities_num

    # TODO: implement saving best state, not just end state

    start_time = time.time()
    for epoch in range(EPOCHS):
        rng = random.random()
        if random < 1/3:
            _get_random_block_exchange(communities_list)
        elif random < 2/3:
        else:
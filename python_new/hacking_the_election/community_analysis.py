import pickle
import statistics
with open("vermont_optimized_community_list.pickle", "rb") as f:
    communities = pickle.load(f)
for community in communities:
    # for block in community.blocks:
    #     block.percent_dem = 0.5
    #     block.percent_rep = 0.3
    #     block.percent_other = 0.2
    percent_dem = [block.percent_dem for block in community.blocks if block.percent_dem != None]
    political_similarity = community.calculate_political_similarity()
    print(f"Political Similarity: {political_similarity}")
    print(f"Percent Dem standard deviation: {statistics.stdev(percent_dem)}")
    racial_similarity = community.calculate_race_similarity()
    print(f"Racial Similarity: {racial_similarity}")
    graphical_compactness = community.calculate_graphical_compactness()
    print(f"Graphical Compactness: {graphical_compactness}")
    print(f"Population: {community.pop}")
    print(f"Score: {(political_similarity*racial_similarity*graphical_compactness)**0.25}")
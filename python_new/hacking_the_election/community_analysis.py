"""
Contains functions which help provide analysis for community maps and for the community generation algorithm

Usage:
python3 -m hacking_the_election.community_analysis [file_to_analyze]
"""

import pickle
import sys
import statistics

import numpy as np
import matplotlib.pyplot as plt

def analyze_pickle(community_pickle):
    with open(community_pickle, "rb") as f:
        communities = pickle.load(f)

    block_list = []
    print(len(communities))
    for community in communities:
        # for block in community.blocks:
        #     if block.id == "1000000US360470622001010":
        #         print("ok wait, this is suss")
        block_list.extend(community.blocks)
    print(len(block_list))
    print(len(set(block_list)))
    id_to_block = {block.id : block for block in block_list}
    print(id_to_block['1000000US360470622001010'])
    pop_sum = 0
    political_similarities = []
    racial_similarities = []
    density_similarities = []
    # border_political_similarities = []
    # border_racial_similarities = []
    # border_density_similarities = []
    border_score = []
    scores = []
    max_other = 0
    for community in communities:
        community.initialize_graph(id_to_block)
        community.find_neighbors_and_border(id_to_block)
        for block in community.blocks:
            if block.percent_other:
                max_other = max(max_other, block.percent_other)
                # if block.percent_other == 1:
                #     print(block.id)
                # print(block.)
        pop_sum += community.pop
        # for block in community.blocks:
        #     block.percent_dem = 0.5
        #     block.percent_rep = 0.3
        #     block.percent_other = 0.2
        # percent_dems = [block.percent_dem for block in community.blocks if block.percent_dem != None]
        political_similarity = community.calculate_political_similarity()
        political_similarities.append(political_similarity)
        # print(f"Political Similarity: {political_similarity}")
        # print(f"Percent Dem standard deviation: {statistics.stdev(percent_dem)}")
        racial_similarity = community.calculate_race_similarity()
        racial_similarities.append(racial_similarity)
        density_similarity = community.calculate_density_similarity()
        density_similarities.append(density_similarity)
        border_score.append(community.calculate_border_score(id_to_block))
        scores.append(community.calculate_score(id_to_block, 0))
        # print(f"Racial Similarity: {racial_similarity}")
        # graphical_compactness = community.calculate_graphical_compactness()
        # print(f"Graphical Compactness: {graphical_compactness}")
        # print(f"Population: {community.pop}")
        # print(f"Score: {(political_similarity*racial_similarity*graphical_compactness)**0.25}")
    print(max_other, "MAX OTHER")
    print(f"Number of communities: {len(communities)}")
    print(f"Average Population: {pop_sum/len(communities)}")
    print(f"Smallest Community Population: {min([community.pop for community in communities])}")
    print(f"Largest Community Population: {max([community.pop for community in communities])}")
    print("Average Political Similarity: ", sum(political_similarities)/len(political_similarities))
    print("Average Racial Similarity: ", sum(racial_similarities)/len(racial_similarities))
    print("Average Density Similarity: ", sum(density_similarities)/len(density_similarities))
    print(f"Average border score: {sum(border_score)/len(border_score)}")
    print(f"Average score: {sum(scores)/len(scores)}")

def graph_metrics(file):
    to_plot_dict = {}
    with open(file, 'r') as f:
        lines = f.readlines()
        for i in range(len(lines)):
            if i % 2 == 0:
                name = lines[i][:-1]
            if i % 2 == 1:
                to_plot_dict[name] = eval(lines[i])
    # print(to_plot_dict)
    max_epoch = len(list(to_plot_dict.values())[0])
    indexes = np.arange(1, max_epoch+1)
    epochs = np.arange(1, max_epoch+1, step=round((max_epoch+1)/15))

    plt.figure(figsize=(12,15))
    plt.subplots_adjust(hspace=0.3)

    plt.subplot(411)
    plt.plot(indexes, to_plot_dict["Number of Communities"])
    plt.xticks(epochs)
    plt.title("Number of Communities")


    plt.subplot(412)
    plt.plot(indexes, to_plot_dict["Energies"])

    plt.xticks(epochs)
    min_value = min(to_plot_dict["Energies"])
    max_value = max(to_plot_dict["Energies"])
    step = (max_value-min_value)/7
    yticks = np.arange(min_value, max_value+step, step=step)
    ylabels = []
    for tick in yticks:
        ylabels.append(str(round(tick,1)) + "%")
    ax = plt.gca()
    ax.set_yticks(yticks)
    ax.set_yticklabels(ylabels)
    # plt.yticks(epochs, labels)
    plt.title("Energy")


    plt.subplot(413)
    plt.plot(indexes, to_plot_dict["Temperatures"])

    plt.xticks(epochs)
    min_value = min(to_plot_dict["Temperatures"])
    max_value = max(to_plot_dict["Temperatures"])
    step = (max_value-min_value)/7
    yticks = np.arange(min_value, max_value+step, step=step)
    ylabels = []
    for tick in yticks:
        ylabels.append(str(round(tick,1)) + "%")
    ax = plt.gca()
    ax.set_yticks(yticks)
    ax.set_yticklabels(ylabels)
    plt.title("Temperature")


    plt.subplot(414)
    plt.plot(indexes, to_plot_dict["Merge Acceptance Rate"], label="Merge Acceptance Rate")
    plt.plot(indexes, to_plot_dict["Split Acceptance Rate"], label="Split Acceptance Rate")
    plt.plot(indexes, to_plot_dict["Exchange Acceptance Rate"], label="Exchange Acceptance Rate")
    
    plt.xticks(epochs)
    ax = plt.gca()
    ax.set_yticks([0,20,40,60,80,100])
    ax.set_yticklabels(["0%", "20%", "40%", "60%", "80%", "100%"])

    plt.title("MAR, SAR, and EAR")
    plt.legend(loc='upper right')
    ax.set_xlabel("Epoch")

    plt.savefig("community_generation_graph.png")

if __name__ == "__main__":
    file = sys.argv[1]
    # print(file[file.find(".")+1:])
    # try:
    if file[file.find(".")+1:] == "pickle":
        analyze_pickle(file)
    elif file[file.find(".")+1:] == "txt":
        graph_metrics(file)
    else:
        raise TypeError("Argument passed in should be a pickle or txt file. ")
    # except:
        # raise TypeError("Argument passed in should be a pickle or txt file.") 
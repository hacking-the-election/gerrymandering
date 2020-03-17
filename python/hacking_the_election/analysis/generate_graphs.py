"""
Generates all the graphs required for data analysis of redistricting.

Usage:
python3 generate_graphs.py [districts_file] [redistricting] [pop_constraint]

`redistricting` should be given the value "true" or "false"
"""

import pickle
import sys

import matplotlib.pyplot as plt
import numpy as np

from hacking_the_election.serialization.save_precincts import Precinct
from hacking_the_election.test.funcs import convert_to_json, polygon_to_list
from hacking_the_election.utils.community import Community
from hacking_the_election.quantification import quantify


COLORS = ["blue", "green"]


def generate_graphs(districts_file, redistricting, pop_constraint):
    """
    Makes these graphs:
     - Number of Changed Precincts Over Iterations
     - Gerrymandering Score Over Iterations
     - Population Convergence
        - Population of each community over iterations
        - This one is the coolest because the lines come together
    """

    with open(districts_file, "rb") as f:
        if redistricting:
            community_stages, changed_precincts, gerrymandering_scores = \
                pickle.load(f)
        else:
            community_stages, changed_precincts = pickle.load(f)

    # Update gerrymandering scores list with communities that weren't quantified.
    with open("tmp.pickle", "wb+") as f:
        pickle.dump([[community_stages[0]], []], f)

    # Last iteration. Loop broke before quantification run.
    convert_to_json(
        [polygon_to_list(c.coords) for c in community_stages[-1]],
        "tmp.json",
        [{"District": c.id} for c in community_stages[-1]]
    )
    gerrymandering_scores.append(quantify("tmp.pickle", "tmp.json"))
    # Initial configuration
    convert_to_json(
        [polygon_to_list(c.coords) for c in community_stages[0]],
        "tmp.json",
        [{"District": c.id} for c in community_stages[0]]
    )
    gerrymandering_scores.insert(0, quantify("tmp.pickle", "tmp.json"))
    
    fig1 = plt.figure(1)

    # Number of Changed Precincts Over Iterations
    X = list(range(len(changed_precincts)))
    Y = [sum([len(refinement) for refinement in iteration])
         for iteration in changed_precincts]
    ax1 = fig1.add_subplot(111)
    ax1.set_title("Number of Changed Precincts Over Iterations")
    ax1.set_xlabel("Iterations")
    ax1.set_ylabel("Changed Precincts")
    ax1.xaxis.set_ticks(np.arange(1, len(changed_precincts) + 1, 1))
    ax1.plot(X, Y)

    # Gerrymandering Score Over Iterations
    if redistricting:
        fig2 = plt.figure(2)
        X = list(range(len(gerrymandering_scores)))
        Y = [score[1] for score in gerrymandering_scores]
        ax2 = fig2.add_subplot(111)
        ax2.set_title("Gerrymandering Score Over Iterations")
        ax2.set_xlabel("Iterations")
        ax2.set_ylabel("Gerrymandering Score")
        ax2.xaxis.set_ticks(np.arange(0, len(gerrymandering_scores), 1))
        ax2.plot(X, Y)

    # Population Convergence
    fig3 = plt.figure(3)

    for stage in community_stages:
        for community in stage:
            community.update_population()

    X = list(range(len(community_stages)))
    Y = [[stage[i].population for stage in community_stages]
         for i in range(len(community_stages[0]))]
    ax2 = fig3.add_subplot(111)
    ax2.set_title("Convergence of Populations of Districts")
    ax2.set_xlabel("Iterations")
    ax2.set_ylabel("Population")
    ax2.set_xlim(left=-0.25, right=len(community_stages) - 0.75)
    ax2.xaxis.set_ticks(np.arange(0, len(community_stages), 1))

    x = np.arange(-100, 100, 0.01)
    
    # Create shaded region to represent constraints
    ideal_pop = (sum([c.population for c in community_stages[0]])
               / len(community_stages[0]))
    max_pop = ideal_pop + (ideal_pop * (pop_constraint / 100))
    min_pop = ideal_pop - (ideal_pop * (pop_constraint / 100))
    ax2.fill_between(x, min_pop, max_pop, facecolor="red", alpha=0.5)

    # Create a line for each district
    for i, community in enumerate(Y):
        line, = ax2.plot(community, color=COLORS[i])
        line.set_label(f"District {i + 1}")
    ax2.legend()

    plt.show()


if __name__ == "__main__":
    generate_graphs(sys.argv[1], True if sys.argv[2] == "true" else False, float(sys.argv[3]))
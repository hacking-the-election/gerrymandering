"""
Generates all the graphs required for data analysis of redistricting.

Usage:
python3 generate_graphs.py [districts_file] [redistricting] [pop_constraint] [base_communities]

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


def get_squishing_function(min_val, max_val):
    """
    Returns a function that takes an input between `min_val` and
    `max_val` and squishes it proportionately between 0 and 1
    """
    def f(x):
        try:
            return (x - min_val) / (max_val - min_val)
        except ZeroDivisionError:
            return 0
    return f


def generate_graphs(districts_file, redistricting, pop_constraint, base_communities):
    """
    Makes these graphs:
     - Number of Changed Precincts Over Iterations
     - Gerrymandering Score Over Iterations
     - Population Convergence
        - Population of each community over iterations
        - This one is the coolest because the lines come together
    |--------
    | - Compactness Over Iteration
    | - Partisanship Stdev Over Iteration
    | - Percent Difference From Ideal Population Over Iteration
    |--------
    For the above there will be one graph with the state average over time.
    For each of them there will also be a graph with each line being a community.
    """

    with open(districts_file, "rb") as f:
        if redistricting:
            community_stages, changed_precincts, _ = pickle.load(f)
        else:
            community_stages, changed_precincts = pickle.load(f)

    for stage in community_stages:
        for community in stage:
            community.update_partisanship()
            community.update_standard_deviation()
            community.update_compactness()
            community.update_population()

    if redistricting:
        # Update gerrymandering scores list with communities that weren't quantified.
        gerrymandering_scores = []
        for stage in community_stages:
            convert_to_json(
                [polygon_to_list(c.coords) for c in stage],
                "tmp.json",
                [{"District": c.id} for c in stage]
            )
            gerrymandering_scores.append(quantify(base_communities, "tmp.json"))
    
    fig1 = plt.figure(1)

    # Number of Changed Precincts Over Iterations
    X = list(range(1, len(changed_precincts) + 1))
    Y = [sum([len(refinement) for refinement in iteration])
         for iteration in changed_precincts]
    ax1 = fig1.add_subplot(111)
    ax1.set_title("Number of Changed Precincts Over Iterations")
    ax1.set_xlabel("Iterations")
    ax1.set_ylabel("Changed Precincts")
    ax1.xaxis.set_ticks(np.arange(1, len(changed_precincts) + 1, 1))
    ax1.plot(X, Y)

    if redistricting:
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
    ax3 = fig3.add_subplot(111)
    ax3.set_title("Convergence of Populations of Districts")
    ax3.set_xlabel("Iterations")
    ax3.set_ylabel("Population")
    ax3.set_xlim(left=-0.25, right=len(community_stages) - 0.75)
    ax3.set_ylim(ymin=406488.4, ymax=609732.6)
    ax3.xaxis.set_ticks(np.arange(0, len(community_stages), 1))

    x = np.arange(-100, 100, 0.01)
    
    # Create shaded region to represent constraints
    ideal_pop = (sum([c.population for c in community_stages[0]])
               / len(community_stages[0]))
    max_pop = ideal_pop + (ideal_pop * (pop_constraint / 100))
    min_pop = ideal_pop - (ideal_pop * (pop_constraint / 100))
    ax3.fill_between(x, min_pop, max_pop, facecolor="red", alpha=0.5)

    # Create a line for each district
    for i, community in enumerate(Y):
        line, = ax3.plot(community, color=COLORS[i])
        line.set_label(f"District {i + 1}")
    ax3.legend()

    # Compactness Over Time (one line for each community)
    fig4 = plt.figure(4)

    X = list(range(len(community_stages)))
    Y = [[stage[i].compactness for stage in community_stages]
         for i in range(len(community_stages[0]))]
    ax4 = fig4.add_subplot(111)
    ax4.set_title("Compactness Over Iterations")
    ax4.set_xlabel("Iterations")
    ax4.set_ylabel("Compactness")

    # Create a line for each district
    for i, community in enumerate(Y):
        line, = ax4.plot(community, color=COLORS[i])
        line.set_label(f"District {i + 1}")
    ax4.legend()

    # Partisanship Standard Deviation Over Iterations
    fig5 = plt.figure(5)

    X = list(range(len(community_stages)))
    Y = [[stage[i].standard_deviation for stage in community_stages]
         for i in range(len(community_stages[0]))]
    ax5 = fig5.add_subplot(111)
    ax5.set_title("Partisanship Diversity Over Iterations")
    ax5.set_xlabel("Iterations")
    ax5.set_ylabel("Standard Deviation")

    # Create a line for each district
    for i, community in enumerate(Y):
        line, = ax5.plot(community, color=COLORS[i])
        line.set_label(f"District {i + 1}")
    ax5.legend()

    # Percent Difference from Average Population Over Iterations
    fig6 = plt.figure(6)

    X = list(range(len(community_stages)))
    Y = [[(abs(stage[i].population - ideal_pop) / ideal_pop) * 100
         for stage in community_stages]
         for i in range(len(community_stages[0]))]
    ax6 = fig6.add_subplot(111)
    ax6.set_title("Difference from Average Population Over Iterations")
    ax6.set_xlabel("Iterations")
    ax6.set_ylabel("Percent Difference")

    # Create a line for each district
    for i, community in enumerate(Y):
        line, = ax6.plot(community, color=COLORS[i])
        line.set_label(f"District {i + 1}")
    ax6.legend()

    # Average Constraint Values Over Iterations
    fig7 = plt.figure()

    X = list(range(len(community_stages)))
    Y = [[] for _ in range(3)]

    # Constraint values for each stage:
    average_constraints = [
        [sum([c.standard_deviation for c in stage]) / (l := len(stage)),
         sum([1 - c.compactness for c in stage]) / l,
         sum([(abs(c.population - ideal_pop) / ideal_pop) * 100
              for c in stage]) / l]
        for stage in community_stages
    ]
    
    # Create "squishing functions" for each of the constraints
    stdev_squish = get_squishing_function(
        min([stage[0] for stage in average_constraints]),
        max([stage[0] for stage in average_constraints])
    )
    compactness_squish = get_squishing_function(
        min([stage[1] for stage in average_constraints]),
        max([stage[1] for stage in average_constraints])
    )
    population_squish = get_squishing_function(
        min([stage[2] for stage in average_constraints]),
        max([stage[2] for stage in average_constraints])
    )

    # Add squished values to Y
    for stage in average_constraints:
        Y[0].append(stdev_squish(stage[0]))
        Y[1].append(compactness_squish(stage[1]))
        Y[2].append(population_squish(stage[2]))

    ax7 = fig7.add_subplot(111)
    ax7.set_title("Average Constraint Values Over Iterations")
    constraint_order = [
        "Partisanship Diversity",
        "Uncompactness",
        "Difference in Population from Average"
    ]
    for constraint_name, constraint_line in zip(constraint_order, Y):
        line, = ax7.plot(X, constraint_line)
        line.set_label(constraint_name)
    ax7.legend()

    plt.show()


if __name__ == "__main__":
    generate_graphs(
        sys.argv[1],
        True if sys.argv[2] == "true" else False,
        float(sys.argv[3]),
        sys.argv[4]
    )

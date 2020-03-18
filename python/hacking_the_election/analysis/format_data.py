"""
Takes a pickle and makes it human readable.

Usage:
python3 format_data.py [input_file] [output_file] [redistricts]
"""

import pickle
import sys

from hacking_the_election.test.funcs import convert_to_json, polygon_to_list
from hacking_the_election.serialization.save_precincts import Precinct
from hacking_the_election.utils.community import Community
from hacking_the_election.quantification import quantify


def format_data(input_file, output_file, redistricts):
    """
    Gets a pickle for a nickle (no tickle!)
    """

    with open(input_file, "rb") as f:
        if redistricts:
            community_stages, changed_precincts, gerrymandering_scores = \
                pickle.load(f)
        else:
            community_stages, changed_precincts = pickle.load(f)

    iterations = list(range(len(community_stages)))
    for stage in community_stages:
        for community in stage:
            community.update_partisanship()
            community.update_standard_deviation()
            community.update_compactness()
            community.update_population()

    if redistricts:
        # Gerrymandering Score
        with open("tmp.pickle", "wb+") as f:
            pickle.dump([[community_stages[0]], []], f)
        # Last iteration. Loop broke before quantification run.
        convert_to_json(
            [polygon_to_list(c.coords) for c in community_stages[-1]],
            "tmp.json",
            [{"District": str(c.id)} for c in community_stages[-1]]
        )
        gerrymandering_scores.append(quantify("tmp.pickle", "tmp.json"))
        # Initial configuration
        convert_to_json(
            [polygon_to_list(c.coords) for c in community_stages[0]],
            "tmp.json",
            [{"District": str(c.id)} for c in community_stages[0]]
        )
        gerrymandering_scores.insert(0, quantify("tmp.pickle", "tmp.json"))

    # Partisanship
    partisanships = [[stage[i].partisanship for stage in community_stages]
                     for i in range(len(community_stages[0]))]
    
    # Standard Deviation
    standard_deviations = \
        [[stage[i].standard_deviation for stage in community_stages]
         for i in range(len(community_stages[0]))]

    # Compactness
    compactness = [[stage[i].compactness for stage in community_stages]
                   for i in range(len(community_stages[0]))]
    
    # Population
    population = [[stage[i].population for stage in community_stages]
                  for i in range(len(community_stages[0]))]

    # Changed precincts
    changed_precincts = [sum([len(refinement) for refinement in iteration])
                         for iteration in changed_precincts]
    changed_precincts.insert(0, 0)

    rows = []
    for i in iterations:
        row = []
        row.append(str(i))
        row.append(str(changed_precincts[i]))
        for c in range(len(community_stages[0])):
            if redistricts:
                row.append(
                    str(gerrymandering_scores[i][0]
                        [str(community_stages[i][c].id)]))
            row.append(str(partisanships[c][i]))
            row.append(str(standard_deviations[c][i]))
            row.append(str(compactness[c][i]))
            row.append(str(population[c][i]))
        rows.append(row)

    with open(output_file, "w+") as f:
        f.write("\n".join(["\t".join(row) for row in rows]))


if __name__ == "__main__":
    format_data(*sys.argv[1:])
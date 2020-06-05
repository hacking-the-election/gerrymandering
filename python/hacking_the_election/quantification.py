"""
Algorithm implementation for quantifying gerrymandering.

Algorithm developed for
"Hacking the Election: Measuring and Solving Gerrymandering in Today's Political System"

!!!===========================================================================!!!

Currently NOT an implementation of the above, but simply a parser for the c++ output from the community algorithm.
Usage:
python3 -m hacking_the_election.quantification <quantification.tsv>
"""

import sys

from hacking_the_election.utils.stats import average


def get_quantification_values(table):
    """Gets the statewide 0 to 1 and -1 to 1 scores from a data table.
    
    :param table: 2d list of a quantification output from the c++ algorithm implementation.
    :type table: list

    :return: 0 to 1 quantification value, and -1 to 1 quantification value.
    :rtype: float, float
    """

    gerry_score = average(table[0])

    total_rep = 0
    total_votes = 0
    for lst in table[-1]:
        total_rep += lst[1]
        total_votes += sum(lst)
    
    partisanship_score = 2 * (0.5 - total_rep / total_votes) * gerry_score

    return gerry_score, partisanship_score


def get_vals_from_half(half):
    return (float(half.split(":")[-1][:-1]) if half[-1] == "]"
            else float(half.split(":")[-1]))


if __name__ == "__main__":

    with open(sys.argv[1], "r") as f:
        table = [line.strip().split("\t")
                 for line in f.read().strip().split("\n")]
    
    # Convert rows to lists of floats.

    for r, row in enumerate(table[:2]):
        for i, val in enumerate(row):
            table[r][i] = float(val)

    for i, lst in enumerate(table[:][-1]):
        table[-1][i] = [get_vals_from_half(half) for half in lst.split(",")]
    
    print(*get_quantification_values(table))
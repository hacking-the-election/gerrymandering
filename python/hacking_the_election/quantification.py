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
    avg_rep = average([d["REP"] for d in table[-1]])
    
    partisanship_score = 2 * (0.5 - avg_rep) * gerry_score

    return gerry_score, partisanship_score


if __name__ == "__main__":

    with open(sys.argv[1], "r") as f:
        table = [line.strip().split("\t")
                 for line in f.read().strip().split("\n")]
    
    # Convert rows to lists of floats.

    for r, row in enumerate(table[:2]):
        for i, val in enumerate(row):
            table[r][i] = float(val)

    for i, d in enumerate(table[:][-1]):
        table[-1][i] = eval("{" + d[1:-1] + "}")
    
    quant_values = get_quantification_values(table)
    print(f"{quant_values[0]}\t{quant_values[1]}")

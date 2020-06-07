"""Creates a votes to seats graph

Usage:
python3 -m hacking_the_election.visualization.votes_to_seats <districts_file> <serialized_state> <state_name> <output_path> <redistricts>
"""

import sys

import matplotlib.pyplot as plt

from hacking_the_election.gerry_measures import gerry_measures_conversion, votes_to_seats


def graph_votes_seats(districts_file, serialized_state, name=None, output_path=None, redistricts=False):
    """Creates a votes/seats graph.

    :param districts_file: Path to file containing district data in either geojson, or as lists of precincts.
    :type districts_file: str

    :param serialized_state: Path to a serialized graph of precincts.
    :type serialized_state: str
    """

    # Create `hacking_the_election.utils.community.Community` objects.
    districts = gerry_measures_conversion(districts_file, serialized_state)
    
    # Get data for votes to seats curve.
    votes_seats_curve = votes_to_seats(districts, graph=True)
    X = []
    Y = []
    for point in votes_seats_curve:
        X.append(point[0])
        Y.append(point[1])
    
    # Fill area between votes to seats curve and the line y = x
    fig = plt.figure()
    ax = fig.add_subplot()
    ax.fill_between(X, Y, X)

    if name is not None:
        if redistricts:
            ax.set_title(f"{name} Redistricts Votes/Seats")
        else:
            ax.set_title(f"{name} Current Districts Votes/Seats")

    ax.set_ylabel("Democratic Share of Seats")
    ax.set_xlabel("Democratic Vote Share")

    if output_path is not None:
        plt.savefig(output_path)
    else:
        plt.show()


if __name__ == "__main__":

    graph_votes_seats(
        sys.argv[1], sys.argv[2],
        sys.argv[3], sys.argv[4],
        sys.argv[5] if sys.argv[5] != "False" else False
    )
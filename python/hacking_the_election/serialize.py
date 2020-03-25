"""
Script for serializing precinct-level election, geo, and population
data into graph of hacking_the_election.precinct.Precinct objects.
"""


import json
import pickle
import sys

from graph import Graph
from shapely.geometry import Polygon, MultiPolygon

from hacking_the_election.utils.precinct import Precinct


def create_graph(election_file, geo_file, pop_file):
    """
    Takes data from inputted files and returns all precincts
    as graph with Precinct objects as nodes.
    """

    # Read election and geo data files.
    with open(election_file, "r") as f:
        election_file_contents = f.read().strip()
    election_data = [line.split("\t") for line in
                     election_file_contents.split("\n")]
    with open(geo_file, "r") as f:
        geodata = json.load(f)

    precinct_graph = Graph()
    for i in range(1, len(election_data)):
        precinct_graph.add_node(i, Precinct({}, {}, 0, [], str(i)))

    return precinct_graph


if __name__ == "__main__":
    
    precinct_graph = create_graph(sys.argv[1:4])
    with open(sys.argv[4], "wb+") as f:
        pickle.dump(precinct_graph)
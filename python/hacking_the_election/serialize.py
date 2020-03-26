"""
Script for serializing precinct-level election, geo, and population
data.

Data is serialized into 2 files:
 - .xml file containing graph.
 - .pickle file containing Precinct objects.

Usage:
python3 serialize.py [election_file] [geo_file] [pop_file] [state] [output.pickle] [output.xml]
"""


import json
from os.path import dirname
import pickle
import sys

from pygraph.classes.graph import graph
from pygraph.readwrite.markup import write
from shapely.geometry import Polygon, MultiPolygon

from hacking_the_election.utils.precinct import Precinct


def create_graph(election_file, geo_file, pop_file, state):
    """
    Returns graph with precinct ids stored in nodes (string of XML)
    and list of Precinct objects.
    """

    # Read election and geo data files.
    with open(election_file, "r") as f:
        election_file_contents = f.read().strip()
    election_data = [line.split("\t") for line in
                     election_file_contents.split("\n")]
    with open(geo_file, "r") as f:
        geodata = json.load(f)

    with open(f"{dirname(__file__)}/state_metadata.pickle", "r") as f:
        state_metadata = json.load(f)[state]
    
    precincts = []
    precinct_graph = graph()

    return write(precinct_graph), precincts


if __name__ == "__main__":
    
    precinct_graph, precincts = create_graph(sys.argv[1:5])
    
    # Save precincts as pickle
    with open(sys.argv[5], "wb+") as f:
        pickle.dump(precincts)

    # Save graph with precinct ids as XML
    with open(sys.argv[6], "w+") as f:
        f.write(precinct_graph)
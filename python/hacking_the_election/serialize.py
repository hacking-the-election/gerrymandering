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


def convert_to_int(string):
    """
    Wrapped error handling for int().
    """
    try:
        return int(string)
    except ValueError:
        if "." in string:
            try:
                return int(string[:string.index(".")])
            except ValueError:
                return 0
        else:
            return 0


def tostring(string):
    """
    Removes redundant quotes or converts int to string.
    """
    try:
        if string[0] in ["\"", "'"] and string[-1] in ["\"", "'"]:
            return string[1:-1]
        else:
            return string
    except TypeError:  # most likely that `string` was of type `int`
        return str(string)


def create_graph(election_file, geo_file, pop_file, state):
    """
    Returns graph with precinct ids stored in nodes (string of XML)
    and list of Precinct objects.
    """

    # Read election and geo data files.
    if election_file != "none":
        if election_file[-4:] == ".json":
            with open(election_file, "r") as f:
                election_data = json.load(f)
        elif election_file[-4:] == ".tab":
            with open(election_file, "r") as f:
                election_file_contents = f.read().strip()
            # create a list of rows that are a list of values in that row
            election_data = [line.split("\t") for line in
                            election_file_contents.split("\n")]
        else:
            raise ValueError('.json or .tab file')
    with open(geo_file, "r") as f:
        geodata = json.load(f)

    with open(f"{dirname(__file__)}/state_metadata.pickle", "r") as f:
        state_metadata = json.load(f)[state]
    dem_keys = state_metadata[state]["dem_keys"]
    rep_keys = state_metadata[state]["rep_keys"]
    json_ids  = state_metadata[state]["geo_id"]
    json_pops = state_metadata[state]["pop_key"]
    ele_id   = state_metadata[state]["ele_id"]

    precincts = []
    precinct_graph = graph()

    # Get election data.

    # {precinct_id: [{dem1:data,dem2:data}, {rep1:data,rep2:data}]}
    precinct_election_data = {}
    # If election data in geodata file...
    if election_file == "none":

        # Fill precinct_election_data
        for precinct in geodata["features"]:
            properties = precinct["properties"]
            precinct_id = "".join(properties[json_id] for json_id in json_ids)
            precinct_election_data[precinct_id] = [
                {key: convert_to_int(properties[key]) for key in dem_keys},
                {key: convert_to_int(properties[key]) for key in rep_keys}
            ]
    # If there is an election data file...
    else:
        # If the election data file is a .json file
        if election_file[-4:] == ".json":
             # Fill precinct_election_data
            for precinct in election_data["features"]:
                properties = precinct["properties"]
                precinct_id = "".join(properties[json_id] for json_id in json_ids)
                precinct_election_data[precinct_id] = [
                    {key: convert_to_int(properties[key]) for key in dem_keys},
                    {key: convert_to_int(properties[key]) for key in rep_keys}
                ]
        # If the election data file is a .tab file
        elif election_file[-4:] == ".tab":

            # headers for different categories
            election_column_names = election_data[0]

            # Get the index of each of the relevant columns.
            dem_key_col_indices = \
                [i for i, col in enumerate(election_column_names)
                if col in dem_keys]

            rep_key_col_indices = \
                [i for i, col in enumerate(election_column_names)
                if col in rep_keys]

            ele_id_col_index = 0
            for i, col in election_column_names:
                if col == ele_id:
                    ele_id_col_index = i
                    break

            # Fill precinct_election_data
            for precinct in election_data[1:]:
                precinct_election_data[precinct[ele_id_col_index]] = [
                    {election_column_names[i]: precinct[i]
                    for i in dem_key_col_indices},
                    {election_column_names[i]: precinct[i]
                    for i in rep_key_col_indices}
                ]

    # then find population
    # {precinct_id : population}
    pop = {}



    return write(precinct_graph), precincts


if __name__ == "__main__":
    
    precinct_graph, precincts = create_graph(sys.argv[1:5])

    # Save precincts as pickle
    with open(sys.argv[5], "wb+") as f:
        pickle.dump(precincts)

    # Save graph with precinct ids as XML
    with open(sys.argv[6], "w+") as f:
        f.write(precinct_graph)
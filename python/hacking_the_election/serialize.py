"""
Script for serializing precinct-level election, geo, and population
data.

Data is serialized into:
 - .pickle file containing graph with nodes containing Precinct objects

Usage:
python3 serialize.py [election_file] [geo_file] [pop_file] [state] [output.pickle]
"""


import json
from os.path import dirname
import pickle
import sys

from pygraph.classes.graph import graph
from pygraph.readwrite.markup import write
from shapely.geometry import Polygon, MultiPolygon

from hacking_the_election.utils.precinct import Precinct
from hacking_the_election.utils.serialization import compare_ids

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

    with open(f"{dirname(__file__)}/state_metadata.pickle", "r") as f:
        state_metadata = json.load(f)[state]

    dem_keys = state_metadata[state]["dem_keys"]
    rep_keys = state_metadata[state]["rep_keys"]
    # keys for third parties, if applicable
    other_keys = state_metadata[state]["other_keys"]
    json_ids  = state_metadata[state]["geo_id"]
    json_pops = state_metadata[state]["pop_key"]
    ele_ids   = state_metadata[state]["ele_id"]

    # Read election and geo data files.
    with open(geo_file, "r") as f:
        geodata = json.load(f)

    if election_file != "none":
        # Find election precincts not in geodata and vice versa

        if election_file[-4:] == ".json":
            election_data_type = "json"
            with open(election_file, "r") as f:
                election_data = json.load(f)

            election_data_ids = ["".join([precinct["properties"][ele_id] for ele_id in ele_ids]) 
                                 for precinct in election_data["features"]]
            geodata_ids = ["".join([precinct["properties"][json_id] for json_id in json_ids])
                           for precinct in geodata["features"]]
            
            election_data_to_geodata = compare_ids(election_data_ids, geodata_ids)

        elif election_file[-4:] == ".tab":
            election_data_type = "tab"
            with open(election_file, "r") as f:
                election_file_contents = f.read().strip()
            # create a list of rows that are a list of values in that row
            election_data = [line.split("\t") for line in
                            election_file_contents.split("\n")]
            ele_id_indices = [i for i, header in enumerate(election_data[0]) if header in ele_ids]
            election_data_ids = ["".join([election_data_row[ele_id_index] for ele_id_index in ele_id_indices])
                                 for election_data_row in election_data[1:]]
            geodata_ids = ["".join([precinct["properties"][json_id] for json_id in json_ids]) 
                           for precinct in geodata["features"]]

            election_data_to_geodata = compare_ids(election_data_ids, geodata_ids)
        else:
            # Election Data can only be .tab or .json files
            raise ValueError('.json or .tab file required')

    else:
        election_data_type = False

    if pop_file != "none":
        # Finds population data, any population_ids to geodata_ids will be done when finding pop
        if pop_file[-4:] == ".json":
            pop_data_type = "json"
            with open(pop_file, "r") as f:
                popdata = json.load(f)
        elif pop_file[-4:] == ".tab":
            pop_data_type = "tab"
            with open(pop_file, 'r') as f:
                popdata = f.read().strip()
        else:
            # Population Data can only be .tab or .json files
            raise ValueError('.json or .tab file required')
    else:
        pop_data_type = False

    precincts = []
    precinct_graph = graph()


    # Get election data. If needed, converts election data ids to geodata ids 
    # using election_data_to_geodata, thus all keys are those of geodata.
    # {precinct_id: [{dem1:data,dem2:data}, {rep1:data,rep2:data}]}
    precinct_election_data = {}
    # If there is an election data file...
    if election_data_type:
        # If the election data file is a .json file
        if election_data_type == "json":
             # Fill precinct_election_data
            for precinct1 in election_data["features"]:
                properties1 = precinct1["properties"]
                # Convert election data id to geodata id
                precinct_id1 = election_data_to_geodata["".join(properties1[ele_id] for ele_id in ele_ids)]
                precinct_election_data[precinct_id1] = [
                    {key: convert_to_int(properties1[key]) for key in dem_keys},
                    {key: convert_to_int(properties1[key]) for key in rep_keys}
                ]
        # If the election data file is a .tab file
        elif election_data_type == "tab":

            # headers for different categories
            election_column_names = election_data[0]

            # Get the index of each of the relevant columns.
            dem_key_col_indices = \
                [i for i, col in enumerate(election_column_names)
                if col in dem_keys]

            rep_key_col_indices = \
                [i for i, col in enumerate(election_column_names)
                if col in rep_keys]

            ele_id_col_indices = [i for i, header in election_column_names if header in ele_ids]

            # Fill precinct_election_data
            for precinct in election_data[1:]:
                # convert election data id to geodata id
                election_data_id = election_data_to_geodata[
                    "".join([precinct[ele_id_col_index] for ele_id_col_index in ele_id_col_indices])
                    ]
                precinct_election_data[election_data_id] = [
                    {election_column_names[i]: precinct[i]
                    for i in dem_key_col_indices},
                    {election_column_names[i]: precinct[i]
                    for i in rep_key_col_indices}
                ]
    # Election data is in geodata
    else:
        # Fill precinct_election_data
        for precinct in geodata["features"]:
            properties = precinct["properties"]
            precinct_id = "".join(properties[json_id] for json_id in json_ids)
            precinct_election_data[precinct_id] = [
                {key: convert_to_int(properties[key]) for key in dem_keys},
                {key: convert_to_int(properties[key]) for key in rep_keys}
            ]
    

    # Then find population. pop should have keys that use geodata ids
    # {precinct_id : population}
    pop = {}
    if pop_data_type:
        if pop_data_type == "json":
            for pop_precinct in popdata["features"]:
                pop_properties = pop_precinct["properties"]
                # Assumes the same Ids are used in geodata as in population data
                pop_precinct_id = "".join(pop_properties[json_id] for json_id in json_ids)
                pop[pop_precinct_id] = sum([pop_properties[key] for key in json_pops])
        # individual conditionals for states, they should go here
        elif pop_data_type == "tab":
            # In these condtionals, pop should have keys that match geodata. this is done here. 
            # compare_ids() from the serialization file in utils can be used for population
            pass
        else:
            raise AttributeError
    else:
        # find where population is stored, either election or geodata
        try:
            _ = geodata["features"][0]["properties"][json_pops[0]]
        except ValueError:
            pop_data_type = "tab"
            # population is in election data
            pop_col_indices = [i for i, header in enumerate(election_data[0]) if header in json_pops]
            # find which indexes have population data
            for row in election_data[1:]:
                pop_precinct_id = "".join([row[ele_id_col_index] for ele_id_col_index in ele_id_col_indices])
                precinct_populations = [row[i] for i in pop_col_indices]
                pop[pop_precinct_id] = sum(precinct_populations)
        else:
            # population is in geodata
            for geodata_pop_precinct in geodata["features"]:
                geodata_pop_properties = geodata_pop_precinct["properties"]
                geodata_pop_precinct_id = "".join(geodata_pop_properties[json_id] for json_id in json_ids)
                precinct_populations = [geodata_pop_properties[key] for key in json_pops]
                pop[precinct_id4] = sum(precinct_populations)

    for precinct in geodata["features"]:
        coordinate_data = precinct["geometry"]["coordinates"]
        geo_id = "".join([precinct["properties"][json_id] for json_id in json_ids])
        precinct_pop = pop[geo_id]
        
    return precinct_graph


if __name__ == "__main__":
    
    precinct_graph = create_graph(sys.argv[1:5])

    # Save graph as pickle
    with open(sys.argv[5], "wb+") as f:
        pickle.dump(precinct_graph)
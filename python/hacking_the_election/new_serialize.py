"""
Script for serializing block-level election, geo, and population data.
Data is serialized into ".pickle" file containing graph with nodes containing Precinct objects
Usage:
python3 -m hacking_the_election.new_serialize [state_name] [output_dir]
"""
import sys
import json
import pandas
import subprocess
from os.path import dirname, abspath
from os import listdir
import pickle
import networkx as nx

from hacking_the_election.utils.precinct import Precinct
# from hacking_the_election.utils.geometry import geojson_to_shapely, get_if_bordering



def create_graph(state_name):
    SOURCE_DIR = abspath(dirname(dirname(dirname(dirname(__file__))))) + "/hte-data-new/raw/" + state_name
    if state_name == "california":
        # Do special stuff for california
        pass

    # Load files in to function, and decompress them if necessary
    files = listdir(f"{SOURCE_DIR}")
    print(files)
    if not "block_geodata.json" in files:
        subprocess.run(["7za", "e", f"{SOURCE_DIR}/block_geodata.7z", f"-o{SOURCE_DIR}"])
    with open(f"{SOURCE_DIR}/block_geodata.json", "r") as f:
        block_geodata = json.load(f)

    if not "geodata.json" in files:
        subprocess.run(["7za", "e", f"{SOURCE_DIR}/geodata.7z", f"-o{SOURCE_DIR}"])
    with open(f"{SOURCE_DIR}/geodata.json", "r") as f:
        geodata = json.load(f)

    if not "block_demographics.csv" in files:
        subprocess.run(["7za", "e", f"{SOURCE_DIR}/block_demographics.7z",f"-o{SOURCE_DIR}"])
    with open(f"{SOURCE_DIR}/block_demographics.csv", "r") as f:
        block_demographics = pandas.read_csv(f)

    with open(f"{SOURCE_DIR}/demographics.csv", "r") as f:
        demographics = pandas.read_csv(f)

    with open(f"{SOURCE_DIR}/election_data.csv", "r") as f:
        election_data = pandas.read_csv(f)

    precinct_list = []

    # Precinct/census group ids, pandas Series
    ids = election_data["GEOID10"]
    print(demographics)
    # for geo_id in ids:
    #     rep_votes = election_data[election_data["GEOID10"] == geo_id]["Rep_2008_pres"].item()
    #     dem_votes = election_data[election_data["GEOID10"] == geo_id]["Dem_2008_pres"].item()

if __name__ == "__main__":
    block_graph = create_graph(sys.argv[1])

    # with open(sys.argv[2], "wb+") as f:
    #     pickle.dump(block_graph, f)
    # print(SOURCE_DIR)
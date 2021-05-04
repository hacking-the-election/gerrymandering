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
from shapely.geometry import Point

from hacking_the_election.utils.precinct import Precinct
from hacking_the_election.utils.block import Block
from hacking_the_election.utils.geometry import geojson_to_shapely, get_if_bordering

def fractional_assignment(racial_data):
    """
    Weights are based off of "A Practical Approach to Using Multiple-Race Response Data", 
    https://www.ncbi.nlm.nih.gov/pmc/articles/PMC2831381/
    """
    keys = list(racial_data.keys())
    for key in keys:
        # Get rid of multi-race categories with other in them
        if key.endswith("other"):
            if key != "other":
                racial_data[key[:-6]] += racial_data[key]
                del racial_data[key]

    keys = list(racial_data.keys())
    for key in keys:
        # Concatcate Asian Americans and Native Hawaiians and other Pacific Islanders
        if key.find("nhpi") != -1:
            if key.find("asian") != -1:
                end_nhpi = key.find("nhpi")+4
                # try:
                #     if key[end_nhpi+1] == ":":
                #         non_nhpi_key = key[:key.find("nhpi")-1] + key[end_nhpi+1:]
                # except:
                non_nhpi_key = key[:-5]
                racial_data[non_nhpi_key] += racial_data[key]
            else:
                non_nhpi_key = key.replace("nhpi", "asian")
                racial_data[non_nhpi_key] += racial_data[key]
            del racial_data[key]
    keys = list(racial_data.keys())
    for key in keys:
        # Rename the Asian category as Asian American and Pacific Islander (AAPI)
        if key.find("asian") != -1:
            aapi_key = key.replace("asian", "aapi")
            racial_data[aapi_key] = racial_data[key]
            del racial_data[key]

    # Apply weights
    racial_data["aian"] += .404*racial_data["aian:aapi"]
    racial_data["aapi"] += .596*racial_data["aian:aapi"]
    del racial_data["aian:aapi"]
    racial_data["aian"] += .186*racial_data["black:aian"]
    racial_data["black"] += .814*racial_data["black:aian"]
    del racial_data["black:aian"]
    racial_data["aian"] += .205*racial_data["white:aian"]
    racial_data["white"] += .795*racial_data["white:aian"]
    del racial_data["white:aian"]
    racial_data["black"] += .621*racial_data["white:black"]
    racial_data["white"] += .379*racial_data["white:black"]
    del racial_data["white:black"]
    racial_data["black"] += .370*racial_data["black:aapi"]
    racial_data["aapi"] += .630*racial_data["black:aapi"]
    del racial_data["black:aapi"]
    racial_data["aapi"] += .327*racial_data["white:aapi"]
    racial_data["white"] += .673*racial_data["white:aapi"]
    del racial_data["white:aapi"]

    racial_data["aian"] += .195*racial_data["white:black:aian"]
    racial_data["black"] += .572*racial_data["white:black:aian"]
    racial_data["white"] += .233*racial_data["white:black:aian"]
    del racial_data["white:black:aian"]
    racial_data["aian"] += .286*racial_data["black:aian:aapi"]
    racial_data["black"] += .461*racial_data["black:aian:aapi"]
    racial_data["aapi"] += .253*racial_data["black:aian:aapi"]
    del racial_data["black:aian:aapi"]
    racial_data["aian"] += .024*racial_data["white:aian:aapi"]
    racial_data["aapi"] += .043*racial_data["white:aian:aapi"]
    racial_data["white"] += .933*racial_data["white:aian:aapi"]
    del racial_data["white:aian:aapi"]
    racial_data["aapi"] += .104*racial_data["white:black:aapi"]
    racial_data["black"] += .113*racial_data["white:black:aapi"]
    racial_data["white"] += .782*racial_data["white:black:aapi"]
    del racial_data["white:black:aapi"]
    racial_data["aian"] += .01*racial_data["white:black:aian:aapi"]
    racial_data["aapi"] += .009*racial_data["white:black:aian:aapi"]
    racial_data["black"] += .02*racial_data["white:black:aian:aapi"]
    racial_data["white"] += .960*racial_data["white:black:aian:aapi"]
    del racial_data["white:black:aian:aapi"] 

def create_graph(state_name):

    SOURCE_DIR = "../../.." + "/hte-data-new/raw/" + state_name
    if state_name == "california":
        # Do special stuff for california
        pass

    # Load files in to function, and decompress them if necessary
    files = listdir(f"{SOURCE_DIR}")

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
        block_demographics = pandas.read_csv(f, header=1)

    with open(f"{SOURCE_DIR}/demographics.csv", "r") as f:
        demographics = pandas.read_csv(f)

    with open(f"{SOURCE_DIR}/election_data.csv", "r") as f:
        election_data = pandas.read_csv(f)

    precinct_coordinates = {precinct["properties"]["GEOID10"] :
        precinct["geometry"]["coordinates"] 
        for precinct in geodata["features"]
    }
    block_coordinates = {block["properties"]["GEOID10"] :
        block["geometry"]["coordinates"] 
        for block in block_geodata["features"]
    }

    precinct_list = []

    # Precinct/census group ids, pandas Series
    precinct_ids = election_data["GEOID10"] 


    precincts_num = len(precinct_ids)
    precincts_created = 0
    for geo_id in precinct_ids:
        rep_votes = election_data[election_data["GEOID10"] == geo_id]["Rep_2008_pres"].item()
        dem_votes = election_data[election_data["GEOID10"] == geo_id]["Dem_2008_pres"].item()
        # In addition to total population, racial data needs to be added as well
        total_pop = demographics[demographics["GEOID10"] == geo_id]["Tot_2010_tot"].item()

        coordinate_data = geojson_to_shapely(precinct_coordinates[geo_id])

        precinct = Precinct(total_pop, coordinate_data, state_name, geo_id, rep_votes, dem_votes)
        precinct_list.append(precinct)
        precincts_created += 1
        print(f"\rPrecincts Created: {precincts_created}/{precincts_num}, {round(100*precincts_created/precincts_num, 1)}%", end="")
        sys.stdout.flush()
    print("\n")
    block_ids = block_demographics["id"]
    block_num = len(block_ids)
    block_list = []
    county_to_blocks = {}
    blocks_created = 0
    previous_county = None
    for geo_id in block_ids:

        row = block_demographics[block_demographics["id"] == geo_id]

        total_pop = row["Total"].item()
        geo_id_beginning = geo_id.find("US")+2
        coordinate_data  = geojson_to_shapely(block_coordinates[geo_id[geo_id_beginning:]])
        racial_data = {}

        racial_data["hispanic"] = row["Total!!Hispanic or Latino"].item()

        racial_data["white"] = row["Total!!Not Hispanic or Latino!!Population of one race!!White alone"].item()
        racial_data["black"] = row["Total!!Not Hispanic or Latino!!Population of one race!!Black or African American alone"].item()
        racial_data["aian"] = row["Total!!Not Hispanic or Latino!!Population of one race!!American Indian and Alaska Native alone"].item()
        racial_data["asian"] = row["Total!!Not Hispanic or Latino!!Population of one race!!Asian alone"].item()
        racial_data["nhpi"] = row["Total!!Not Hispanic or Latino!!Population of one race!!Native Hawaiian and Other Pacific Islander alone"].item()
        racial_data["other"] = row["Total!!Not Hispanic or Latino!!Population of one race!!Some Other Race alone"].item()
        
        racial_data["white:black"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!White; Black or African American"].item()
        racial_data["white:aian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!White; American Indian and Alaska Native"].item()
        racial_data["white:asian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!White; Asian"].item()
        racial_data["white:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!White; Native Hawaiian and Other Pacific Islander"].item()
        racial_data["white:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!White; Some Other Race"].item()
        racial_data["black:aian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!Black or African American; American Indian and Alaska Native"].item()
        racial_data["black:asian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!Black or African American; Asian"].item()
        racial_data["black:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!Black or African American; Native Hawaiian and Other Pacific Islander"].item()
        racial_data["black:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!Black or African American; Some Other Race"].item()
        racial_data["aian:asian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!American Indian and Alaska Native; Asian"].item() 
        racial_data["aian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!American Indian and Alaska Native; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["aian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!American Indian and Alaska Native; Some Other Race"].item() 
        racial_data["asian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!Asian; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["asian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!Asian; Some Other Race"].item() 
        racial_data["nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of two races!!Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        
        racial_data["white:black:aian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; Black or African American; American Indian and Alaska Native"].item() 
        racial_data["white:black:asian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; Black or African American; Asian"].item() 
        racial_data["white:black:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; Black or African American; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["white:black:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; Black or African American; Some Other Race"].item() 
        racial_data["white:aian:asian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; American Indian and Alaska Native; Asian"].item() 
        racial_data["white:aian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; American Indian and Alaska Native; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["white:aian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; American Indian and Alaska Native; Some Other Race"].item() 
        racial_data["white:asian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; Asian; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["white:asian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; Asian; Some Other Race"].item() 
        racial_data["white:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!White; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["black:aian:asian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!Black or African American; American Indian and Alaska Native; Asian"].item() 
        racial_data["black:aian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!Black or African American; American Indian and Alaska Native; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["black:aian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!Black or African American; American Indian and Alaska Native; Some Other Race"].item() 
        racial_data["black:asian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!Black or African American; Asian; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["black:asian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!Black or African American; Asian; Some Other Race"].item() 
        racial_data["black:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!Black or African American; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["aian:asian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!American Indian and Alaska Native; Asian; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["aian:asian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!American Indian and Alaska Native; Asian; Some Other Race"].item() 
        racial_data["aian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!American Indian and Alaska Native; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["asian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of three races!!Asian; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        
        racial_data["white:black:aian:asian"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; Black or African American; American Indian and Alaska Native; Asian"].item() 
        racial_data["white:black:aian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; Black or African American; American Indian and Alaska Native; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["white:black:aian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; Black or African American; American Indian and Alaska Native; Some Other Race"].item() 
        racial_data["white:black:asian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; Black or African American; Asian; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["white:black:asian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; Black or African American; Asian; Some Other Race"].item() 
        racial_data["white:black:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; Black or African American; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["white:aian:asian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; American Indian and Alaska Native; Asian; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["white:aian:asian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; American Indian and Alaska Native; Asian; Some Other Race"].item() 
        racial_data["white:aian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; American Indian and Alaska Native; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["white:asian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!White; Asian; Native Hawaiian and Other Pacific Islander, Some Other Race"].item() 
        racial_data["black:aian:asian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!Black or African American; American Indian and Alaska Native; Asian; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["black:aian:asian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!Black or African American; American Indian and Alaska Native; Asian; Some Other Race"].item() 
        racial_data["black:aian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!Black or African American; American Indian and Alaska Native; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["black:asian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!Black or African American; Asian; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["aian:asian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of four races!!American Indian and Alaska Native; Asian; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        
        racial_data["white:black:aian:asian:nhpi"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of five races!!White; Black or African American; American Indian and Alaska Native; Asian; Native Hawaiian and Other Pacific Islander"].item() 
        racial_data["white:black:aian:asian:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of five races!!White; Black or African American; American Indian and Alaska Native; Asian; Some Other Race"].item() 
        racial_data["white:black:aian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of five races!!White; Black or African American; American Indian and Alaska Native; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["white:black:asian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of five races!!White; Black or African American; Asian; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["white:aian:asian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of five races!!White; American Indian and Alaska Native; Asian; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        racial_data["black:aian:asian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of five races!!Black or African American; American Indian and Alaska Native; Asian; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 

        racial_data["white:black:aian:asian:nhpi:other"] = row["Total!!Not Hispanic or Latino!!Two or More Races!!Population of six races!!White; Black or African American; American Indian and Alaska Native; Asian; Native Hawaiian and Other Pacific Islander; Some Other Race"].item() 
        
        # racial_data = fractional_assignment(racial_data)
        fractional_assignment(racial_data)
        block = Block(total_pop, coordinate_data, state_name, geo_id, racial_data)
        block_list.append(block)
        if previous_county == None or previous_county != geo_id[geo_id_beginning:geo_id_beginning+5]:
            county_to_blocks[geo_id[geo_id_beginning:geo_id_beginning+5]] = [block]
        else:
            county_to_blocks[geo_id[geo_id_beginning:geo_id_beginning+5]].append(block)
        previous_county = geo_id[geo_id_beginning:geo_id_beginning+5]
        # print(previous_county)
        blocks_created += 1
        print(f"\rBlocks Created: {blocks_created}/{block_num}, {round(100*blocks_created/block_num, 1)}%", end="")
        sys.stdout.flush()
    # with open("vermont_p.pickle", "wb") as f:
    #     pickle.dump(precinct_list, f)
    # with open("vermont_c.pickle", "wb") as f:
    #     pickle.dump(county_to_blocks, f)
    # with open("vermont_p.pickle", "rb") as f:
    #     precinct_list = pickle.load(f)
    # with open("vermont_c.pickle", "rb") as f:
    #    county_to_blocks = pickle.load(f)

    print("\n")
    seen_blocks = {}

    # Stores the ids of blocks which are in a precinct {id : [id]}
    precinct_to_blocks = {}
    blocks_matched = 0
    previous_precinct = None
    for precinct in precinct_list:
        possible_blocks = county_to_blocks[precinct.id[:5]]
        accounted_population = 0
        for block in possible_blocks:
            # Temporarily necessary for vermont testing
            if block.id == "1000000US500239541002044":
                continue
            if accounted_population > precinct.pop:
                break
            if precinct.max_x < block.min_x or precinct.min_x > block.max_x or precinct.max_y < block.min_y or precinct.min_y > block.max_y:
                continue
            else:
                if abs(precinct.coords.intersection(block.coords).area-block.coords.area)<0.5*block.coords.area:
                    # print(block.id, precinct.id)
                    if block.id in seen_blocks:
                        print("ALARM BELLS: ", block.id, seen_blocks[block.id], precinct.id)
                    # if precinct.coords.contains(block.coords.exterior.coords):
                    # if precinct.coords.contains(block.coords.representative_point()):
                    # print(previous_precinct, precinct.id)
                    if previous_precinct == None or previous_precinct != precinct.id:
                        precinct_to_blocks[precinct.id] = [block]
                    else:
                        precinct_to_blocks[precinct.id].append(block)
                    previous_precinct = precinct.id
                    accounted_population += block.pop
                    blocks_matched += 1
                    seen_blocks[block.id] = precinct.id
        print(f"\rBlocks Matched: {blocks_matched}/{block_num}", end="")
        sys.stdout.flush()
    print("\
        n")
    for precinct in precinct_list:
        precinct_blocks = precinct_to_blocks[precinct.id]
        # print(precinct_blocks)
        block_pop_sum = sum([block.pop for block in precinct_blocks])
        if block_pop_sum < 0:
            print("NEGATIVE BLOCK AREAS! BIG PROBLEM")
        elif block_pop_sum == 0:
            for block in precinct_blocks:
                block.rep_votes = 0
                block.dem_votes = 0
        else:
            for block in precinct_blocks:
                try:
                    block.rep_votes = precinct.total_rep * block.pop/block_pop_sum
                    block.dem_votes = precinct.total_dem * block.pop/block_pop_sum
                except:
                    print([block.pop for block in precinct_blocks], precinct.id)
                block.create_election_data()

    block_graph = nx.Graph()
    for i, block in enumerate(block_list):
        block_graph.add_node(i, block=block)
    node_num = len(block_graph.nodes())
    print(f"Number of blocks/nodes: {node_num}")

if __name__ == "__main__":
    block_graph = create_graph(sys.argv[1])

    # with open(sys.argv[2], "wb+") as f:
    #     pickle.dump(block_graph, f)
    # print(SOURCE_DIR)
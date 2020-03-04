"""
usage: python3 load_precincts.py [state_file]
"""


import json
import pickle

from hacking_the_election.utils.geometry import shapely_to_polygon

def load(state_file):
    """
    Return the list of precincts that was saved to a state file
    """
    with open(state_file, 'rb') as f:
        state = pickle.load(f)
    return state


def print_stats(lst, state):
    """
    Prints relevant information from state file
    """
    total_pop = 0
    total_dem = 0
    total_rep = 0
    n_missing = 0
    for precinct in lst[0]:
        total_pop += precinct.population
        if precinct.d_election_sum == -1 and precinct.r_election_sum == -1:
            n_missing += 1
        else:
            total_dem += precinct.d_election_sum
            total_rep += precinct.r_election_sum
    print(f"{state}:\n"
          f"Number of Precincts: {len(lst[0])}\n"
          f"Number of Districts: {len(lst[1]['features'])}\n"
          f"Population: {total_pop}\n"
          f"Democratic Votes: {total_dem}\n"
          f"Republican Votes: {total_rep}\n"
          f"Number of precincts without election data: {n_missing}")


def convert_to_json(state_file, output_file):
    """
    Converts pickled list of Precinct objects to geojson
    this is being left for dead
    """
    precincts = load(state_file)[0]
    features = []
    for precinct in precincts:
        precinct = shapely_to_polygon(precinct)
        precinct_json = {
            "type": "Feature",
            "geometry": {
                "type": "Polygon",
                "coordinates": precinct.coords
                },
            "properties":{
                "ID": precinct.vote_id,
                "POPULATION": precinct.population,
                "DEMOCRAT": precinct.d_election_sum,
                "REPUBLICAN": precinct.r_election_sum
                }
            }
        for key, val in precinct.d_election_data.items():
            precinct_json["properties"][key] = val
        for key, val in precinct.r_election_data.items():
            precinct_json["properties"][key] = val
        precinct_json = shapely_to_polygon(precinct_json)
        features.append(precinct_json)
    print(features)
    geo_json = {"type": "FeatureCollection", "features":features}
    print('stuff')
    with open(output_file, 'w+') as f:
        json.dump(geo_json, f)


if __name__ == "__main__":
    
    import sys

    lst = load(sys.argv[1])
    
    print_stats(lst, sys.argv[1].split('/')[-1].split(".")[0])
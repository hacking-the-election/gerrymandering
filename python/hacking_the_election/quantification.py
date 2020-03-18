"""
usage: python3 quantification.py [communities_pickle_file] [districts_file]

`communities_file` - path to file that stores community data,
                     which should store data as a list of community objects
                     with an attribute being a list of precinct_ids, all in a pickle

`districts_file` - path to file that stores district json, either preexisting or the .json for base communities
"""
# Note: How is gerrymandering defined? 
# While cracking and packing (splitting and combining political communities) does not necessarily imply gerrymandering,
# gerrymandering almost always requires cracking and packing. 
# Therefore, this quantification algorithm solely measures cracking and packing, but takes partisanship into 
# account to weight the gerrymandering score in order to create a fuller picture.

import json
from os.path import dirname
import pickle
import sys

sys.path.append(dirname(dirname(__file__)))

from hacking_the_election.utils.geometry import (
    get_area_intersection,
    polygon_to_shapely,
    shapely_to_polygon
)
from hacking_the_election.utils.stats import average, stdev
from hacking_the_election.serialization import save_precincts
from hacking_the_election.utils.community import Community
from hacking_the_election.utils.geometry import communities_to_json, clip

def quantify(communities_file, districts_file, acceptable_partisanship_threshold):
    '''
    Creates gerrymandering scores for each district and community in a district file by 
    comparing them to each other. Weights by population, weights population by area. 
    Then at the very end, weights by average of area of community in district / total area of community.
    communities_file should be a .pickle file.
    districts_file should be a .json file.
    When the partisanship factor is found, divide by the threshold and multiply by the unmodified_score. Should be percentage, 
    e.g. 9.5% = 9.5
    '''
    with open (communities_file, 'rb') as f:
        data = pickle.load(f)
    with open(districts_file, 'r') as f:
        district_data = json.load(f)
    # we don't need precinct corridors, only communities
    data = data[0][-1]
    for community in data:
        community.update_partisanship()
    communities_to_json(data, '../../../base_communities.json')
    # communities_to_json(data, '../../../base_communities.json')
    # keys: community index
    # values: list of: [decimal percentage of republicans, shapely community border coordinates polygon]
    community_coords = {community:community.coords
                     for community in data}
    # keys: ids
    # values: json polygon of coordinates
    try:
        district_dict = {district["properties"]["District"]: polygon_to_shapely(district["geometry"]["coordinates"]) 
                        for district in district_data["features"]}
    except KeyError:
        district_dict = {district["properties"]["ID"]: polygon_to_shapely(district["geometry"]["coordinates"]) 
                        for district in district_data["features"]}
    # find partianshipps
    partisanships = {community:community.partisanship for community in data}
    # find areas of communities
    community_areas = {community: community.coords.area for community in community_coords}
    # begin finding district gerrymandering scores
    district_scores = {}
    for district_id, district in district_dict.items():
        # finds total area of district
        total_area = district.area
        # keys: community.id for community object
        # values: area of intersection with district
        intersecting_communities ={}
        for community in community_coords.keys():
            coords = district.intersection(community.coords)
            # # note: this is where stuff is really slow
            # # coords = shapely_to_polygon(clip([district, community.coords], 1))
            # features = []
            # try: 
            #     _ = coords[0][0][0][0]
            # # coords[0][0].append(coords[0][0][0])
            #     features.append({"geometry": {"type":"MultiPolygon", "coordinates":coords}, "type":"Feature", "properties":{"ID":community.id}})
            # except:
            # # coords[0].append(coords[0][0])
            #     features.append({"geometry": {"type":"Polygon", "coordinates":coords}, "type":"Feature", "properties":{"ID":community.id}})
            # completed_json = {"type":"FeatureCollection", "features":features}
            # with open('../../../intersection.json', 'w') as f:
            #     json.dump(completed_json, f)
            # print(district.intersection(community.coords), district_id)
            if district.intersection(community.coords).area > 0:
                intersecting_communities[community] = district.intersection(community.coords).area
        # print({community.id : intersecting_communities[community] for community in intersecting_communities})
        # find biggest community, and area
        biggest_community = {}
        for key in intersecting_communities:
            print(biggest_community)
            if len(biggest_community) == 0:
                biggest_community[key] = intersecting_communities[key]
            else:
                if intersecting_communities[key] > list(biggest_community.values())[0]:
                    del biggest_community[list(biggest_community.keys())[0]]
                    biggest_community[key] = intersecting_communities[key]

        # find average_community_area and calculate weights for each district 
        average_community_population = average([community.population for community in data])

        pop_weights = [community.population/average_community_population for community in data]

        # Create list of partisanship weights (decimal of proportion of republicans in district) 
        # for intersecting communities
        district_partisanships = [community.partisanship for community in intersecting_communities]
        # find weighted standard deviation using partisanship, area_weights
        stdev_district = stdev(district_partisanships, pop_weights) * 100
        # print(stdev_district)
        # add score to district_scores dict:
        score = 1 - list(biggest_community.values())[0]/total_area
        # print('score', score)
        weighted1_score = score * stdev_district / float(acceptable_partisanship_threshold)
        # print('after partisanship factor', weighted1_score)
        district_scores[district_id] = weighted1_score
    state_score = average(district_scores.values())
    return district_scores, state_score
        

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) != 3:
        raise TypeError("Wrong number of arguments, see python file")
    district_score, state_score = quantify(*args)
    print(district_score, state_score)
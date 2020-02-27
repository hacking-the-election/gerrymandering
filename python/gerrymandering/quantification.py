"""
usage: python3 quantification.py [communities_pickle_file] [districts_file]

`communities_file` - path to file that stores community data,
                     which should store data as a list of community objects
                     with an attribute being a list of precinct_ids, all in a pickle

`districts_file` - path to file that stores district json
"""
# Note: How is gerrymandering defined? 
# While cracking and packing (splitting and combining political communities) does not necessarily imply gerrymandering,
# gerrymandering almost always requires cracking and packing. 
# Therefore, this quantification algorithm solely measures cracking and packing, but takes partisanship into 
# account to weight the gerrymandering score in order to create a fuller picture.

import sys
import os
import pickle
import json

sys.path.append('..')
sys.path.append('../serialization')
import gerrymandering
from gerrymandering.utils import *
from serialization import save_precincts
print('yes')
def quantify(communities_file, districts_file):
    '''
    Creates gerrymandering scores for each district and community in a district file by 
    comparing them to each other. Weights by population, weights population by area.
    '''
    with open (communities_file, 'rb') as f:
        data = pickle.load(f)
    with open(districts_file, 'r') as f:
        district_data = json.load(f)
    # keys: community index
    # values: list of: [decimal percentage of republicans, shapely community border coordinates polygon]
    community_dict = {community.id:[community.get_partisanship(community.precincts.values()), community.coords] 
                     for community in data}
    # keys: ids
    # values: json polygon of coordinates
    district_dict = {district["properties"]["District"]: district["geometry"]["coordinates"] 
                    for district in district_data["features"]}
    # find partianshipps
    partisanships = {id:community_dict[community][0] for id, community in community_dict.items()}
    # find areas of communities
    community_areas = {community:save_precincts.area(community_dict[community][1]) for community in community_areas}
    # begin finding district gerrymandering scores
    district_scores = {}
    for district in district_dict:
        # finds total area of district
        total_area = sum(save_precincts.area(district_dict[district]))
        # keys: community.id for community object
        # values: area of intersection with district
        intersecting_communities ={}
        for community in community_dict.keys():
            community1 = community_dict[community][1]
            if get_area_intersection(polygon_to_shapely(district), community1) > 0:
                intersecting_communities[community] = get_area_intersection(polygon_to_shapely(district, community1))
        # find average_community_area and calculate weights for each district relative to 
        average_community_area = sum(intersecting_communities.values())/len(intersecting_communities)
        area_weights = [area/average_community_area for area in intersecting_communities.values()]
        biggest_community = {}
        for key in intersecting_communities:
            if intersecting_communities[key] > biggest_community.get(key, 0):
                biggest_community[key] = intersecting_communities[key]
        # Create list of partisanship (decimal of proportion of republicans in district) 
        # for intersecting communities
        district_partisanships = [partisanships[community] for community in intersecting_communities]
        stdev_district = stdev(district_partisanships)
        for key, value in biggest_community.items():
            score = 1 - value/total_area
            district_scores[district] = score * stdev_district
    

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) != 2:
        raise TypeError("Wrong number of arguments, see python file")
    quantify(*args)
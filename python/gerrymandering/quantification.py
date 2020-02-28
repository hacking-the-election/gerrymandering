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

def quantify(communities_file, districts_file):
    '''
    Creates gerrymandering scores for each district and community in a district file by 
    comparing them to each other. Weights by population, weights population by area. 
    Then at the very end, weights by average of area of community in district / total area of community.
    communities_file should be a .pickle file.
    districts_file should be a .json file.
    '''
    with open (communities_file, 'rb') as f:
        data = pickle.load(f)
    with open(districts_file, 'r') as f:
        district_data = json.load(f)

    for community in data:
        community.update_partisanship()
    # keys: community index
    # values: list of: [decimal percentage of republicans, shapely community border coordinates polygon]
    community_dict = {community.id:[community.partisanship, community.coords] 
                     for community in data}
    # keys: ids
    # values: json polygon of coordinates
    district_dict = {district["properties"]["District"]: district["geometry"]["coordinates"] 
                    for district in district_data["features"]}
    # find partianshipps
    partisanships = {id:community_dict[id][0] for id in community_dict}
    print(partisanships)
    # find areas of communities
    community_areas = {community:save_precincts.area(shapely_to_polygon(community_dict[community][1])) for community in community_dict}
    # begin finding district gerrymandering scores
    district_scores = {}
    for district in district_dict:
        # finds total area of district
        total_area = save_precincts.area(district_dict[district][0][0])
        # keys: community.id for community object
        # values: area of intersection with district
        intersecting_communities ={}
        for community in community_dict.keys():
            # note: this is where stuff is really slow
            community1 = community_dict[community][1]
            if get_area_intersection(polygon_to_shapely(district_dict[district][0]), community1) > 0:
                intersecting_communities[community] = get_area_intersection(
                                                    polygon_to_shapely(district_dict[district][0]), community1)
        # find average_community_area and calculate weights for each district 
        average_community_area = sum(intersecting_communities.values())/len(intersecting_communities)
        area_weights = [area/average_community_area for area in intersecting_communities.values()]
        # percentage of community in district, for community in intersecting_communities
        community_percentages = [intersecting_communities[community]/community_areas[community] 
                                for community in intersecting_communities]
        # average of percentage of community in district. Therefore districts that take a 
        # small portion of communitites that are big get punished proportionally, weighted by the 
        # area of that community in the district.
        community_weight = sum([community_percentages[num]*area_weights[num] 
        for num in range(len(community_percentages))])
        # Create list of partisanship weights (decimal of proportion of republicans in district) 
        # for intersecting communities
        district_partisanships = [partisanships[community] for community in intersecting_communities]
        # find weighted standard deviation using partisanship, area_weights
        stdev_district = stdev(district_partisanships, area_weights)
        # find biggest community, and area
        biggest_community = {}
        for key in intersecting_communities:
            if intersecting_communities[key] > biggest_community.get(key, 0):
                biggest_community[key] = intersecting_communities[key]
        # add score to district_scores dict
        for key, value in biggest_community.items():
            score = 1 - value/total_area
            weighted1_score = score * stdev_district
            weighted2_score = weighted1_score / community_weight
            district_scores[district] = weighted2_score
    state_score = average(district_scores.values())
    print(district_scores, state_score)
        

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) != 2:
        raise TypeError("Wrong number of arguments, see python file")
    quantify(*args)
"""
usage: python3 quantification.py [communities_pickle_file] [districts_file]

`communities_file` - path to file that stores community data,
                     which should store data as a list of community objects
                     with an attribute being a list of precinct_ids, all in a pickle

`districts_file` - path to file that stores district json
"""

import sys
import os
import pickle

sys.path.append('..')
sys.path.append('../serialization')
import gerrymandering
import serialization.save_precincts
from utils import *
print('yes')
def quantify(communities_file, districts_file):
    with open (communities_file, 'rb') as f:
        data = pickle.load(f)
        community_dict = {}
        for num, community in enumerate(data):
            community_dict[num] = [community.precincts(), community.coords()]
        print(data[0].precincts)

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) != 2:
        raise TypeError("Wrong number of arguments, see python file")
    quantify(*args)
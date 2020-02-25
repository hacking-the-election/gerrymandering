"""
usage: python3 quantification.py [communities_file] [districts_file]

`communities_file` - path to file that stores community data,
                     which should store data as a list of community objects
                     with an attribute being a list of precinct_ids

`districts_file` - path to file that stores district json
"""

import sys
import json

def quantify(communities_file, districts_file):
    pass

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) != 2:
        raise TypeError("Wrong number of arguments, see python file")
    quantify(*args)
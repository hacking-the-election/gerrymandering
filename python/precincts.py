"""
Precinct-level maps from election-geodata compiled by Nathaniel Kelso and Michal Migurski.
Election data from harvard dataverse.

in election-geodata geojson files, "GEOID10" property (or equivalent) is the same as many different things
in harvard dataverse
"""


import json
from os import mkdir
from os.path import abspath, dirname, isdir
import pickle
import warnings


# where the precinct data is stored
# temporary, to be changed with legitimate data
objects_dir = abspath(dirname(dirname(__file__))) + '/data'


def save(state, precinct_list):
    """
    Save the list of precincts for a state to a file
    """
    file = f'{objects_dir}/{state}'
    with open(file, 'wb+') as f:
        pickle.dump(precinct_list, f)


def load(state):
    """
    Return the list of precincts that was saved to a state file
    """
    with open(f'{objects_dir}/{state}', 'rb') as f:
        state = pickle.load(f)
    return state


def area(coords):
    """
    Returns the area of a poylgon with vertices at 2-d list `coords`

    see https://www.mathopenref.com/coordpolygonarea2.html
    """
    a = 0

    try:
        for i, j in zip(range(len(coords)), range(-1, len(coords) - 1)):
            a += (coords[j][0] + coords[i][0]) * (coords[j][1] - coords[i][1])
    except TypeError:
        # keep trying until it works
        # the list may have been nested in more layers than necessary
        return area(coords[0])

    return a / 2


def convert_to_int(string):
    """
    Wrapped error handling for int().
    """
    try:
        return int(string)
    except Exception:
        return 0


class Precinct:
    """
    Represents a voting precinct

    params:
        `coords` - 2-d list of x and y coordinates of vertices
        `name` - name of precinct
        `state` - state that precinct is from
        `vote_id` - name from id system consistent 
                    between harvard and election-geodata files
        `d_election_data` - dict of name of vote to
                            number of votes.
                            e.g. {"g2002_GOV_dv": 100}
        `r_election_data` - above but for republicans.
    """

    def __init__(self, coords, name, state, vote_id,
                 d_election_data, r_election_data):
        
        # coordinate data
        self.coords = coords
        self.area = area(coords)
        
        # meta info
        self.name = name
        self.vote_id = vote_id
        self.state = state

        # election data
        self.d_election_data = d_election_data
        self.r_election_data = r_election_data

        self.d_election_sum = 0
        self.r_election_sum = 0

        # only include data for elections for which there is data for both parties
        for key in self.r_election_data.keys():
            if key in self.d_election_data.keys():
                d_election_sum += self.d_election_data[key]
                r_election_sum += self.r_election_data[key]

        self.dem_rep_ratio = self.d_election_sum / self.r_election_sum


    @classmethod
    def generate_from_files(cls, election_data_file, geo_data_file, state, id_finder, json_id):
        """
        Creates precinct objects for state from necessary information

        params:
            `election_data_file` - path to file containing
                                  election data for precinct (.tab)

            `geo_data_file` - path to file containing geodata for precinct
                              (.json or .geojson)

            `state` - name of state containing precinct

            `id-finder` - a function that takes a dict of data columns to
                          lists of values (from election data) and returns
                          a list of lists containing for each precinct a
                          precinct id (6-digit strings) and a column number

            `json_id` - the name of the json attribute the precinct ids 
                        should be matched with
        """

        with open(geo_data_file, 'r') as f:
            geo_data = json.load(f)

        with open(election_data_file, 'r') as f:
            election_data = f.read().strip()
        
        data_rows = [row.split('\t') for row in election_data.split('\n')]
        # 2-d list with each sublist being a column in the
        # election data file
        data_columns = [[data_rows[x][y] for x in range(len(data_rows))]
                        for y in range(len(data_rows[0]))]
        # keys: data categories; values: lists of corresponding values
        # for each precinct
        data_dict = {column[0]: column[1:] for column in data_columns}
        
        # keys in `data_dict` that correspond to party vote counts
        dem_keys = [key for key in data_dict.keys() if key[-2:] == 'dv']
        rep_keys = [key for key in data_dict.keys() if key[-2:] == 'rv']
        
        # [[precinct_id1, col1], [precinct_id2, col2]]
        precinct_ids = id_finder(data_dict)

        # Looks for precinct name (or if there is one)
        if "precinct_name" in data_dict:
            precinct_name_col = "precinct_name"
        elif "precinct" in data_dict:
            precinct_name_col = "precinct"
        else:
            precinct_name_col = False
        
        # keys: precinct ids.
        # keys of values: keys in `data_dict` that correspond
        #                 to vote counts.
        # values of values: number of votes for given party
        #                   in that election.
        dem_cols = {
            precinct[0]: {
                key: convert_to_int(data_dict[key][precinct[1]])
                for key in dem_keys
            } for precinct in precinct_ids
        }
        rep_cols = {
            precinct[0]: {
                key: convert_to_int(data_dict[key][precinct[1]])
                for key in rep_keys
            } for precinct in precinct_ids
        }

        precinct_list = []
        
        # match election and geo data and save Precinct objects
        # containing said data.
        precinct_geo_list = []
        for precinct in geo_data['features']:
            precinct_geo_list.append(precinct["properties"][json_id][-6:])
        for precinct_id, precinct_col in precinct_ids:
            if precinct_id in precinct_geo_list:
                if precinct_name_col:
                    precinct_list.append(Precinct(
                        precinct['geometry']['coordinates'],
                        precinct_name_col[precinct_col], state, precinct_id,
                        dem_cols[precinct_id], rep_cols[precinct_id]
                    ))
                else:
                    warnings.warn(f"Precinct with id {precinct_id} has no name")
                    precinct_list.append(Precinct(
                        precinct['geometry']['coordinates'],
                        "None", state, precinct_id,
                        dem_cols[precinct_id], rep_cols[precinct_id]
                    ))
            else:
                warnings.warn(f"Precinct with id {precinct_id} was not found in geodata.")

        # Saves precinct list to state file
        try:
            save(precinct_list[0].state, precinct_list)
        except IndexError:
            raise Exception("No precincts saved to precinct list.")

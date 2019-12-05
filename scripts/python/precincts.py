"""
Precinct-level maps from election-geodata compiled by Nathaniel Kelso and Michal Migurski.
Election data from harvard dataverse.

in election-geodata geojson files, "VTDST10" property is the same as harvard "ed_precinct
"""


import json
from os import mkdir
from os.path import abspath, dirname, isdir
import pickle


objects_dir = abspath(dirname(dirname(dirname(__file__)))) + '/data/objects'  # where the precinct data is stored


def save(precinct):
    """
    Save the data for a precinct object to a file
    """
    with open(f'{objects_dir}/{precinct.state}/{precinct.vote_id}', 'wb+') as f:
        pickle.dump(precinct, f)


def load(state, vote_id):
    """
    Load a precinct object that was saved to a file
    """
    with open(f'{objects_dir}/{state}/{vote_id}', 'r') as f:
        precinct = pickle.load(f)
    return precinct


def area(coords):
    """
    Returns the area of a poylgon with vertices at 2-d list `coords`
    """
    a = 0

    try:
        for i, j in zip(range(len(coords)), range(-1, len(coords) - 1)):
            a += (coords[j][0] + coords[i][0]) * (coords[j][1] - coords[i][1])
    except TypeError as e:
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

    args:
        `coords` - 2-d list of x and y coordinates of vertices
        `name` - name of precinct
        `state` - state that precinct is from
        `vote_id` - name from id system consistent between harvard and election-geodata files
        `d_election_data` - dict of name of vote to number of votes. i.e.
            {"g2002_GOV_dv": 100}
        `r_election_data` - above but for republicans.
    """

    def __init__(self, coords, name, state, vote_id,
                 d_election_data, r_election_data):
        
        # coordinate data
        self.coords = coords
        self.area = area(coords)
        
        # meta info for human purposes
        self.name = name
        self.vote_id = vote_id
        self.state = state

        # election data
        self.d_election_data = d_election_data
        d_election_sum = sum(list(d_election_data.values()))
        self.r_election_data = r_election_data
        r_election_sum = sum(list(r_election_data.values()))
        self.dem_rep_ratio = d_election_sum / r_election_sum

        self.dem_average = d_election_sum / len(list(d_election_data.values()))
        self.rep_average = d_election_sum / len(list(r_election_data.values()))

        # create directory for states if it doesn't already exist
        if not isdir(f'{objects_dir}/{self.state}'):
            mkdir(f'{objects_dir}/{self.state}')

    @classmethod
    def generate_from_files(cls, election_data, geojson, state):
        """
        Saves precinct objects from the harvard dataverse election data file and the election-geodata file to files in data/object directory
        """
        with open(geojson, 'r') as f:
            data = json.load(f)

        with open(election_data, 'r') as f:
            content = f.read().strip()
        
        data_rows = [row.split('\t') for row in content.split('\n')]
        data_columns = [[data_rows[x][y] for x in range(len(data_rows))]
                        for y in range(len(data_rows[0]))]
        data_dict = {column[0]: column[1:] for column in data_columns}
        dem_keys = [key for key in data_dict.keys() if key[-2:] == 'dv']
        rep_keys = [key for key in data_dict.keys() if key[-2:] == 'rv']
        dem_cols = {data_dict['ed_precinct'][i]: {key: convert_to_int(data_dict[key][i]) for key in dem_keys} for i in range(len(data_dict['ed_precinct']))}
        rep_cols = {data_dict['ed_precinct'][i]: {key: convert_to_int(data_dict[key][i]) for key in rep_keys} for i in range(len(data_dict['ed_precinct']))}


        for precinct in data['features']:
            precinct_id_geo = f'"{precinct["properties"]["VTDST10"]}"'
            for i, precinct_id_ele in enumerate(data_dict['ed_precinct']):
                if precinct_id_ele == precinct_id_geo:
                    save(Precinct(precinct['geometry']['coordinates'][0], data_dict['precinct'][i], state, precinct_id_geo, dem_cols[precinct_id_ele], rep_cols[precinct_id_ele]))


# TODO
# add comments
# add example file in objects folder
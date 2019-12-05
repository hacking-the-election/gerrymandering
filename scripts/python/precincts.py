# Precinct-level maps from election-geodata compiled by Nathaniel Kelso and Michal Migurski.
# Election data from harvard dataverse.

# in election-geodata geojson files, "VTDST10" property is the same as harvard "ed_precinct"


from os import mkdir
from os.path import dirname, isdir
import pickle


objects_dir = dirname(dirname(__file__)) + '/data/objects'  # where the precinct data is stored


def save(precinct):
    """
    Save the data for a precinct object to a file
    """
    with open(f'{objects_dir}/{precinct.state}/{precinct.vote_id}', 'w+') as f:
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
    area = 0

    for i, j in zip(range(len(coords)), range(-1, len(coords) - 1)):
        area += (coords[j][0] + coords[i][0]) * (coords[j][1] - coords[i][1])

    return area / 2


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
        self.rep_average = d_election_sum / len(list(r_elevtion_data.values()))

        # create directory for states if it doesn't already exist
        if not isdir(f'{objects_dir}/{self.state}'):
            mkdir(f'{objects_dir}/{self.state}')

    @classmethod
    def generate_from_files(cls, election_data, geojson):
        """
        Returns a list of precinct objects from the harvard dataverse election data file and the election-geodata file
        """


# TODO
# generate_from_files functions
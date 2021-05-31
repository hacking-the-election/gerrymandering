"""
A class representing a unit of voter
data in our project's data files: the precinct.
"""

class Precinct:
    """Class containing voter, population, and geo data for a single precinct.
    Should be pointed to by node in `networkx.Graph` object.

    :param rep_data: Republican voter counts corresponding to elections.
    :type rep_data: dict

    Example value for `rep_data`: 
    ``{"ush_r_06": 832, "uss_r_06": 123}``

    :param dem_data: Democrat voter counts corresponding to elections.
    :type dem_data: dict

    :param pop: Population of the precinct.
    :type pop: int

    :param coords: Geometric shape of precinct.
    :type coords: `shapely.geometry.Polygon`

    :param geoid: GEOID or similar identifier string for precinct.
    :type geoid: str
    """

    def __init__(self, pop, coords, state, precinct_id, total_data, rep_data, dem_data, scale_factor=0.02):

        self.pop = pop
        # should be shapely polygon
        self.coords = coords
        self.min_x, self.min_y, self.max_x, self.max_y = coords.bounds

        # x_length = self.max_x - self.min_x
        # y_length = self.max_y - self.min_y

        # self.min_x -= (x_length * scale_factor)
        # self.max_x += (x_length * scale_factor)
        # self.min_y -= (y_length * scale_factor)
        # self.max_y += (y_length * scale_factor)
        # centroid will be in list form, i.e. [35.274923, 71.47102]
        self.centroid = list(self.coords.centroid.coords[0])
        self.state = state
        self.id = precinct_id
        # used for data visualization
        self.community = None
        
        self.total_votes = total_data
        self.rep_votes = rep_data
        self.dem_votes = dem_data
        self.other_votes = None

        # number of parties with data
        # self.num_parties_data = 0

        if self.total_votes == 0:
            self.percent_dem = None
            self.percent_rep = None
            self.percent_other = None
        else:
            self.other_votes = self.total_votes - self.rep_votes - self.dem_votes
            self.percent_dem = self.dem_votes / self.total_votes
            self.percent_rep = self.rep_votes / self.total_votes
            self.percent_other = 1 - self.percent_dem - self.percent_rep


    @property
    def dem_rep_partisanship(self):
        """A value between -1 and 1, which represent democratic and republican, respectively.
        """

        try:
            republican = self.rep_votes / (self.rep_votes + self.dem_votes)
        except ZeroDivisionError:
            republican = 0
        if republican < 0.5:
            return republican - 1
        elif republican >= 0.5:
            return republican
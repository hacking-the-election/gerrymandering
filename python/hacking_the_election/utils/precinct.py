"""
A class representing the smallest unit of voter
data in our project's data files: the precinct.
"""


from hacking_the_election.utils.stats import standard_deviation


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

    def __init__(self, pop, coords, state, precinct_id, rep_data, dem_data, 
                 green_data=None, lib_data=None, reform_data=None, ind_data=None, const_data=None, other_data=None, scale_factor=0.02):

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
        
        # number of parties with data
        self.num_parties_data = 0

        try:
            self.total_rep = float(rep_data)
            self.num_parties_data += 1
        except ValueError:
            self.total_rep = 0
        try:
            self.total_dem = float(dem_data)
            self.num_parties_data += 1
        except ValueError:
            self.total_dem = 0
        # Green Party of the United States
        if green_data:
            try:
                self.total_green = float(green_data)
                self.num_parties_data += 1
            except ValueError:
                self.total_green = 0
        else:
            self.total_green = 0
        # Libertarian Party
        if lib_data:
            try:
                self.total_lib = float(lib_data)
                self.num_parties_data += 1
            except ValueError:
                self.total_lib = 0
        else:
            self.total_lib = 0
        # Reform Party of the United States of America
        if reform_data:
            try:
                self.total_reform = float(reform_data)
                self.num_parties_data += 1
            except ValueError:
                self.total_reform = 0
        else:
            self.total_reform = 0
        # Independent Party
        if ind_data:
            try:
                self.total_ind = float(ind_data)
                self.num_parties_data += 1
            except ValueError:
                self.total_ind = 0
        else:
            self.total_ind = 0
        # Constitution Party
        if const_data:
            try:
                self.total_const = float(const_data)
                self.num_parties_data += 1
            except ValueError:
                self.total_const = 0
        else:
            self.total_const = 0

        if other_data:
            try:
                self.total_other = float(other_data)
                self.num_parties_data += 1
            except ValueError:
                self.total_other = 0
        else:
            self.total_other = 0

        self.total_votes = (self.total_dem + self.total_rep + self.total_green
                          + self.total_lib + self.total_reform + self.total_ind
                          + self.total_const + self.total_other)
        if self.total_votes == 0:
            self.percent_dem = None
            self.percent_rep = None

            self.percent_green = None
            self.percent_lib = None
            self.percent_reform = None
            self.percent_ind = None
            self.percent_const = None

            self.percent_other = None
        else:

            self.percent_dem = self.total_dem / self.total_votes
            self.percent_rep = self.total_rep / self.total_votes

            self.percent_green = self.total_green / self.total_votes
            self.percent_lib = self.total_lib / self.total_votes
            self.percent_reform = self.total_reform / self.total_votes
            self.percent_ind = self.total_ind / self.total_votes
            self.percent_const = self.total_const / self.total_votes

            self.percent_other = self.total_other / self.total_votes

    @property
    def dem_rep_partisanship(self):
        """A value between -1 and 1, which represent democratic and republican, respectively.
        """

        try:
            republican = self.total_rep / (self.total_rep + self.total_dem)
        except ZeroDivisionError:
            republican = 0
        if republican < 0.5:
            return republican - 1
        elif republican >= 0.5:
            return republican
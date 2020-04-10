"""
A class representing the smallest unit of voter
data in our project's data files: the precinct.
"""
from stats import standard_deviation

class Precinct:
    """Class containing voter, population, and geo data for a single precinct.
    Should be pointed to by node in `pygraph.classes.graph.graph` object.

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
                 green_data=None, lib_data=None, reform_data=None, ind_data=None, const_data=None):

        self.pop = pop
        # should be shapely polygon
        self.coords = coords
        # centroid will be in list form, i.e. [35.274923, 71.47102]
        self.centroid = list(self.coords.centroid.coords[0])
        self.state = state
        self.id = precinct_id
        # used for data visualization
        self.community = None
        
        
        # number of parties with data
        self.num_parties_data = 0

        self.total_rep = rep_data
        self.total_dem = dem_data
        # Green Party of the United States
        if green_data:
            self.total_green = green_data
            self.num_parties_data += 1
        else:
            self.total_green = 0
        # Libertarian Party
        if lib_data:
            self.total_lib = lib_data
            self.num_parties_data += 1
        else:
            self.total_lib = 0
        # Reform Party of the United States of America
        if reform_data:
            self.total_reform = reform_data
            self.num_parties_data += 1
        else:
            self.total_reform = 0
        # Independents
        if ind_data:
            self.total_ind = ind_data
            self.num_parties_data += 1
        else:
            self.total_ind = 0
        # Constitution Party
        if const_data:
            self.total_const = const_data
            self.num_parties_data += 1
        else:
            self.total_const = 0

        self.total_votes = sum(self.total_dem + self.total_rep + self.total_green + self.total_lib + self.total_reform + self.total_ind + self.total_const)

        self.percent_dem = self.total_dem / self.total_votes
        self.percent_rep = self.total_rep / self.total_votes

        self.percent_green = self.total_green / self.total_votes
        self.percent_lib = self.total_lib / self.total_votes
        self.percent_reform = self.total_reform / self.total_votes
        self.percent_ind = self.total_ind / self.total_votes
        self.percent_const = self.total_const / self.total_votes
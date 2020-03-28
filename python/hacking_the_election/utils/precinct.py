"""
A class representing the smallest unit of voter
data in our project's data files: the precinct.
"""


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

    def __init__(self, rep_data, dem_data, pop, coords, geoid):

        self.rep_data = rep_data
        self.dem_data = dem_data
        
        total_rep = sum(self.rep_data.values())
        total_dem = sum(self.dem_data.values())
        try:
            self.partisanship = total_rep / (total_rep + total_dem)
        except ZeroDivisionError:
            # No voters in precinct.
            self.partisanship = 0

        self.pop = pop
        # should be shapely polygon
        self.coords = coords
        # centroid will be in list form, i.e. [35.274923, 71.47102]
        self.centroid = list(self.coords.centroid.coords[0])
        self.id = geoid

        self.community = None
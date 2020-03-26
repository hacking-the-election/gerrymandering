"""
A class representing the smallest unit of voter
data in our project's data files: the precinct.
"""


class Precinct:
    """
    Class containing precinct voter, population, and geodata.
    
    Should be pointed to by node in graph.Graph object.
    """

    def __init__(self, rep_data, dem_data, pop, coords, geoid):

        self.rep_data = rep_data
        self.dem_data = dem_data
        self.pop = pop
        # should be shapely polygon
        self.coords = coords
        # centroid will be in list form, i.e. [35.274923, 71.47102]
        self.centroid = list(coords.centroid)
        self.id = geoid
        # used for data visualization
        self.community = None
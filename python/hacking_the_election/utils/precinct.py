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
        self.coords = coords

        self.id = geoid
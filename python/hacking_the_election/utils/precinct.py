"""
A class representing the smallest unit of voter
data in our project's data files: the precinct.
"""


class Precinct:
    """
    Class containing precinct voter, population, and geodata.
    Also has a `node` attribute that can be used for graph
    theory related functions.
    """

    def __init__(self, rep_data, dem_data, pop, coords, geoid, node):

        self.rep_data = rep_data
        self.dem_data = dem_data
        self.pop = pop
        self.coords = coords

        self.id = geoid

        self.node = node
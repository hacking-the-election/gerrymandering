"""
Definitions for population refinement step in communities algorithm
"""

POPULATION_GIVE_PRECINCT_KWARGS = {
    "coords": True,
    "partisanship": False,
    "standard_deviation": False,
    "population": True,
    "compactness": False,
    "allow_zero_precincts": False,
    "allow_multipolygons": False
}


class PopulationRange:
    """
    Continuous range of numbers between two values.
    """

    def __init__(self, lower, upper):
        self.upper = upper
        self.lower = lower

    def __contains__(self, key):
        return key <= self.upper and key >= self.lower

    def __str__(self):
        return f"{int(self.lower)} - {int(self.upper)}"
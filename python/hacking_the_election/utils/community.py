"""
A class representing political communities
"""


class Community:
    """Political communities - groups of precincts.

    :param community_id: Identifier for this community.
    :type community_id: object
    """

    def __init__(self, community_id):

        self.id = community_id

        self.precincts = {}  # Dict maps precinct ids to precinct objects.
        self.partisanship = 0
        self.partisanship_stdev = 0
        self.compactness = 0
        self.population = 0
        self.coords = None

    def take_precinct(self, precinct):
        """Adds a precinct to this community.
        
        :param precinct: The precinct to add to this community.
        :type precinct: class:`hacking_the_election.utils.precinct.Precinct`
        """

        self.precincts[precinct.id] = precinct
        precinct.community = self.id

    def give_precinct(self, other, precinct_id):
        """Gives a precinct to a different community.

        :param other: The community to give the precinct to.
        :type other: class:`hacking_the_election.utils.community.Community`

        :param precinct_id: The id of the precinct to give to the other community. (Must be in self.precincts.keys())
        :type precinct_id: str
        """

        try:
            precinct = self.precincts[precinct_id]
        except KeyError:
            raise ValueError(f"Precinct {precinct_id} not in community {self.id}")
        del self.precincts[precinct_id]
        other.take_precinct(precinct)
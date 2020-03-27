"""
A class representing political communities
"""


class Community:
    """Political communities - groups of precincts.
    """

    def __init__(self, precincts, community_id):

        self.precincts = {precinct.id: precinct for precinct in precincts}
        self.id = community_id
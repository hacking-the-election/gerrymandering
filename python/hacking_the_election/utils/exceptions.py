"""
Custom exceptions.
"""


class LoopBreakException(Exception):
    """
    Used to break outer loops from within nested loops.
    """

    def __init__(self, level=0):
        self.level = level


class CommunityFillCompleteException(Exception):
    """
    Used to break out of all levels of
    recursion when a community is filled.
    """

    def __init__(self, unchosen_precincts, unchosen_precincts_border):
        self.unchosen_precincts = unchosen_precincts
        self.unchosen_precincts_border = unchosen_precincts_border
        Exception.__init__(self)


class ZeroPrecinctCommunityException(Exception):
    """
    Used when a community is attempting to give its last precinct.
    """


class CreatesMultiPolygonException(Exception):
    """
    Used when a precinct transaction turns
    either community into a MultiPolygon.
    """
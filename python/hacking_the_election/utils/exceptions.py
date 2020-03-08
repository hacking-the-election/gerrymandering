"""
Custom exceptions.
"""


class LoopBreakException(Exception):
    """
    Used to break outer loops from within nested loops.
    """


class CommunityFillCompleteException(Exception):
    """
    Used to break out of all levels of recursion when a community is filled.
    """

    def __init__(self, unchosen_precincts, unchosen_precincts_border):
        self.unchosen_precincts = unchosen_precincts
        self.unchosen_precincts_border = unchosen_precincts_border
        Exception.__init__(self)
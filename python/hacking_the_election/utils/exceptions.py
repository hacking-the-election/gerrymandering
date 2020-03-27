"""Custom exceptions
"""


class CommunityCompleteException(Exception):
    """To be raised when the backtracking algorithm has
    found a solution for a single community.
    """
"""
Functions for partisanship refinement step in communities algorithm
"""


import random

from hacking_the_election.utils.exceptions import (
    CreatesMultiPolygonException,
    ZeroPrecinctCommunityException
)
from hacking_the_election.utils.geometry import get_if_bordering


GIVE_PRECINCTS_COMPACTNESS_KWARGS = {
    "partisanship": False,
    "standard_deviation": False,
    "population": False, 
    "compactness": True, 
    "coords": True
}


def get_average_compactness(communities):
    """
    Returns average schwartzberg compactness value of community in
    `communities`. Does not automatically update values.
    """
    compactness_values = [c.compactness for c in communities]
    return sum(compactness_values) / len(compactness_values)


def format_float(n):
    """
    Returns float with 3 places to the left and 3 to the right of the
    decimal point.
    """

    n_chars = list(str(round(n, 3)))

    # Add zeroes on right side.
    n_right_digits = len("".join(n_chars).split(".")[-1])
    if n_right_digits < 3:
        for i in range(3 - n_right_digits):
            n_chars.append("0")
    
    # Add zeroes on left side.
    n_left_digits = len("".join(n_chars).split(".")[0])
    if n_left_digits < 3:
        for i in range(3 - n_left_digits):
            n_chars.insert(0, "0")

    return "".join(n_chars)


def add_precinct(communities, community, precinct, give):
    """
    This function probably does something.

    I don't really care,
    because nobody except myself will read this code anyway.
    """
    if give:
        other_community = \
            random.choice(
                [c for c in communities
                 if (get_if_bordering(c.coords, precinct.coords)
                     and c != community)]
            )
        community.give_precinct(
            other_community,
            precinct.vote_id,
            **GIVE_PRECINCTS_COMPACTNESS_KWARGS
        )
    else:
        for other_community in communities:
            if precinct in other_community.precincts.values():
                other_community.give_precinct(
                    community,
                    precinct.vote_id,
                    **GIVE_PRECINCTS_COMPACTNESS_KWARGS
                )
                
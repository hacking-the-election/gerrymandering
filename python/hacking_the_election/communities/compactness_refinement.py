"""
Refines a state broken into communities
so that they are compact within a threshold.
"""


from hacking_the_election.utils.compactness import get_average_compactness
from hacking_the_election.utils.initial_configuration import LoopBreakException


def refine_for_compactness(communities, minimum_compactness):
    """
    Returns communities that are all below the minimum compactness.
    """

    for community in communities:
        community.update_compactness()

    try:
        for community in communities:
            # Find outer circle for community

            # Find precincts that are outside of this circle

            while True:
                # Add precincts one by one
                if get_average_compactness(communities) > minimum_compactness:
                    raise LoopBreakException
    except LoopBreakException:
        return communities
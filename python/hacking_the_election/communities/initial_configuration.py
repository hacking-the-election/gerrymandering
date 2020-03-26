"""Randomly groups precincts in a state to communities of equal (or as close as possible) size.
"""


def create_initial_configuration(precinct_graph, precincts, c_communities):
    """Randomly groups precincts in a state to communities of equal (or as close as possible) size.
    
    :param precinct_graph: Graph containing all precincts in state as nodes with precinct ids as attributes.
    :type precinct_graph: class:`pygraph.classes.graph.graph`

    :param precincts: List of all precincts in state.
    :type precincts: List of class:`hacking_the_election.utils.precinct.Precinct`

    :param n_districts: Number of groups of precincts to return.
    :type n_districts: `int`

    :return: A list of communities. Each is a contiguous shape with a number of precincts within 1 of that of all the other communities.
    :rtype: list of class:`hacking_the_election.utils.community.Community`
    """
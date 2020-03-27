"""Randomly groups precincts in a state to communities of equal (or as close as possible) size.
"""


from hacking_the_election.utils.community import Community
from hacking_the_election.utils.exceptions import CommunityCompleteException
from hacking_the_election.utils.graph import get_discontinuous_components, remove_edges_to


def _create_community(graph, group_size, group):
    """Implements backtracking algorithm for precinct choosing a group of precincts that doesnt make the group of unchosen precincts discontiguous.

    :param graph: The graph from which to choose a group of precincts. Should not contain any precincts that belong to another community.
    :type graph: `pygraph.classes.graph.graph`

    :param group_size: The number of precincts that are to be in this group.
    :type group_size: int

    :param group: set that precincts should be added to.
    :type group: set of int
    """

    graph_nodes = graph.nodes()

    # Precincts that can be added without separating the graph.
    eligible_precincts = set()
    for node in graph_nodes:
        eligibility_conditions = [
            node not in group,
            len(get_discontinuous_components(graph)) == len(group) + 1
        ]
        if all(eligibility_conditions):
            # Logic for second condition:
            # Each time a precinct is added to the group,
            # it is completely disconnected from the graph.
            # This means that the number of components should always be
            # the number of precincts in the group + 1.
            eligible_precincts.add(node)
    
    tried_precincts = set()
    while True:
        try:
            selected_precinct = min(
                eligible_precincts - tried_precincts
            )
        except ValueError:
            # All eligible precincts have been tried.
            return
        
        group.append(selected_precinct)
        removed_edges = remove_edges_to(selected_precinct, graph)
        if len(group) == group_size:
            raise CommunityCompleteException
        _create_community(graph)
        # Above call didn't raise error, so the algorithm has
        # backtracked and the value at this step needs to be changed.

        # Undo edge removal.
        for edge in removed_edges:
            graph.add_edge(edge)
        group.remove(selected_precinct)
        tried_precincts.add(selected_precinct)



def create_initial_configuration(precinct_graph, precincts, n_communities):
    """Randomly group precincts in a state creating contiguous shapes.
    
    :param precinct_graph: Graph containing all precincts in state as nodes with precinct ids as attributes.
    :type precinct_graph: class:`pygraph.classes.graph.graph`

    :param precincts: List of all precincts in state.
    :type precincts: List of class:`hacking_the_election.utils.precinct.Precinct`

    :param n_communities: Number of groups of precincts to return.
    :type n_communities: `int`

    :return: A list of communities. Each is a contiguous shape with a number of precincts within 1 of that of all the other communities.
    :rtype: list of class:`hacking_the_election.utils.community.Community`
    """
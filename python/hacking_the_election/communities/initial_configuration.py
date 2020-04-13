"""Randomly groups precincts in a state to communities of equal (or as close as possible) size.
"""


from copy import deepcopy

from hacking_the_election.utils.community import Community
from hacking_the_election.utils.exceptions import CommunityCompleteException
from hacking_the_election.utils.graph import (
    get_discontinuous_components,
    remove_edges_to
)


def _create_community(graph, group_size, group, calls):
    """Implements backtracking algorithm for precinct choosing a group of precincts that doesnt make the group of unchosen precincts discontiguous.

    :param graph: The graph from which to choose a group of precincts. Should not contain any precincts that belong to another community.
    :type graph: `pygraph.classes.graph.graph`

    :param group_size: The number of precincts that are to be in this group.
    :type group_size: int

    :param group: set that precincts should be added to.
    :type group: set of int

    :param calls: a variable that stores the number of times this function was called.
    :type calls: list
    """
    calls.append(None)

    graph_nodes = graph.nodes()

    # Precincts that can be added without separating the graph.
    eligible_precincts = set()
    for node in graph_nodes:

        if node in group:
            continue

        # This is temporary.
        # Just to see if removing this precinct would make an island.
        removed_edges = remove_edges_to(node, graph)
        if get_discontinuous_components(graph) == len(group) + 2:
            # Logic for second condition:
            # Each time a precinct is added to the group,
            # it is completely disconnected from the graph.
            # This means that the number of components should always be
            # the number of precincts in the group + 2.
            eligible_precincts.add(node)
        for edge in removed_edges:
            graph.add_edge(edge)

    tried_precincts = set()
    # While not all eligible precincts have been tried.
    while tried_precincts != eligible_precincts:
        selected_precinct = min(
            eligible_precincts - tried_precincts
        )
        
        group.add(selected_precinct)
        removed_edges = remove_edges_to(selected_precinct, graph)
        if len(group) == group_size:
            raise CommunityCompleteException
        _create_community(graph, group_size, group, calls)
        # Above call didn't raise error, so the algorithm has
        # backtracked and the value at this step needs to be changed.

        # Undo edge removal.
        for edge in removed_edges:
            graph.add_edge(edge)
        group.remove(selected_precinct)
        tried_precincts.add(selected_precinct)

    # Backtrack (goes back down to lower levels of recursion):
    return


def create_initial_configuration(precinct_graph, n_communities):
    """Randomly group precincts in a state creating contiguous shapes.
    
    :param precinct_graph: Graph containing all precincts in state as nodes with precinct ids as attributes.
    :type precinct_graph: class:`pygraph.classes.graph.graph`

    :param n_communities: Number of groups of precincts to return.
    :type n_communities: `int`

    :return: A list of communities. Each is a contiguous shape with a number of precincts within 1 of that of all the other communities. Also the number of times _create_community was called.
    :rtype: list of class: `hacking_the_election.utils.community.Community` and int
    """
    
    calls = []

    # Nothing having to do with islands is required here,
    # since that is taken care of in the creation of hte graph.
    s = len(precinct_graph.nodes()) // n_communities
    community_sizes = [s for _ in range(n_communities)]
    for i in range(len(precinct_graph.nodes()) % n_communities):
        community_sizes[i] += 1

    # Sets of node identifiers for each community
    community_node_groups = [set() for _ in range(n_communities)]
    # Call create_community `n_communities` - 1 times.
    # The remaining nodes belong to the last community.
    tmp_graph = deepcopy(precinct_graph)
    for community_node_group, community_size in zip(community_node_groups[:-1],
                                                    community_sizes[:-1]):
        try:
            _create_community(tmp_graph, community_size, community_node_group, calls)
        except CommunityCompleteException:
            pass
        for node in community_node_group:
            tmp_graph.del_node(node)
    
    # Last community.
    for node in tmp_graph.nodes():
        community_node_groups[-1].add(node)
    
    # The final output list of `Community` objects.
    communities = []
    for i, community_node_group in enumerate(community_node_groups):
        community = Community(i)
        for node in community_node_group:
            community.take_precinct(precinct_graph.node_attributes(node)[0])
        communities.append(community)

    return communities, len(calls)
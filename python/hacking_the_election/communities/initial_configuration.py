"""Randomly groups precincts in a state to communities of equal (or as close as possible) size.
"""


from copy import deepcopy

from pygraph.classes.graph import graph
from pygraph.classes.exceptions import AdditionError

from hacking_the_election.utils.community import Community
from hacking_the_election.utils.exceptions import CommunityCompleteException
from hacking_the_election.utils.graph import (
    get_discontinuous_components,
    remove_edges_to
)


def _back_track(G, selected, G2, last_group_len, group_size):
    """Implementation of backtracking algorithm to create contiguous groups out of graph of precincts.

    :param G: The graph containing the precinct objects.
    :type G: `pygraph.classes.graph.graph`

    :param selected: A list of nodes that will create contiguous groups when split into groups of length `group_size`.
    :type selected: list of int

    :param G2: The graph after removing the last selected node.
    :type G2: `pygraph.classes.graph.graph`

    :param last_group_len: The number of nodes selected for the last group.
    :type last_group_len: int

    :param group_size: Maximum allowed size for a group.
    :type group_size: int
    """

    if len(selected) == len(G.nodes()):
        raise CommunityCompleteException

    if last_group_len == group_size:
        # Start a new group
        last_group_len = 0
    # Check continuity of remaining part of graph
    if get_discontinuous_components(G2) > len(selected) + 2:
        return
    
    available = []  # Nodes that can be added to the current group.
    if last_group_len == 0:
        # Can choose any node that is not already selected.
        available = [node for node in G.nodes() if node not in selected]
    else:
        # Find all nodes bordering current group.
        for node in selected[-last_group_len:]:
            for neighbor in G.neighbors(node):
                if neighbor not in available and neighbor not in selected:
                    available.append(neighbor)

    if len(available) == 0:
        return
    
    for node in sorted(available):
        selected.append(node)
        _back_track(G, selected, remove_edges_to(node, G2),
                    last_group_len + 1, group_size)
        selected.remove(node)


def create_initial_configuration(precinct_graph, n_communities):
    """Produces a list of contiguous communities based off of a state represented by a graph of precincts.

    :param precinct_graph: A graph containing all the precincts in a state. Edges exist between bordering precincts.
    :type precinct_graph: `pygraph.classes.graph.graph`

    :param n_communities: The number of communities to group the precincts into.
    :type n_communities: int

    :return: A list of communities that contain groups of bordering precincts.
    :type: list of `hacking_the_election.utils.community.Community`
    """

    n_precincts = len(precinct_graph.nodes())
    if n_precincts % n_communities == 0:
        group_size = n_precincts / n_communities
    else:
        group_size = (n_precincts // n_communities) + 1
    group_size = int(group_size)
    
    light_graph = graph()  # A graph without the node attributes of heavy precinct objects.
    for node in precinct_graph.nodes():
        light_graph.add_node(node)
    for edge in precinct_graph.edges():
        try:
            light_graph.add_edge(edge)
        except AdditionError:
            pass
    
    selected = []
    try:
        _back_track(light_graph, selected, deepcopy(light_graph), 0, group_size)
    except CommunityCompleteException:
        pass

    node_groups = [selected[i:i + group_size] for i in range(0, len(selected), group_size)]
    precinct_groups = [[precinct_graph.node_attributes(node)[0] for node in group]
                       for group in node_groups]
    communities = []
    for i, group in enumerate(precinct_groups):
        community = Community(i)
        for precinct in group:
            community.take_precinct(precinct)
        communities.append(community)
    return communities
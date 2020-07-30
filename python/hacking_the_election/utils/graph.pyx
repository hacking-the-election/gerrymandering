"""
Graph theory related functions and classes.
"""

import copy

from pygraph.classes.graph import graph as Graph
from pygraph.classes.digraph import digraph as DirectedGraph
from pygraph.classes.exceptions import AdditionError


cdef float _get_community_pop(community):
    community.update_population()
    return float(community.population)


cdef float _get_precinct_xcoord(tuple precinct_tuple):
    return precinct_tuple[0].centroid[0]


cpdef light_copy(G):
    """Returns a graph without node attributes.

    :param G: A graph containing node and edge data.
    :type G: `pygraph.classes.graph.graph`

    :return: A graph with equivalent node and edge data to `graph`, but without any node attributes.
    :rtype: `pygraph.classes.graph.graph`
    """

    G2 = Graph()

    cdef int v
    for v in G.nodes():
        G2.add_node(v)
    cdef (int, int) e
    for e in G.edges():
        if not G2.has_edge(e):
            G2.add_edge(e)
    return G2


cpdef void contract(G, tuple t):
    """Contracts an edge in a graph.

    :param G: Graph containing edge `t`
    :type G: `pygraph.classes.graph.graph`

    :param t: Edge within `G`.
    :type t: (int, int)
    """
    
    cdef list new_node_attributes = []
    cdef int v
    cdef list v_attributes
    for v in t:
        v_attributes = G.node_attributes(v)
        if len(v_attributes) != 0:
            new_node_attributes += v_attributes
        else:
            new_node_attributes.append(v)
    
    cdef list new_node_neighbors = list(set(G.neighbors(t[0]))
                                      | set(G.neighbors(t[1])))
    new_node_neighbors.remove(t[0]); new_node_neighbors.remove(t[1])
    
    G.del_edge(t)
    G.del_node(t[0]); G.del_node(t[1])

    cdef list nodes = G.nodes()
    cdef int new_node
    if len(nodes) != 0:
        new_node = max(nodes) + 1
    else:
        new_node = 0
    G.add_node(new_node, attrs=new_node_attributes)
    cdef int neighbor
    for neighbor in new_node_neighbors:
        G.add_edge((new_node, neighbor))


cdef void _dfs(graph, set nodes, int v):
    """Finds all the nodes in a component of a graph containing a start node.

    Implementation of the depth-first search algorithm translated from wikipedia:
    https://en.wikipedia.org/wiki/Depth-first_search#Pseudocode

    :param graph: The graph to traverse.
    :type graph: `pygraph.classes.graph.graph`

    :param nodes: a set that will have nodes added to it as they are traversed.
    :type nodes: set of int

    :param v: The start node in the search tree
    :type v: int
    """
    nodes.add(v)
    cdef int w
    cdef list neighbors
    neighbors = graph.neighbors(v)
    for w in neighbors:
        if w not in nodes:
            _dfs(graph, nodes, int(w))


cpdef list get_components(graph):
    """Finds number of discontinuous components of a graph.

    :param graph: The graph that contains `node`
    :type graph: `pygraph.classes.graph.graph`

    :return: The number of discontinuous components in `graph`.
    :rtype: int
    """

    cpdef list components = []

    cdef set discovered_nodes = set()
    cdef set graph_nodes = set(graph.nodes())

    # While not all nodes have been discovered.
    while discovered_nodes != graph_nodes:
        component = set()
        # Search all nodes in `start_v`'s component.
        _dfs(graph, component, int(min(graph_nodes - discovered_nodes)))
        components.append(list(component))
        discovered_nodes.update(component)
    return components


cpdef get_induced_subgraph(graph, list precincts):
    """Creates an induced subgraph of a graph based off of a list of precincts in the graph.

    :param graph: A graph with precincts as node attributes.
    :type graph: `pygraph.classes.graph.graph`

    :param precincts: A list of precincts which are attributes in `graph`
    :type precincts: list of `hacking_the_election.utils.precicnt.Precinct`

    :return: A graph that is the induced subgraph of `graph`, containing `precincts` as nodes.
    :rtype: `pygraph.classes.graph.graph`
    """

    cdef list nodes = [precinct.node for precinct in precincts]
    cdef list edges = []

    cdef int node
    cdef int neighbor
    for node in nodes:
        for neighbor in graph.neighbors(node):
            if neighbor in nodes:
                edges.append((node, neighbor))
    
    induced_subgraph = Graph()
    for node in nodes:
        induced_subgraph.add_node(node)
    cdef tuple edge
    for edge in edges:
        try:
            induced_subgraph.add_edge(edge)
        except AdditionError:
            pass

    return induced_subgraph


cpdef _get_dfs_spanning_tree(graph, spanning_tree, int v, parent=None, int depth=0):
    """Performs the DFS algorithm and finds the spanning tree of the traversal.

    :param graph: A graph on which to perform a dfs traversal.
    :type graph: `pygraph.classes.graph.graph`

    :param spanning_tree: The directed graph which will be used as the spanning tree.
    :type spanning_tree: `pygraph.classes.digraph.digraph`

    :param v: The start node.
    :type v: int

    :param parent: The parent node of the inputted start node, defaults to None (for root node of entire search)
    :type parent: int or NoneType

    :param depth: The current depth of recursion, defaults to 0.
    :type depth: int
    """
    spanning_tree.add_node(v)
    spanning_tree.add_node_attribute(v, depth)
    if parent is not None:
        spanning_tree.add_edge((parent, v))

    cdef int u
    for u in graph.neighbors(v):
        if u not in spanning_tree.nodes():
            _get_dfs_spanning_tree(graph, spanning_tree, u, v, depth + 1)
        else:
            if u not in spanning_tree.incidents(v):
                spanning_tree.add_edge((v, u))
    
    # Calculate and store lowpoint.
    cdef set descendants = set()
    cdef int descendant
    _dfs(spanning_tree, descendants, v)
    cdef int lowpoint = min(
        [spanning_tree.node_attributes(descendant)[0]
         for descendant in descendants])
    spanning_tree.add_node_attribute(v, lowpoint)


cpdef set get_articulation_points(graph):
    """Finds the set of articulation points within a graph.

    :param graph: A graph.
    :type graph: `pygraph.classes.graph.graph`

    :return: The set of articulation points in the graph.
    :rtype: set of int
    """
    cdef int v = graph.nodes()[0]
    cdef int root = v
    cdef int descendant
    cdef set descendants
    
    spanning_tree = DirectedGraph()
    _get_dfs_spanning_tree(graph, spanning_tree, v)  # Populate tree.

    # Get depth and lowpoint of each node.
    cdef dict depths = {}
    cdef dict lowpoints = {}

    cdef list node_attributes
    cdef int node
    for node in spanning_tree.nodes():
        node_attributes = spanning_tree.node_attributes(node)
        depths[node] = node_attributes[0]
        lowpoints[node] = node_attributes[1]
    
    # Find articulaiton points.
    cdef set articulation_points = set()
    cdef int child
    for node in spanning_tree.nodes():
        if node != root:
            for child in spanning_tree.neighbors(node):
                if lowpoints[child] >= depths[node]:
                    articulation_points.add(node)
        else:
            if len(spanning_tree.neighbors(node)) > 1:
                articulation_points.add(node)
    
    return articulation_points


cdef _remove_edges_to(graph, int node):
    """Removes edges connected to a node.

    :param graph: A graph.
    :type graph: `pygraph.classes.graph.graph`

    :param node: A node.
    :type node: int

    :return: A copy of `graph` with all edges connected to `node` removed.
    :rtype: `pygraph.classes.graph.graph`
    """

    new_graph = light_copy(graph)
    cdef tuple edge
    for edge in new_graph.edges():
        if node in edge:
            try:
                new_graph.del_edge(edge)
            except ValueError:
                pass
    return new_graph


cpdef list get_giveable_precincts(state_graph, list communities, int community):
    """Finds all precincts that can be given to another community.

    :param state_graph: A graph containing all precincts in a state.
    :type state_graph: `pygraph.classes.graph.graph`

    :param communities: All the communities in a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param community: ID of the community that is taking the nodes.
    :type community: int

    :return: A list of tuples each containing a giveable precinct and a community it can be given to.
    :rtype: list of tuple containing `hacking_the_election.utils.precinct.Precinct` and `hacking_the_election.utils.community.Community`
    """

    cpdef list giveable_precincts = []

    cdef dict community_dict = {c.id: c for c in communities}

    cpdef community_graph = community_dict[community].induced_subgraph
    cdef list community_nodes = community_graph.nodes()
    cdef list node_neighbors = []
    cdef int node
    cdef int neighbor
    cdef int neighbor_community_id
    cpdef node_precinct
    cpdef other_community

    for node in community_nodes:
        node_neighbors = state_graph.neighbors(node)
        if not all([neighbor in community_nodes for neighbor in node_neighbors]):
            # The node is bordering another community.
            node_precinct = state_graph.node_attributes(node)[0]

            # Get a community that is bordering `community`
            for neighbor in node_neighbors:
                neighbor_community_id = \
                    state_graph.node_attributes(neighbor)[0].community
                if neighbor_community_id != community:
                    other_community = community_dict[neighbor_community_id]

            giveable_precincts.append((node_precinct, other_community))
    
    return giveable_precincts


cpdef list get_takeable_precincts(state_graph, list communities, int community):
    """Finds all the precincts in a state that can be taken by a community.

    :param state_graph: A graph containing all precincts in a state.
    :type state_graph: `pygraph.classes.graph.graph`

    :param communities: All the communities in a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param community: ID of the community that is taking the nodes.
    :type community: int

    :return: A list of tuples containing a takeable precinct and the community it is in.
    :rtype: list of tuples of `hacking_the_election.utils.precinct.Precinct` and `hacking_the_election.utils.community.Community`.
    """

    cdef set takeable_precincts_set = set()
    cpdef takeable_precincts = []

    # Map ids to community objects
    cdef dict community_dict = {c.id: c for c in communities}

    cdef list community_nodes = community_dict[community].induced_subgraph.nodes()
    cdef int node
    cdef int neighbor
    cpdef neighbor_precinct
    cpdef other_community_subgraph
    for node in community_nodes:
        for neighbor in state_graph.neighbors(node):
            if neighbor not in community_nodes:
                # `neighbor` is a bordering precinct from another community.
                neighbor_precinct = state_graph.node_attributes(neighbor)[0]
                other_community = community_dict[neighbor_precinct.community]
                takeable_precincts_set.add((neighbor_precinct, other_community))

    takeable_precincts = list(takeable_precincts_set)

    return takeable_precincts


def graph_from_file(file_path, precinct_list):
    """Creates a graph from a specifically formatted text file. (Used for debugging.)

    :param file_path: path to file (.txt)
    :type file_path: str

    :param precinct_list: list of precincts to attach to nodes, ordered with nodes
    :type precinct_list: list of hacking_the_election.utils.precinct.Precinct objects

    :return: graph
    :rtype: pygraph.classes.graph.graph
    """
    return_graph = Graph()
    with open(file_path, "r") as f:
        file_graph = f.read().strip()
    nodes = eval((file_graph.split(']')[0] + ']'))
    edges = eval((file_graph.split(']')[1] + ']'))
    for node in nodes:
        return_graph.add_node(node, attrs=[precinct_list[node]])
    for edge in edges:
        return_graph.add_edge(edge)
    return return_graph
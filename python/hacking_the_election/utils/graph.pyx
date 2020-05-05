"""
Graph theory related functions and classes.
"""

import copy

from pygraph.classes.graph import graph as Graph
from pygraph.classes.exceptions import AdditionError


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


cpdef int get_node_number(precinct, graph):
    """Returns the node number of precinct that is in the node_attributes of a graph.

    :param precinct: Precinct to find node number of.
    :type precinct: `hacking_the_election.utils.precinct.Precinct`

    :param graph: Graph that contains `precinct` in node_attributes.
    :type graph: `pygraph.classes.graph.graph`

    :return: The node that contains `precinct` as an attribute.
    :rtype: int
    """

    cdef int node
    for node in graph.nodes():
        if precinct in graph.node_attributes(node):
            return node
    raise ValueError("Precinct not part of inputted graph.")


cpdef void _dfs(graph, set nodes, int v):
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
        _dfs(graph, component, int(min(graph_nodes -  discovered_nodes)))
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

    cdef list nodes = [get_node_number(precinct, graph) for precinct in precincts]
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
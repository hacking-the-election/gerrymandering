"""
Graph theory related functions and classes.
"""


from pygraph.classes.graph import graph as Graph
from pygraph.classes.exceptions import AdditionError


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


cpdef remove_edges_to(int node, graph):
    """Returns graph with all edges removed to a given node.

    :param node: The node to remove the edges from.
    :type node: int

    :param graph: The graph that contains `node`
    :type graph: `pygraph.classes.graph.graph`

    :return: A graph that is equivalent to `graph`, except all the edges connected to `node`.
    :rtype: `pygraph.classes.graph.graph`
    """

    new_graph = Graph()
    cdef int v
    for v in graph.nodes():
        new_graph.add_node(int(v))
    cdef tuple edge
    for edge in graph.edges():
        try:
            new_graph.add_edge(tuple([int(i) for i in edge]))
        except AdditionError:
            pass

    for edge in new_graph.edges()[:]:
        if node in edge:
            try:
                new_graph.del_edge(edge)
            except ValueError:
                # Edge was already deleted because it is a duplicate
                # for the sake of undirected edges.
                pass

    return new_graph


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
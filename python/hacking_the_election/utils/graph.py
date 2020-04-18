"""
Graph theory related functions and classes.
"""


from pygraph.classes.graph import graph as Graph
from pygraph.classes.exceptions import AdditionError


def remove_edges_to(node, graph):
    """Returns graph with all edges removed to a given node.

    :param node: The node to remove the edges from.
    :type node: int

    :param graph: The graph that contains `node`
    :type graph: `pygraph.classes.graph.graph`

    :return: A graph that is equivalent to `graph`, except all the edges connected to `node`.
    :rtype: `pygraph.classes.graph.graph`
    """

    new_graph = Graph()
    for vertex in graph.nodes():
        new_graph.add_node(vertex)
    for edge in graph.edges():
        try:
            new_graph.add_edge(edge)
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


def dfs(graph, nodes, v):
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
    for w in graph.neighbors(v):
        if w not in nodes:
            dfs(graph, nodes, w)


def get_components(graph):
    """Finds number of discontinuous components of a graph.

    :param graph: The graph that contains `node`
    :type graph: `pygraph.classes.graph.graph`

    :return: The number of discontinuous components in `graph`.
    :rtype: int
    """

    components = []

    discovered_nodes = set()
    graph_nodes = set(graph.nodes())

    # While not all nodes have been discovered.
    while discovered_nodes != graph_nodes:
        component = set()
        # Search all nodes in `start_v`'s component.
        dfs(graph, components, min(graph_nodes -  discovered_nodes))
        components.append(component)
        discovered_nodes.update(component)
    return components
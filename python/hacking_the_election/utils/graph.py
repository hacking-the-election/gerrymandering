"""
Graph theory related functions and classes.
"""


def remove_edges_to(node, graph):
    """Returns and removes all edges connected to a node. In-place on a graph.

    :param node: The node to remove the edges from.
    :type node: int

    :param graph: The graph that contains `node`
    :type graph: `pygraph.classes.graph.graph`
    """
    removed_edges = []
    for edge in graph.edges():
        if node in edge:
            try:
                graph.del_edge(edge)
                removed_edges.append(edge)
            except ValueError:
                pass
    return removed_edges


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


def get_discontinuous_components(graph):
    """Finds number of discontinuous components of a graph.

    :param graph: The graph that contains `node`
    :type graph: `pygraph.classes.graph.graph`

    :return: The number of discontinuous components in `graph`.
    :rtype: int
    """

    n_components = 0

    discovered_nodes = set()
    graph_nodes = set(graph.nodes())

    # While not all nodes have been discovered.
    while discovered_nodes != graph_nodes:
        # Search all nodes in `start_v`'s component.
        dfs(graph, discovered_nodes, min(graph_nodes -  discovered_nodes))
        n_components += 1

    return n_components
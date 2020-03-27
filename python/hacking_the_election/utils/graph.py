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
            removed_edges.append(edge)
            graph.del_edge(edge)
    return removed_edges


def get_discontinuous_components(graph):
    """Finds number of discontinuous components of a graph. Implements breadth-first search algorithm.

    Breadth-first algorithm implementation translated from pseudocode found here: https://en.wikipedia.org/wiki/Breadth-first_search

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
        start_v = min(graph_nodes -  discovered_nodes, 1)

        # Search all nodes in `start_v`'s component.
        queue = []
        discovered_nodes.add(start_v)
        queue.append(start_v)
        while queue != []:
            v = queue.pop(0)
            for w in graph.neighbors(v):
                if w not in discovered_nodes:
                    discovered_nodes.add(w)
                    queue.append(w)
        
        n_components += 1

    return n_components
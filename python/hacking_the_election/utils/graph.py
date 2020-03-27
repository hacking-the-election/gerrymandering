"""
Graph theory related functions and classes.
"""


import random
from copy import deepcopy


def remove_all_edges(node, graph):
    """Removes all edges connected to a node. 

    :param node: The node to remove the edges from.
    :type node: int

    :param graph: The graph that contains `node`
    :type graph: `pygraph.classes.graph.graph`

    :return: Graph with all edges connected to `node` removed.
    :rtype: `pygraph.classes.graph.graph`
    """

    edges_removed_graph = deepcopy(graph)

    for edge in edges_removed_graph.edges():
        if node in edge:
            edges_removed_graph.del_edge(edge)

    return edges_removed_graph


def _get_component(tree_root, graph):
    """Finds all the nodes in the same component of a graph as a specific node. Implements breadth-first search algorithm.

    Implementation translated from pseudocode found here: https://en.wikipedia.org/wiki/Breadth-first_search
    
    :param tree_root: The node to find the component of.
    :type tree_root: int

    :param graph: The graph that contains `node`
    :type graph: `pygraph.classes.graph.graph`

    :return: The set of nodes that are in the same component of `graph` as `tree_root`
    :rtype: set of int
    """

    


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
        start_v = random.sample(graph_nodes -  discovered_nodes, 1)[0]

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
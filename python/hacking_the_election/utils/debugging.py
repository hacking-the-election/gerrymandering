# Functions to help with the debugging of programs

from pygraph.classes.graph import graph

def graph_from_file(file_path, precinct_list):
    """
    :param file_path: path to file (.txt)
    :type file_path: str

    :param precinct_list: list of precincts to attach to nodes, ordered with nodes
    :type precinct_list: list of hacking_the_election.utils.precinct.Precinct objects

    :return: graph
    :rtype: pygraph.classes.graph
    """
    return_graph = graph()
    with open(file_path, "rb") as f:
        file_graph = f.read()
    nodes = eval((file_graph.split(']')[0] + ']').strip)
    edges = eval((file_graph.split(']')[1] + ']').strip)
    for node in nodes:
        return_graph.add_node(node, attrs=[precinct_list[node]])
    for edge in edges:
        return_graph.add_edge(edge)
    return return_graph

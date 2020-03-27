import pickle

from pygraph.classes.graph import graph
from hacking_the_election.utils.precinct import Precinct

def visualize_graph(graph, colors=False, graph_from_file=None, save=None):
    """
    Uses matplotlib to visualize graph with centroids saved in precincts saved in nodes.
    If colors is True, also uses community colors saved in precincts, otherwise doesn't
    If graph_from_file isn't None, reads graph from xml file inputted in the graph argument
    If save is a file path, saves matplotlib graph to file.
    """
    if graph_from_file != None:
        with open(graph, 'rb') as f:
            graph_to_visualize = pickle.load(f)
    else:
        graph_to_visualize = graph
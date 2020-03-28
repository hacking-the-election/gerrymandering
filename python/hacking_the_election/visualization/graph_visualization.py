"""Draws graphs (data structure)
"""

import pickle
import sys

from PIL import Image, ImageDraw
from hacking_the_election.utils.precinct import Precinct
from pygraph.classes.graph import graph


def visualize_graph(graph, output_path, colors=None):
    """Creates an image of a graph and saves it to a file.

    :param graph: The graph you want to visualize.
    :type graph: `pygraph.classes.graph.graph`

    :param output_path: Path to where image should be saved.
    :type output_path: str

    :param colors: A function that outputs a number for each of the node attributes in the graph, defaults to None
    :type colors: function or NoneType
    """

    graph_image = Image.new("RGB", (1000, 1000), "white")

    graph_image.save(output_path)


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)

    visualize_graph(graph, sys.argv[2])
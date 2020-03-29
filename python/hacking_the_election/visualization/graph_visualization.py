"""Draws graphs (data structure)
"""

import pickle
import sys

from PIL import Image, ImageDraw
from pygraph.classes.graph import graph

from hacking_the_election.utils.precinct import Precinct
from hacking_the_election.utils.visualization import modify_coords


def visualize_graph(graph, output_path, coords, colors=None):
    """Creates an image of a graph and saves it to a file.

    :param graph: The graph you want to visualize.
    :type graph: `pygraph.classes.graph.graph`

    :param output_path: Path to where image should be saved.
    :type output_path: str

    :param coords: A function that outputs coordinates of a node.
    :type coords: function that outputs list of 2 float

    :param colors: A function that outputs an rgb code for each node, defaults to None
    :type colors: (function that outputs list of 2 float) or NoneType
    """

    graph_image = Image.new("RGB", (1000, 1000), "white")
    draw = ImageDraw.Draw(graph_image, "RGB")

    graph_nodes = graph.nodes()

    modified_coords = modify_coords(
        [coords(node) for node in graph_nodes], [0, 0, 1000, 1000]
    )
    if colors:
        node_colors = [colors(node) for node in graph_nodes]

    for center, node in zip(modified_coords, graph_nodes):
        pass

    graph_image.save(output_path)


if __name__ == "__main__":

    # If called from the command line, it is
    # assumed that the node attributes are precincts.

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)

    visualize_graph(graph, sys.argv[2], lambda node: graph.node_attributes(node).centroid)
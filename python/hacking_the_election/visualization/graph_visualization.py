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
        [coords(node) for node in graph_nodes], [1000, 1000]
    )
    node_coords = {node: point for node, point
                   in zip(graph_nodes, modified_coords)}
    if colors is not None:
        node_colors = [colors(node) for node in graph_nodes]
    else:
        node_colors = [(0, 0, 0) for _ in graph_nodes]

    for center, node, color in zip(modified_coords, graph_nodes, node_colors):
        draw.ellipse(
            [(center[0] - 1, center[1] - 1),
             (center[0] + 1, center[1] + 1)],
            fill=color
        )

        for neighbor in graph.neighbors(node):
            draw.line(
                [tuple(center),
                 tuple(node_coords[neighbor])],
                fill=(0, 0, 0),
                width=1
        )

    graph_image.save(output_path)


if __name__ == "__main__":

    # If called from the command line, it is
    # assumed that the node attributes are precincts.

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)

    visualize_graph(graph, sys.argv[2], lambda node: graph.node_attributes(node)[0].centroid)
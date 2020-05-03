"""Miscellaneous high-level visualization functions.
"""

import os

from hacking_the_election.utils.visualization import add_leading_zeroes, COLORS
from hacking_the_election.visualization.map_visualization import visualize_map


def draw_state(graph, animation_dir, shapes=[]):
    """Draws a precinct map of a state with colors associated to each community.

    :param graph: A graph with precincts as node attributes.
    :type graph: `pygraph.classes.graph.graph`

    :param animation_dir: The directory where the file should be saved.
    :type animation_dir: str

    :param shapes: A list of extra shapes to be drawn.
    :type shapes: list of (`shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`)
    """

    # Draw districts to image file.
    precincts = [graph.node_attributes(node)[0] for node in graph.nodes()]

    # Get next file number in `animation_dir`
    file_numbers = []
    for f in os.listdir(animation_dir):
        try:
            file_numbers.append(int(f[:3]))
        except ValueError:
            pass
    try:
        new_file_number = max(file_numbers) + 1
    except ValueError:
        # No numbered files in dir.
        new_file_number = 0
    file_name = f"{add_leading_zeroes(new_file_number)}.png"

    visualize_map(precincts, os.path.join(animation_dir, file_name),
        coords=lambda x: x.coords, color=lambda x: COLORS[x.community],
        extras=shapes)
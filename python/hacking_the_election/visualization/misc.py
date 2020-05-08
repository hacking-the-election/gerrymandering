"""Miscellaneous high-level visualization functions.
"""

import os

from PIL import Image, ImageDraw

from hacking_the_election.utils.visualization import (
    add_leading_zeroes,
    COLORS,
    get_partisanship_colors
)
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


def draw_gradient():
    """Draws a rectangle containing the gradient that is used for partisanship coloring.
    """

    image = Image.new("RGB", (1000, 100), "white")
    draw = ImageDraw.Draw(image, "RGB")

    color_dict = get_partisanship_colors(
        [(i * 2) / 1000 for i in range(-500, 501)], lambda x: x)
    for i in range(-500, 501):
        draw.line([(i + 500, 0), (i + 500, 100)],
            fill=color_dict[(i * 2) / 1000], width=1)
    
    image.show()
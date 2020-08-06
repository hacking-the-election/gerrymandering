"""Miscellaneous high-level visualization functions.
"""

import os

from PIL import Image, ImageDraw

from hacking_the_election.utils.visualization import (
    get_community_colors,
    get_next_file_path,
    get_partisanship_colors
)
from hacking_the_election.visualization.map_visualization import visualize_map


def draw_state(graph, animation_dir, shapes=[], fpath=None):
    """Draws a precinct map of a state with colors associated to each community.

    :param graph: A graph with precincts as node attributes.
    :type graph: `networkx.Graph`

    :param animation_dir: The directory where the file should be saved.
    :type animation_dir: str

    :param shapes: A list of extra shapes to be drawn.
    :type shapes: list of (`shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`)

    :param fpath: A custom path for this image to be saved to.
    :type fpath: str
    """

    precincts = [graph.nodes[node]['precinct'] for node in graph.nodes]
    colors = get_community_colors(len(set([p.community for p in precincts])))
    
    if fpath is not None:
        output_path = fpath
    else:
        output_path = get_next_file_path(animation_dir)

    # Draw districts to image file.
    visualize_map(precincts, output_path,
        coords=lambda x: x.coords, color=lambda x: colors[x.community],
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


def draw_communities(communities, output_path=None, show=True):
    """Creates a state map of a list of communities. Colored by partisanship.

    :param communities: A list of communities.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param output_path: Path to the file the communities should be saved to, defaults to None
    :type output_path: str or NoneType

    :param show: Whether or not the drawing should be showed, defaults to True
    :type show: bool
    """

    for community in communities:
        community.update_coords()

    color_dict = get_partisanship_colors(communities, lambda c: c.dem_rep_partisanship)

    visualize_map(communities, output_path, coords=lambda c: c.coords,
        color=lambda c: color_dict[c], show=show)
"""Visualizes map shapes.
"""


import pickle
import sys
import types

from PIL import Image, ImageDraw

from hacking_the_election.utils.visualization import (
    get_partisanship_colors,
    modify_coords
)
from hacking_the_election.utils.geometry import shapely_to_geojson


def _draw_polygon(draw, polygon, color):
    """Draws a polygon in color and leaves holes white.

    :param draw: The "draw" object from Pillow.
    :type draw: `PIL.ImageDraw`

    :param polygon: A list of (x, y) tuples for the exterior (first) and each of the interiors of the polygon.
    :type polygon: list of list of 2-tuple of (float or int).

    :param color: The rgb code for the color of the polygon.
    :type color: 3-tuple of int.
    """

    # Draw exterior of image.
    try:
        draw.polygon([tuple(point) for point in polygon[0]], fill=color, outline=(0, 0, 0))
    except TypeError as e:
        # print(polygon)
        raise e
    # Draw blank interiors as holes.
    for interior in polygon[1:]:
        draw.polygon([tuple(point) for point in interior],
            fill=None, outline=(0, 0, 0))


def _get_coords(shapes, coords, n_extras):
    """Gets the list coords of a list of shapes.

    :param shapes: A list of shapes.
    :type shapes: list

    :param coords: A function that takes an item in `shapes` and returns a shapely object.
    :type coords: function

    :param n_extras: The number of shapes that are already in shapely format.
    :type n_extras: int

    :return: A list of lists, each containing coordinate data for a shape in geojson format.
    :rtype: list of lists in geojson format.
    """

    if n_extras > 0:
        shape_coords = [shapely_to_geojson(coords(shape)) for shape in shapes[:-n_extras]]
        shape_coords += [shapely_to_geojson(shape) for shape in shapes[-n_extras:]]
    else:
        shape_coords = [shapely_to_geojson(coords(shape)) for shape in shapes]

    flattened_coords = []  # List of all points in state.
    # Flatten coords.
    for shape in shape_coords:
        if isinstance(shape[0][0][0], float):
            # Shape is a Polygon.
            for ring in shape:
                flattened_coords += ring
        elif isinstance(shape[0][0][0], list):
            # Shape is MultiPolygon.
            for polygon in shape:
                for ring in polygon:
                    flattened_coords += ring
    
    # Must be modified together because bounding box is that of all of the state.
    flat_modified_coords = modify_coords(flattened_coords, [1000, 1000])
    modified_coords = []
    # Unflatten coords.
    coord_index = 0
    for shape in shape_coords:
        if isinstance(shape[0][0][0], float):
            # Shape is a Polygon.
            rings = []
            for ring in shape:
                new_ring_coords = []
                for i, point in enumerate(ring):
                    new_ring_coords.append(flat_modified_coords[coord_index])
                    coord_index += 1
                rings.append(new_ring_coords)
            modified_coords.append(rings)
        elif isinstance(shape[0][0][0], list):
            # Shape is MultiPolygon.
            polygons = []
            for polygon in shape:
                rings = []
                for ring in polygon:
                    new_ring_coords = []
                    for i, point in enumerate(ring):
                        new_ring_coords.append(flat_modified_coords[coord_index])
                        coord_index += 1
                    rings.append(new_ring_coords)
                polygons.append(rings)
            modified_coords.append(polygons)
    
    return modified_coords


def visualize_map(shapes, output_path, coords=lambda x: x, color=None, extras=[], show=False):
    """Draws a set of geograph units onto a canvas.

    :param shapes: A list of shapes to visualize.
    :type shapes: list

    :param output_path: Path to where image should be saved. None if you don't want it to be saved.
    :type output_path: str or NoneType

    :param coords: A function that returns a shapely object when passed an element in `shapes`, defaults to `lambda x: x`
    :type coords: function

    :param color: A function that takes a shape in `shapes` and returns an rgb 3-tuple or a list of rbg tuples that correspond to the index of shapes in `shapes`, defaults to None
    :type color: function or NoneType

    :param extras: A list of extra shapes that are to be drawn with default settings.
    :type extras: list of (`shapely.geometry.Polygon` or `shapely.geometry.MultiPolygon`)

    :param show: whether or not to show the image once generated.
    :type show: bool
    """

    map_image = Image.new("RGB", (1000, 1000), "white")
    draw = ImageDraw.Draw(map_image, "RGB")

    modified_coords = _get_coords(shapes + extras, coords, len(extras))

    if color is not None:
        if isinstance(color, types.FunctionType):
            colors = [color(shape) for shape in shapes]
        elif isinstance(color, list):
            colors = color
        else:
            raise TypeError(f"`color` argument should be NoneType, function, or list, not {type(color)}")
    else:
        colors = [(255, 255, 255) for _ in shapes]
    
    for shape, color in zip(modified_coords[:len(shapes)], colors):
        if isinstance(shape[0][0][0], float):
            # Shape is polygon.
            _draw_polygon(draw, shape, color)
        elif isinstance(shape[0][0][0], list):
            # Shape is multipolygon.
            for polygon in shape:
                _draw_polygon(draw, polygon, color)
    
    # Draw extras.
    for shape in modified_coords[len(shapes):]:
        if isinstance(shape[0][0][0], float):
            # Shape is polygon.
            _draw_polygon(draw, shape, None)
        elif isinstance(shape[0][0][0], list):
            # Shape is multipolygon.
            for polygon in shape:
                _draw_polygon(draw, polygon, None)

    if output_path is not None:
        map_image.save(output_path)
    if show:
        map_image.show()


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)

    precincts = [precinct_graph.node_attributes(node)[0] for node in precinct_graph.nodes()]
    color_dict = get_partisanship_colors(precincts, lambda p: p.dem_rep_partisanship)

    visualize_map(precincts, None if sys.argv[2] == "None" else sys.argv[2],
        coords=lambda x: x.coords, color=lambda x: color_dict[x], show=True)
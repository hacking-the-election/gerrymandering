"""Visualizes map shapes.
"""


import pickle
import sys
import types

from PIL import Image, ImageDraw

from hacking_the_election.utils.visualization import modify_coords
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
    draw.polygon([tuple(point) for point in polygon[0]], fill=color, outline=(0, 0, 0))
    # Draw white interiors as holes.
    for interior in polygon[1:]:
        draw.polygon([tuple(point) for point in interior],
            fill=(255, 255, 255), outline=(0, 0, 0))


def visualize_map(shapes, output_path, coords=lambda x: x, color=None, show=False):
    """Draws a set of geograph units onto a canvas.

    :param shapes: A list of shapes to visualize.
    :type shapes: object

    :param output_path: Path to where image should be saved. None if you don't want it to be saved.
    :type output_path: str or NoneType

    :param coords: A function that returns a shapely object when passed an element in `shapes`, defaults to `lambda x: x`
    :type coords: function

    :param color: A function that takes a shape in `shapes` and returns an rgb 3-tuple or a list of rbg tuples that correspond to the index of shapes in `shapes`, defaults to None
    :type color: function or NoneType

    :param show: whether or not to show the image once generated.
    :type show: bool
    """

    map_image = Image.new("RGB", (1000, 1000), "white")
    draw = ImageDraw.Draw(map_image, "RGB")

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
                for ring in shape:
                    new_ring_coords = []
                    for i, point in enumerate(ring):
                        new_ring_coords.append(flat_modified_coords[coord_index])
                        coord_index += 1
                    rings.append(new_ring_coords)
                polygons.append(rings)
            modified_coords.append(polygons)

    if color is not None:
        if isinstance(color, types.FunctionType):
            colors = [color(shape) for shape in shapes]
        elif isinstance(color, list):
            colors = color
        else:
            raise TypeError(f"`color` argument should be NoneType, function, or list, not {type(color)}")
    else:
        colors = [(255, 255, 255) for _ in shapes]
    
    for shape, color in zip(modified_coords, colors):
        if isinstance(shape[0][0][0], float):
            # Shape is polygon.
            tuple_coords = [[tuple(point) for point in ring] for ring in shape]
            _draw_polygon(draw, tuple_coords, color)
        elif isinstance(shape[0][0][0], list):
            # Shape is multipolygon.
            tuple_coords = [[[tuple(point) for point in ring]
                            for ring in polygon]
                           for polygon in shape]
            for polygon in tuple_coords:
                _draw_polygon(draw, polygon, color)

    if output_path is not None:
        map_image.save(output_path)
    if show:
        map_image.show()


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)

    precincts = [precinct_graph.node_attributes(node)[0] for node in precinct_graph.nodes()]

    # TODO: Add color function based on partisanship.
    visualize_map(precincts, None if sys.argv[2] == "None" else sys.argv[2], coords=lambda x: x.coords, show=True)
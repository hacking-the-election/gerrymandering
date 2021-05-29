from PIL import Image, ImageDraw
import pickle
import sys
import numpy as np
from random import randint

from shapely.geometry import Point, MultiPolygon, Polygon

from hacking_the_election.utils.geometry import shapely_to_geojson
from hacking_the_election.utils.visualization import get_community_colors, modify_coords
from hacking_the_election.visualization.map_visualization import _get_coords, _draw_polygon

def visualize_map(communities, output_path, quality=8192):
    """
    Visualizes map of communities.
    Takes in a list of communities, a path to output the picture to,
    and a quality which should be the resolution of the image. 
    """
    image = Image.new("RGB", (quality,quality), "white")
    draw = ImageDraw.Draw(image, "RGB")

    flattened_x = []
    flattened_y = []
    x_values = []
    y_values = []
    blocks = []
    flatten_to_block_indexes = {}
    block_list = []
    for community in communities:
        for block in community.blocks:
            block_list.append(block)
    counter = 0
    for i, block in enumerate(block_list):
        # print(shapely_to_geojson(block.coords)[0])
        test = np.array(shapely_to_geojson(block.coords)[0])
        x_coords = []
        y_coords = []
        for coord in test:
            x_coords.append(coord[0])
            y_coords.append(coord[1])
            flattened_x.append(coord[0])
            flattened_y.append(coord[1])
            flatten_to_block_indexes[counter] = i
            counter += 1
        x_values.append(x_coords)
        y_values.append(y_coords)
        # blocks.append(test)
        blocks.append(shapely_to_geojson(block.coords)[0])

    flattened_x = np.array(flattened_x)
    flattened_y = np.array(flattened_y)

    min_x = flattened_x.min()
    max_x = flattened_x.max()
    min_y = flattened_y.min()
    max_y = flattened_y.max()
    dilation = max(max_x-min_x, max_y-min_y)

    flattened_x = np.subtract(flattened_x, np.array(min_x))
    flattened_y = np.subtract(flattened_y, np.array(min_y))

    flattened_x *= 0.95*quality/dilation
    flattened_y *= 0.95*quality/dilation

    flattened_y = np.negative(flattened_y) + quality

    flattened_x = np.add(flattened_x, (quality - flattened_x.max()) / 2)
    flattened_y = np.subtract(flattened_y, flattened_y.min()/2)

    flattened_x, flattened_y = flattened_x.tolist(), flattened_y.tolist()

    with open("./modified_coords.pickle", "wb") as f:
        pickle.dump(blocks, f)

    colors = []
    for i in range(len(communities)):
        # Ensure colors don't come too close to black or white
        colors.append((randint(5,250),randint(5,250),randint(5,250)))

    block_tracker = 0
    blocks = []
    block_holder = []
    coord_num = len(flattened_x)
    for i in range(len(flattened_x)):
        if flatten_to_block_indexes[i] == block_tracker:
            block_holder.append([flattened_x[i], flattened_y[i]])
        else:
            blocks.append(block_holder)
            block_holder = [[flattened_x[i], flattened_y[i]]]
            block_tracker += 1
        print(f"\rCoords Unflattened: {i}/{coord_num}, {round(100*i/coord_num, 1)}%", end="")
        sys.stdout.flush()
    print("\n", end="")

    for i, block in enumerate(blocks):
        # for polygon in modified_coords[i]:
        #     print(polygon)
        draw.polygon([tuple(point) for point in block], fill=colors[block_list[i].community-1], outline=(0, 0, 0))
            # _draw_polygon(draw, polygon, colors[i])

    image.save(output_path)
    image.show()

if __name__ == '__main__':
    with open("../community_list.pickle", "rb") as f:
        community_list = pickle.load(f)

    visualize_map(community_list, "../community_visualization.jpg")

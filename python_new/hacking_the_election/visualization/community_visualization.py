"""
Contains functions for visualizing communities. 

Usage (in python directory):

python3 -m hacking_the_election.visualization.community_visualization [path_to_community_list.pickle] [mode]
"""

from PIL import Image, ImageDraw
import pickle
import sys
import numpy as np
from random import randint

from shapely.geometry import Point, MultiPolygon, Polygon

from hacking_the_election.utils.geometry import shapely_to_geojson
from hacking_the_election.utils.visualization import get_community_colors, modify_coords

def visualize_map(communities, output_path, quality=8192, mode="random", outline=True):
    """
    Visualizes map of communities.
    Takes in a list of communities, a path to output the picture to,
    and a quality which should be the resolution of the image. 
    """
    quality = int(quality)
    image = Image.new("RGB", (quality,quality), "white")
    draw = ImageDraw.Draw(image, "RGB")

    flattened_x = []
    flattened_y = []
    x_values = []
    y_values = []
    blocks = []
    block_list = []
    community_id_to_index = {communities[i].id : i for i in range(len(communities))}
    for community in communities:
        state_name = community.state
        for block in community.blocks:
            block_list.append(block)

    block_num = len(block_list)
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

        # x_values.append(x_coords)
        x_values.append(np.array(x_coords))
        # y_values.append(y_coords)
        y_values.append(np.array(y_coords))
        # blocks.append(test)
        blocks.append(shapely_to_geojson(block.coords)[0])
        print(f"\rRetrieve Coordinates: {i}/{block_num}, {round(100*i/block_num, 1)}%", end="")
        sys.stdout.flush()
    print("\n", end="")


    flattened_x = np.array(flattened_x)
    flattened_y = np.array(flattened_y)

    min_x = flattened_x.min()
    max_x = flattened_x.max()
    min_y = flattened_y.min()
    max_y = flattened_y.max()
    dilation = max(max_x-min_x, max_y-min_y)

    final_max_x = (max_x-min_x)*0.95*quality/dilation
    final_max_y = quality - ((max_y-min_y)*0.95*quality/dilation)

    modified_coords = []
    for i in range(block_num):
        block_x_values = x_values[i]
        block_y_values = y_values[i]
        block_x_values = np.subtract(block_x_values, min_x)
        block_y_values = np.subtract(block_y_values, min_y)
        block_x_values *= 0.95*quality/dilation
        block_y_values *= 0.95*quality/dilation
        block_y_values = np.negative(block_y_values) + quality

        block_x_values = np.add(block_x_values, (quality - final_max_x) / 2)
        block_y_values = np.subtract(block_y_values, final_max_y/2)
        modified_coords.append(zip(block_x_values.tolist(), block_y_values.tolist()))
        print(f"\rCoords Modified: {i}/{block_num}, {round(100*i/block_num, 1)}%", end="")
        sys.stdout.flush()
    print("\n", end="")        

    colors = []
    if mode == "block_partisanship":
        for block in block_list:
            percent_dem = block.percent_dem
            percent_rep = block.percent_rep
            if percent_dem:
                # print(block.id)
                # try:
                colors.append((round(percent_rep*255),0,round(percent_dem*255)))
                # except:
                    # print(block.id, block.pop, block.racial_data, block.total_votes, block.dem_votes, block.percent_dem)
            else:
                colors.append((128,128,128))
    elif mode in ["community_partisanship", "random"]:
        for i in range(len(communities)):
            # Ensure colors don't come too close to black or white
            if mode == "community_partisanship":
                percent_dem = communities[i].percent_dem
                percent_rep = communities[i].percent_rep
                if percent_dem:
                    colors.append((round(percent_rep*255),0,round(percent_dem*255)))
                else:
                    colors.append((128,128,128))
            elif mode  == "random":
                colors.append((randint(5,250),randint(5,250),randint(5,250)))

    for i, block in enumerate(modified_coords):
        if mode == "block_partisanship":
            if outline == "true":
                draw.polygon([tuple(point) for point in block], fill=colors[i], outline=(0, 0, 0))
            else:
                draw.polygon([tuple(point) for point in block], fill=colors[i])
        else:
            if outline == "true":
                draw.polygon([tuple(point) for point in block], fill=colors[community_id_to_index[block_list[i].community]], outline=(0, 0, 0))
            else:
                draw.polygon([tuple(point) for point in block], fill=colors[community_id_to_index[block_list[i].community]])

    last_slash =output_path.rfind("/")
    modified_output_path = output_path[:last_slash+1] + state_name + "_" + output_path[last_slash+1:]
    image.save(modified_output_path)
    image.show()

if __name__ == '__main__':
    with open(sys.argv[1], "rb") as f:
        community_list = pickle.load(f)

    mode = sys.argv[2]
    if mode not in ["random", "community_partisanship", "block_partisanship"]:
        raise Exception("Mode argument needs to be 'random', 'block_partisanship', or 'community_partisanship'!")
    try:
        quality = sys.argv[3]
    except:
        visualize_map(community_list, "docs/images/" + mode + "_community_visualization.jpg", mode=mode)
    else:
        try:
            _ = int(quality)
        except:
            outline = quality
            if outline.lower() in ["true", "false"]:
                visualize_map(community_list, "docs/images/" + mode + "_community_visualization.jpg", mode=mode, outline=outline.lower())
            else:
                raise Exception("Outline argument must be 'true' or 'false'!")
        else:
            try:
                outline = sys.argv[4]
            except:
                visualize_map(community_list, "docs/images/" + mode + "_community_visualization.jpg", mode=mode, quality=quality)
            else:
                if outline.lower() in ["true", "false"]:
                    visualize_map(community_list, "docs/images/" + mode + "_community_visualization.jpg", mode=mode, quality=quality, outline=outline.lower())
                else:
                    raise Exception("Outline argument must be 'true' or 'false'!")

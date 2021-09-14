"""
Contains functions for visualizing communities. 

Usage (in python directory):

python3 -m hacking_the_election.visualization.community_visualization [path_to_community_list.pickle] [mode]
"""

from PIL import Image, ImageDraw
import pickle
import sys
import numpy as np
import math
from random import randint

from shapely.geometry import Point, MultiPolygon, Polygon

from hacking_the_election.utils.geometry import shapely_to_geojson
from hacking_the_election.utils.visualization import get_community_colors, modify_coords

def visualize_map(communities, output_path, quality=8192, mode="community_random", outline=True):
    """
    Visualizes map of communities.
    Takes in a list of communities, a path to output the picture to,
    and a quality which should be the resolution of the image. 
    """
    block_list = []
    for community in communities:
        state_name = community.state
        for block in community.blocks:
            block_list.append(block)
    id_to_block = {block.id : block for block in block_list}
    for community in communities:
        community.initialize_graph(id_to_block)
        community.find_neighbors_and_border(id_to_block)
    community_coords = []
    for i, community in enumerate(communities):
        # print(np.array([[id_to_block[edge[0]].centroid, id_to_block[edge[1]].centroid] for edge in community.border_edges]))
        
        # edge_array = [id_to_block[community.border_edges[0][0]].centroid, id_to_block[community.border_edges[0][1]].centroid]
        # for edge in community.border_edges[1:]:
        #     edge_array.append(id_to_block[edge[0]].centroid)
        #     edge_array.append(id_to_block[edge[1]].centroid)

        # edge_array = []
        # for edge in community.border_edges:
        #     coords_0 = id_to_block[edge[0]].centroid
        #     coords_1 = id_to_block[edge[1]].centroid
        #     edge_array.append(((coords_0[0]+coords_1[0])/2, (coords_0[1]+coords_1[1])/2))
        # community_coords.append(np.array(edge_array))
        # print(edge_array)
        # print(community.border_edges)
        if not community.border_edges:
            continue    
        for edge in community.border_edges:
            coords_0 = id_to_block[edge[0]].centroid
            coords_1 = id_to_block[edge[1]].centroid
            community_coords.append(np.array([coords_0,coords_1]))
        # community_coords.append(np.array([[id_to_block[edge[0]].centroid, id_to_block[edge[1]].centroid] for edge in community.border_edges]))
        # edge_coord = id_to_block[community.border_edges[0][0]].coords.intersection(id_to_block[community.border_edges[0][1]].coords)
        # print(edge_coord) 
        #    edge_coord = edge_coord.union(id_to_block[edge[0]].coords.intersection(id_to_block[edge[1]].coords))
        # community_coords.append(np.array(shapely_to_geojson(edge_coord))[0])
        print(f"\rCommunity Boundaries Found: {i}/{len(communities)}, {round(100*i/len(communities), 1)}%", end="")

        
    quality = int(quality)
    image = Image.new("RGB", (quality,quality), "white")
    draw = ImageDraw.Draw(image, "RGB")


    flattened_x = []
    flattened_y = []
    x_values = []
    y_values = []
    blocks = []
    community_id_to_index = {communities[i].id : i for i in range(len(communities))}

    block_num = len(block_list)
    community_x_values = []
    community_y_values = []
    for coords in community_coords:
        community_x_coords = []
        community_y_coords = []
        for coord in coords:
            community_x_coords.append(coord[0])
            community_y_coords.append(coord[1])
        community_x_values.append(np.array(community_x_coords))
        community_y_values.append(np.array(community_y_coords))
    # print(community_x_values)

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

    modified_community_coords = []
    for i in range(len(community_x_values)):
        community_x_value = community_x_values[i]        
        community_y_value = community_y_values[i]      
        community_x_value = np.subtract(community_x_value, min_x)
        community_y_value = np.subtract(community_y_value, min_y)
        community_x_value *= 0.95*quality/dilation
        community_y_value *= 0.95*quality/dilation
        community_y_value = np.negative(community_y_value) + quality

        community_x_value = np.add(community_x_value, (quality - final_max_x) / 2)
        community_y_value = np.subtract(community_y_value, final_max_y/2)
        # print(list(zip(community_x_values.tolist(), community_y_values.tolist())))
        modified_community_coords.append(zip(community_x_value.tolist(), community_y_value.tolist()))

    colors = []
    if mode in ["block_partisanship", "block_race", "block_density", "block_random"]:
        if mode == "block_partisanship":
            for block in block_list:
                percent_dem = block.percent_dem_votes
                percent_rep = block.percent_rep_votes
                if percent_dem:
                    # print(block.id)
                    # try:
                    colors.append((round(percent_rep*255),0,round(percent_dem*255)))
                    # except:
                        # print(block.id, block.pop, block.racial_data, block.total_votes, block.dem_votes, block.percent_dem)
                else:
                    colors.append((128,128,128))
        elif mode == "block_density":
            max_density = max([block.density for block in block_list])
            for block in block_list:
                density = block.density
                colors.append((0,round(255 * math.log(1023 * density/max_density+1,1024)), 0))
                # print((0,round(255 * (math.log((255 * density/max_density)+1,256))), 0))
                # print((255 * density/max_density)+1)
        elif mode == "block_race":
            for block in block_list:
                percent_white = block.percent_white
                percent_black = block.percent_black
                percent_hispanic = block.percent_hispanic
                percent_aapi = block.percent_aapi
                percent_aian = block.percent_aian
                percent_other = block.percent_other
                if percent_white:
                    color = np.zeros(3)
                    # White: Red
                    # Black: Green
                    # Hispanic: Blue
                    # Asian: Yellow
                    # American Indian: Purple
                    # Other: Turquoise
                    color += percent_white * np.array([255,0,0])
                    color += percent_black * np.array([0,255,0])
                    color += percent_hispanic * np.array([0,0,255])
                    color += percent_aapi * np.array([255,255,0])
                    color += percent_aian * np.array([255,0,255])
                    color += percent_other * np.array([0,255,255])
                    # print(tuple(np.round(color)))
                    colors.append(tuple([round(val) for val in color]))
                else:
                    colors.append((128,128,128))
        elif mode == "block_random":
            for block in block_list:
                colors.append((randint(5,250),randint(5,250),randint(5,250)))
    elif mode in ["community_partisanship", "community_random"]:
        for i in range(len(communities)):
            # Ensure colors don't come too close to black or white
            if mode == "community_partisanship":
                percent_dem = communities[i].percent_dem
                percent_rep = communities[i].percent_rep
                if percent_dem:
                    colors.append((round(percent_rep*255),0,round(percent_dem*255)))
                else:
                    colors.append((128,128,128))
            elif mode  == "community_random":
                colors.append((randint(5,250),randint(5,250),randint(5,250)))
    # print(len(modified_coords), len(colors))
    for i, block in enumerate(modified_coords):
        if mode in ["block_random", "block_partisanship", "block_race", "block_density"]:
            if outline == "true":
                draw.polygon([tuple(point) for point in block], fill=colors[i], outline=(0, 0, 0))
            else:
                draw.polygon([tuple(point) for point in block], fill=colors[i])
        else:
            if outline == "true":
                draw.polygon([tuple(point) for point in block], fill=colors[community_id_to_index[block_list[i].community]], outline=(0, 0, 0))
            else:
                draw.polygon([tuple(point) for point in block], fill=colors[community_id_to_index[block_list[i].community]])
    for i, community in enumerate(modified_community_coords):
        # print(community, [tuple(point) for point in community], [tuple(point) for point in block])
        # for edge in [tuple(point) for point in community]:
            # draw.line([tuple(point) for point in edge], fill=(0,0,0), width=10, joint="curve")
        draw.line([tuple(point) for point in community], fill=(0,0,0), width=2, joint="curve")

    last_slash =output_path.rfind("/")
    modified_output_path = output_path[:last_slash+1] + state_name + "_" + output_path[last_slash+1:]
    image.save(modified_output_path)
    image.show()

if __name__ == '__main__':
    with open(sys.argv[1], "rb") as f:
        community_list = pickle.load(f)

    mode = sys.argv[2]
    if mode not in ["community_random", "community_partisanship", "block_random", "block_partisanship", "block_race", "block_density"]:
        raise Exception("Mode argument needs to be 'community_random', 'block_partisanship', 'block_race', 'block_density', or 'community_partisanship'!")
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

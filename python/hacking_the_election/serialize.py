"""
Script for serializing precinct-level election, geo, and population
data.

Data is serialized into:
 - .pickle file containing graph with nodes containing Precinct objects

Usage:
python3 serialize.py [election_file] [geo_file] [pop_file] [state] [output.pickle]
"""


import json
from os.path import dirname
import pickle
import sys
from time import time
from itertools import combinations

from pygraph.classes.graph import graph
from pygraph.readwrite.markup import write
from shapely.geometry import Polygon, MultiPolygon

from hacking_the_election.utils.precinct import Precinct
from hacking_the_election.utils.serialization import compare_ids, split_multipolygons, combine_holypolygons
from hacking_the_election.utils.geometry import geojson_to_shapely, get_if_bordering
from hacking_the_election.utils.graph import get_components
from hacking_the_election.visualization.graph_visualization import visualize_graph

def convert_to_int(string):
    """
    Wrapped error handling for int().
    """
    try:
        return int(string)
    except ValueError:
        if "." in string:
            try:
                return int(string[:string.index(".")])
            except ValueError:
                return 0
        else:
            return 0


def tostring(string):
    """
    Removes redundant quotes or converts int to string.
    """
    try:
        if string[0] in ["\"", "'"] and string[-1] in ["\"", "'"]:
            return string[1:-1]
        else:
            return string
    except TypeError:  # most likely that `string` was of type `int`
        return str(string)


def create_graph(election_file, geo_file, pop_file, state):
    """
    Returns graph with precinct ids stored in nodes (string of XML)
    and list of Precinct objects.
    """
    start_time = time()
    with open("./state_metadata.json", "r") as f:
        state_metadata = json.load(f)

    # This should be a race for president
    dem_key = state_metadata[state]["party_data"]["dem_keys"][0]
    rep_key = state_metadata[state]["party_data"]["rep_keys"][0]
    try:
        total_key = state_metadata[state]["party_data"]["total_keys"][0]
    except:
        total_key = None
    try:
        other_key = state_metadata[state]["party_data"]["other_keys"][0]
    except:
        other_key = None

    try:
        green_key = state_metadata[state]["party_data"]["green_keys"][0]
    except:
        green_key = None
    try:
        lib_key = state_metadata[state]["party_data"]["libertarian_keys"][0]
    except:
        lib_key = None
    try:
        reform_key = state_metadata[state]["party_data"]["reform_keys"][0]
    except:
        reform_key = None
    try:
        ind_key = state_metadata[state]["party_data"]["independent_keys"][0]
    except:
        ind_key = None
    try:
        const_key = state_metadata[state]["party_data"]["constitution_keys"][0]
    except:
        const_key = None 

    json_ids  = state_metadata[state]["geo_id"]
    json_pops = state_metadata[state]["pop_key"]
    ele_ids   = state_metadata[state]["ele_id"]

    # Read election and geo data files.
    with open(geo_file, "r") as f:
        geodata = json.load(f)

    if election_file != "none":
        # Find election precincts not in geodata and vice versa

        if election_file[-4:] == ".json":
            election_data_type = "json"
            with open(election_file, "r") as f:
                election_data = json.load(f)

            election_data_ids = [tostring("".join([precinct["properties"][ele_id] for ele_id in ele_ids]))
                                 for precinct in election_data["features"]]
            geodata_ids = [tostring("".join([precinct["properties"][json_id] for json_id in json_ids]))
                           for precinct in geodata["features"]]
            
            election_data_to_geodata = compare_ids(election_data_ids, geodata_ids)

        elif election_file[-4:] == ".tab":
            election_data_type = "tab"
            with open(election_file, "r") as f:
                election_file_contents = f.read().strip()
            # create a list of rows that are a list of values in that row
            election_data = [line.split("\t") for line in
                            election_file_contents.split("\n")]
            ele_id_col_indices = [i for i, header in enumerate(election_data[0]) if header in ele_ids]
            election_data_ids = [tostring("".join([election_data_row[ele_id_index] for ele_id_index in ele_id_col_indices]))
                                 for election_data_row in election_data[1:]]
            geodata_ids = [tostring("".join([precinct["properties"][json_id] for json_id in json_ids])) 
                           for precinct in geodata["features"]]

            election_data_to_geodata = compare_ids(election_data_ids, geodata_ids)
        else:
            # Election Data can only be .tab or .json files
            raise ValueError('.json or .tab file required')

    else:
        election_data_type = False

    if pop_file != "none":
        # Finds population data, any population_ids to geodata_ids will be done when finding pop
        if pop_file[-4:] == ".json":
            pop_data_type = "json"
            with open(pop_file, "r") as f:
                popdata = json.load(f)
        elif pop_file[-4:] == ".tab":
            pop_data_type = "tab"
            with open(pop_file, 'r') as f:
                popdata = f.read().strip()
        else:
            # Population Data can only be .tab or .json files
            raise ValueError('.json or .tab file required')
    else:
        pop_data_type = False

    precincts = []
    unordered_precinct_graph = graph()


    # Get election data. If needed, converts election data ids to geodata ids 
    # using election_data_to_geodata, thus all keys are those of geodata.
    # {precinct_id: [{dem:data}, {rep:data}], [{green:data}, {lib:data}] as neeeded}
    precinct_election_data = {}
    # If there is an election data file...
    if election_data_type:
        # If the election data file is a .json file
        if election_data_type == "json":
             # Fill precinct_election_data
            for precinct1 in election_data["features"]:
                properties1 = precinct1["properties"]
                # Convert election data id to geodata id
                precinct_id1 = election_data_to_geodata[tostring("".join(properties1[ele_id] for ele_id in ele_ids))]
                party_data = [
                    {'dem': convert_to_int(properties1[dem_key])},
                    {'rep': convert_to_int(properties1[rep_key])}
                ]
                if total_key:
                    party_data.append({'other': convert_to_int(properties1[total_key]) - sum(
                        [properties1[key] for key in [green_key, lib_key, reform_key, ind_key, const_key] if key != None]
                    )})
                # If there is already a total_key, the other_key id is not needed.
                elif other_key:
                    party_data.append({'other': convert_to_int(properties1[other_key])})


                if green_key:
                    party_data.append({'green': convert_to_int(properties1[green_key])})
                if lib_key:
                    party_data.append({'lib': convert_to_int(properties1[lib_key])})
                if reform_key:
                    party_data.append({'reform': convert_to_int(properties1[reform_key])})
                if ind_key:
                    party_data.append({'ind': convert_to_int(properties1[ind_key])})
                if const_key:
                    party_data.append({'const': convert_to_int(properties1[const_key])})

                precinct_election_data[precinct_id1] = party_data
        # If the election data file is a .tab file
        elif election_data_type == "tab":

            # headers for different categories
            election_column_names = election_data[0]

            # Get the index of each of the relevant columns.
            dem_key_col_index = \
                [i for i, col in enumerate(election_column_names)
                if col == dem_key][0]

            rep_key_col_index = \
                [i for i, col in enumerate(election_column_names)
                if col == rep_key][0]

            ele_id_col_indices = [i for i, header in enumerate(election_column_names) if header in ele_ids]

            # Fill precinct_election_data
            for precinct in election_data[1:]:
                # convert election data id to geodata id
                election_data_id = election_data_to_geodata[
                    tostring("".join([precinct[ele_id_col_index] for ele_id_col_index in ele_id_col_indices]))
                    ]
                party_data = [
                    {'dem': precinct[dem_key_col_index]
                },
                    {'rep': precinct[rep_key_col_index]
                },
                ]
                if total_key:
                    # I apologize. This basically finds indexes, then subtracts values from total, even
                    # if it doesn't look like it.
                    total_key_col_index = \
                        [i for i, col in enumerate(election_column_names)
                        if col == total_key][0]
                    party_data.append({'other': convert_to_int(election_column_names[total_key_col_index]) 
                        - sum([election_column_names[[i for i, col in enumerate(election_column_names) if col == key][0]] 
                        for key in [green_key, lib_key, reform_key, ind_key, const_key] if key != None]
                    )})
                # If there is already a total_key, the other_key id is not needed.
                elif other_key:
                    other_key_col_index = \
                        [i for i, col in enumerate(election_column_names)
                        if col == other_key][0]
                    party_data.append({'other': convert_to_int(election_column_names[other_key_col_index])})


                if green_key:
                    key_index = [i for i, col in enumerate(election_column_names)
                        if col == green_key][0]
                    party_data.append({'green': convert_to_int(election_column_names[key_index])})
                if lib_key:
                    key_index = [i for i, col in enumerate(election_column_names)
                        if col == lib_key][0]
                    party_data.append({'lib': convert_to_int(election_column_names[key_index])})
                if reform_key:
                    key_index = [i for i, col in enumerate(election_column_names)
                        if col == reform_key][0]
                    party_data.append({'reform': convert_to_int(election_column_names[key_index])})
                if ind_key:
                    key_index = [i for i, col in enumerate(election_column_names)
                        if col == ind_key][0]
                    party_data.append({'ind': convert_to_int(election_column_names[key_index])})
                if const_key:
                    key_index = [i for i, col in enumerate(election_column_names)
                        if col == const_key][0]
                    party_data.append({'const': convert_to_int(election_column_names[key_index])})

                precinct_election_data[election_data_id] = party_data
    else:
        # Fill precinct_election_data
        for precinct in geodata["features"]:
            properties = precinct["properties"]
            precinct_id = tostring("".join(properties[json_id] for json_id in json_ids))
            precinct_election_data[precinct_id] = [
                {'dem': convert_to_int(properties[dem_key])},
                {'rep': convert_to_int(properties[rep_key])}
            ]
            if total_key:
                party_data.append({'other': convert_to_int(properties[total_key]) - sum(
                        [properties[key] for key in [green_key, lib_key, reform_key, ind_key, const_key] if key != None]
                    )})
            # If there is already a total_key, the other_key id is not needed.
            elif other_key:
                party_data.append({'other': convert_to_int(properties[other_key])})


            if green_key:
                party_data.append({'green': convert_to_int(properties[green_key])})
            if lib_key:
                party_data.append({'lib': convert_to_int(properties[lib_key])})
            if reform_key:
                party_data.append({'reform': convert_to_int(properties[reform_key])})
            if ind_key:
                party_data.append({'ind': convert_to_int(properties[ind_key])})
            if const_key:
                party_data.append({'const': convert_to_int(properties[const_key])})


    # Then find population. pop should have keys that use geodata ids
    # {precinct_id : population}
    pop = {}
    if pop_data_type:
        if pop_data_type == "json":
            for pop_precinct in popdata["features"]:
                pop_properties = pop_precinct["properties"]
                # Assumes the same Ids are used in geodata as in population data
                pop_precinct_id = tostring("".join(pop_properties[json_id] for json_id in json_ids))
                pop[pop_precinct_id] = sum([pop_properties[key] for key in json_pops])
        # individual conditionals for states, they should go here
        elif pop_data_type == "tab":
            # In these condtionals, pop should have keys that match geodata. this is done here. 
            # compare_ids() from the serialization file in utils can be used for population
            pass
        else:
            raise AttributeError
    else:
        # find where population is stored, either election or geodata
        try:
            _ = geodata["features"][0]["properties"][json_pops[0]]
        except ValueError:
            pop_data_type = "tab"
            # population is in election data
            pop_col_indices = [i for i, header in enumerate(election_data[0]) if header in json_pops]
            # find which indexes have population data
            for row in election_data[1:]:
                pop_precinct_id = election_data_to_geodata[
                    tostring("".join([row[ele_id_col_index] for ele_id_col_index in ele_id_col_indices]))
                ]
                precinct_populations = [row[i] for i in pop_col_indices]
                pop[pop_precinct_id] = sum(precinct_populations)
        else:
            # population is in geodata
            for geodata_pop_precinct in geodata["features"]:
                geodata_pop_properties = geodata_pop_precinct["properties"]
                geodata_pop_precinct_id = tostring("".join(geodata_pop_properties[json_id] for json_id in json_ids))
                precinct_populations = [geodata_pop_properties[key] for key in json_pops]
                pop[geodata_pop_precinct_id] = sum(precinct_populations)

    # Create a geodata dictionary
    geodata_dict = {
        tostring("".join(precinct["properties"][json_id] for json_id in json_ids)) :
        precinct["geometry"]["coordinates"]
        for precinct in geodata["features"]
    }


    # Remove multipolygons from our dictionaries. (This is so our districts/communities stay contiguous)
    split_multipolygons(geodata_dict, pop, precinct_election_data)

    combine_holypolygons(geodata_dict, pop, precinct_election_data)
    # Modify geodata_dict to use 'shapely.geometry.Polygon's
    geodata_dict = {
        precinct :
        geojson_to_shapely(coords)
        for precinct, coords in geodata_dict.items()
    }
    print(get_if_bordering(geodata_dict['50021VD1731'], geodata_dict['50021VD1721']), 'YO')
    # Create list of Precinct objects
    precinct_list = []
    for precinct_id, coordinate_data in geodata_dict.items():
        precinct_pop = pop[precinct_id]
        precinct_election = {
            (list(party_dict.keys())[0] + '_data') :
            (list(party_dict.values())[0])
            for party_dict in precinct_election_data[precinct_id]
        }
        precinct_list.append(
            Precinct(precinct_pop, coordinate_data, state, precinct_id, **precinct_election)
        )
    
    # Add nodes to our unordered graph
    for i, precinct in enumerate(precinct_list):
        unordered_precinct_graph.add_node(i, attrs=[precinct])
    print('Nodes: ', len(unordered_precinct_graph.nodes()))
    print(time() - start_time)

    # Add edges to our graph
    completed_precincts = []
    for node in unordered_precinct_graph.nodes():
        coordinate_data = unordered_precinct_graph.node_attributes(node)[0].coords
        min_x, min_y, max_x, max_y = coordinate_data.bounds
        x_length = max_x - min_x
        y_length = max_y - min_y
        # Given a bounding box, how much it should be scaled by for precincts to consider
        scale_factor = 0.02

        min_x -= (x_length * scale_factor)
        max_x += (x_length * scale_factor)
        min_y -= (y_length * scale_factor)
        max_y += (y_length * scale_factor)
        precincts_to_check = []
        for check_node in unordered_precinct_graph.nodes():
            # If the node being checked is the node we checking to
            if check_node == node:
                continue
            # If the node being checked already has edges, then no point in checking
            if check_node in completed_precincts:
                continue
            # If there is already an edge between the two
            if unordered_precinct_graph.has_edge((node, check_node)):
                continue
            check_coordinate_data = unordered_precinct_graph.node_attributes(check_node)[0].coords
            for point in check_coordinate_data.exterior.coords:
                if min_x <= point[0] <= max_x:
                    if min_y <= point[1] <= max_y:
                        precincts_to_check.append(check_node)
                        break
        print(precincts_to_check, node)
        for precinct10 in precincts_to_check:            
            if get_if_bordering(coordinate_data, unordered_precinct_graph.node_attributes(precinct10)[0].coords):
                unordered_precinct_graph.add_edge((node, precinct10))
        completed_precincts.append(node)

    print(time() - start_time)
    # The graph will have multiple representations, i.e. an "edge" going both ways, 
    # but it's only considered one edge in has_edge(), add_edge(), delete_edge(), etc.
    print('Edges: ', len(unordered_precinct_graph.edges())/2)
    # Create list of nodes in ascending order by degree
    ordered_nodes = sorted(unordered_precinct_graph.nodes(), key=lambda  n: len(unordered_precinct_graph.neighbors(n))) 


    # Repeat until entire graph is contiguous
    # Get components of graph (islands of precincts)
    graph_components = get_components(unordered_precinct_graph)
    if not len(graph_components) == 1:
        # Keys are minimum distances to and from different islands, 
        # values are tuples of nodes to add edges between
        min_distances = {}
        # For nonrepeating combinations of components, add to list of edges
        for combo in combination([range(0, (len(graph_components) - 1))]):
            # Create list of centroids to compare
            centroids_1 = {unordered_precinct_graph.node_attributes(precinct).centroid :
                precinct
                for precinct in graph_components[int(str(combo)[0])]}
            centroids_2 = {unordered_precinct_graph.node_attributes(precinct).centroid :
                precinct
                for precinct in graph_components[int(str(combo)[1])]}
            min_distance = 0
            for centroid_1 in centroids_1.keys():
                for centroid_2 in centroids_2.keys():
                    # No need to sqrt unnecessarily
                    distance = ((centroid_1[0] - centroid_2[0]) ** 2 +
                                (centroid_1[0] - centroid_2[1]) ** 2)
                    if min_distance == 0:
                        min_distance = distance
                        min_tuple = (centroids_1[centroid_1], centroids_2[centroid_2])
                    elif distance < min_distance:
                        min_distance = distance
                        min_tuple = (centroids_1[centroid_1], centroids_2[centroid_2])
            min_distances[min_distance] = min_tuple
        while len(get_components(unordered_precinct_graph)) != 1:
            # Find edge to add
            edge_to_add = min_distances[min(min_distances)]
            unordered_precinct_graph.add_edge(edge_to_add)
    # Create ordered graph
    ordered_precinct_graph = graph()
    # Add nodes from unordered graph to ordered
    for i, node in enumerate(ordered_nodes):
        ordered_precinct_graph.add_node(i, attrs=[unordered_precinct_graph.node_attributes(node)[0]])
    # Then, add EDGES.
    for node in ordered_nodes:
        neighbors = unordered_precinct_graph.neighbors(node)
        for neighbor in neighbors:
            if ordered_precinct_graph.has_edge((ordered_nodes.index(node), ordered_nodes.index(neighbor))):
                continue
            else:
                ordered_precinct_graph.add_edge((ordered_nodes.index(node), ordered_nodes.index(neighbor)))
    print(time() - start_time)
    return ordered_precinct_graph


if __name__ == "__main__":
    ordered_precinct_graph = create_graph(*sys.argv[1:5])

    # Save graph as pickle
    with open(sys.argv[5], "wb+") as f:
        pickle.dump(ordered_precinct_graph, f)
    with open('./vermont_debug.txt', "w") as f:
        f.write(str(ordered_precinct_graph))
    # Visualize graph
    visualize_graph(
        ordered_precinct_graph,
        './graph.jpg',
        lambda n : ordered_precinct_graph.node_attributes(n)[0].centroid,
        # sizes=(lambda n : ordered_precinct_graph.node_attributes(n)[0].pop/500),
        show=True
    )
"""
Usage:
python3 save_precincts.py [election_data_file] [geo_data_file] [district_file] [state] [objects_dir] [metadata_file] [population_file]

`election_data_file` - absolute path to file containing election data
                       for state. If there is no such file, this
                       argument should be "none"

`geo_data_file` - absolute path to json file containing geo data for
                  precinct bounaries state

`district_file` - absolute path to json file containing geo data for
                  district boundaries in state

`state` - name of state

`objects-dir` - absolute path to dir where pickled precinct objects
                will be stored

`metadata_file` - absolute path to file containing state metadata for
                  column names
`population_file` - absolute path to .tab, .json, or .txt file
                    containing voter registration or population file.
                    If there is none, this argument should be "none"
"""


import json
import logging
import pickle
import sys
from os.path import dirname, abspath
import warnings

from shapely.geometry import Polygon

sys.path.insert(-1, dirname(dirname(abspath(__file__))))
from gerrymandering.utils import get_point_in_polygon as gpip


logging.basicConfig(level=logging.INFO, filename="precincts.log")


def customwarn(message, category, filename, lineno, file=None, line=None):
    logging.warning(warnings.formatwarning(message, category, filename, lineno))

warnings.showwarning = customwarn


def polygon_to_shapely(polygon):
    """
    Converts list-type polygon `shape` to
    `shapely.geometry.Polygon`
    """
    tuple_polygon = [[tuple(coord) for coord in linear_ring]
                     for linear_ring in polygon]
    return Polygon(tuple_polygon[0], tuple_polygon[1:])


def save(state, precinct_dict, district_dict, objects_dir):
    """
    Save the list of precincts for a state to a file
    """
    file = f'{objects_dir}/{state}.pickle'
    with open(file, 'wb+') as f:
        pickle.dump([precinct_dict, district_dict], f)


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
    Removes redundant quotes, but will probably do more later
    """
    try:
        if string[0] in ["\"", "'"] and string[-1] in ["\"", "'"]:
            return string[1:-1]
        else:
            return string
    except TypeError:  # most likely that `string` was of type `int`
        return str(string)


def area(linear_pair):
    """
    Uses shoelace theorem to calculate area.
    Takes in linear_pair and returns a float
    """
    area = 0
    left_area = 0
    right_area = 0
    index = 0
    for num, point in enumerate(linear_pair):
        if point == linear_pair[-1]:
            left_area += point[0]*linear_pair[0][1]
            right_area += point[1]*linear_pair[0][0]
        else:
            left_area += point[0]*linear_pair[num+1][1]
            right_area += point[1]*linear_pair[num+1][0]
    area = abs(left_area - right_area)/2
    return area

def hole_remover(precinct_dict, pop, dem_keys, rep_keys, dem_cols, rep_cols):
    num_of_holes = 0
    unaccounted_holes = 0
    already_checked_holes = []
    for num, precinct in enumerate(precinct_dict):
        if precinct in already_checked_holes:
            continue
        if len(precinct_dict[precinct]) > 1:
            num_of_holes += 1
            hole_pop = 0
            hole_dem = {key : 0 for key in dem_keys}
            hole_rep = {key : 0 for key in rep_keys}
            hole_precinct_ids = []
            for check_precinct in precinct_dict:
                if (check_precinct == precinct) or (check_precinct in already_checked_holes):
                    continue
                for hole in precinct_dict[precinct][1:]:
                    if gpip([hole], precinct_dict[check_precinct][0][0]):
                        if check_precinct in hole_precinct_ids:
                            continue
                        hole_precinct_ids.append(check_precinct)
                        hole_pop += pop[check_precinct]
                        for key in dem_keys:
                            hole_dem[key] += dem_cols[check_precinct][key]
                        for key in rep_keys:
                            hole_rep[key] += rep_cols[check_precinct][key]
                        
            total_pop = hole_pop + pop[precinct]
            total_dem = {key: (dem_cols[precinct][key] + hole_dem[key]) for key in dem_keys}
            total_rep = {key: (rep_cols[precinct][key] + hole_rep[key]) for key in rep_keys}
            if len(hole_precinct_ids) == 0:
                unaccounted_holes += 1
            for hole_precinct in hole_precinct_ids:
                already_checked_holes.append(hole_precinct)
                del pop[hole_precinct]
                del dem_cols[hole_precinct]
                del rep_cols[hole_precinct]
            pop[precinct] = total_pop
            dem_cols[precinct] = total_dem
            rep_cols[precinct] = total_rep
            # Keep only the outer shell of the holy precinct
            precinct_dict[precinct] = [precinct_dict[precinct][0]]
    for hole in already_checked_holes:
        del precinct_dict[hole]
    return (already_checked_holes, num_of_holes, unaccounted_holes)

class Precinct:
    """
    Represents a voting precinct

    params:
        `coords` - 2-d list of x and y coordinates of vertices
        `state` - state that precinct is from
        `vote_id` - name from id system consistent 
                    between harvard and election-geodata files
        `population` - population of the precinct
        `d_election_data` - dict of name of vote to
                            number of votes.
                            e.g. {"g2002_GOV_dv": 100}
        `r_election_data` - above but for republicans.
    """

    @staticmethod
    def generate_from_files(election_data_file, geo_data_file,
                            district_file, state, objects_dir,
                            state_metadata_file, population_file):
        """
        Creates precinct objects for state from necessary information

        params:
            `election_data_file` - path to file containing
                                  election data for precinct (.tab (or .json, if necessary))

            `geo_data_file` - path to file containing geodata
                              for precinct. (.json or .geojson)

            `district_file` - path to file containing district geojson

            `state` - name of state containing precinct

            `objects_dir` - path to dir where serialized
                            list of precincts is to be stored

            `state_metadata_file` - path to file containing ids to match

            `population_file` - path to file containing population, if applicable

        Note: precincts from geodata that can't be matched to election
              data will be saved with voter counts of -1
        """

        precinct_dict = []  # where the final precincts will be stored

        with open(state_metadata_file, 'r') as f:
            STATE_METADATA = json.load(f)

        with open(geo_data_file, 'r') as f:
            geo_data = json.load(f)


        dem_keys = STATE_METADATA[state]["dem_keys"]
        rep_keys = STATE_METADATA[state]["rep_keys"]
        json_id = STATE_METADATA[state]["geo_id"]
        json_pop = STATE_METADATA[state]["pop_key"]
        ele_id = STATE_METADATA[state]["ele_id"]

        # Create list of precinct ids in geodata
        precinct_geo_ids = []
        for num, precinct in enumerate(geo_data['features']):
            if state == "colorado":
                precinct_geo_ids.append(
                    tostring(precinct['properties'][json_id])[1:])
            else:
                if len(json_id) > 1:
                    precinct_id = ' '
                    for thing in json_id:
                        precinct_id += precinct['properties'][thing]
                    precinct_geo_ids.append(tostring(precinct_id))
                else:
                    precinct_geo_ids.append(
                        tostring(precinct['properties'][json_id[0]]))

        # whether or not there is an
        # individual population file
        if population_file != 'none':
            is_population_data = True
            with open(population_file, 'r') as f:
                population_data = f.read().strip()

            population_data_rows = [row.split('\t') for row in population_data.split('\n')]

            population_data_columns = [[population_data_rows[x][y] for x in range(len(population_data_rows))]
                for y in range(len(population_data_rows[0]))]
            population_ele_ids = {population_data_columns[num][0]: population_data_columns[num][1:] for 
            num, column in enumerate(population_data_columns)}

            pop = {}
            # Conditionals for each state
        else:
            is_population_data = False
            # Population is in geodata
            pop = {p["properties"][json_id][1:] if state == "colorado"
            else ' '.join([p['properties'][x] for x in json_id]) if len(json_id) > 1
            else p["properties"][json_id[0]]:
            convert_to_int(sum(num for num in 
            p["properties"][json_pop])) if len(json_pop) > 1
            else convert_to_int(p["properties"][json_pop[0]])
            for p in geo_data["features"]}

        # whether or not there is an
        # individual election data file
        if election_data_file != "none":
            # the election_data is in .json, but population is in a different one
            # in this case, we choose to use the election_json, and just take population from the other file
            # ele_id is the id for election_data then, and geo_id is the one for population
            # for ids, use the election_data .json 
            if election_data_file[-4:] == 'json':
                with open(election_data_file, 'r') as f:
                    election_data = json.load(f)
                # match geo_id and ele_id with a dictionary
                population_to_election = {}
                # conditionals for each state
                '''
                failed arkansas
                '''
                # if state == 'arkansas':
                #     for precinct in geo_data["features"]:
                #         dict_len = len(population_to_election)
                #         precinct_id = precinct["properties"][json_id[0]].lower()
                #         for index in range(len(precinct_id) - 5):
                #             if precinct_id[index:index+4] == 'ward':
                #                 precinct_id.replace('ward', '')
                #                 break
                #         precinct_matched = False
                #         for precinct in election_data["features"]:
                #             if precinct["properties"][ele_id].lower() == precinct_id:
                #                 population_to_election[precinct_id] = precinct["properties"][ele_id]
                #                 precinct_matched = True
                #         if dict_len != (len(population_to_election) - 1):
                #             print('failed to match ids, precinct_ids are: ', precinct_id)
                # create population dictionary
                pop = {}
                # fill population dictionary with conversion data.
                for precinct in geo_data["features"]:
                    id = precinct["properties"][json_id]
                    pop = precinct["properties"][json_pop]
                    pop[population_to_election[id]] = pop

                # create dem, rep cols
                # They have the following format: {precinct_id : [53,157,38,31]}
                dem_cols = {precinct["properties"][ele_id] : 
                            [precinct["properties"][dem_key] for dem_key in dem_keys] 
                            for precinct in election_data["features"]}
                rep_cols = {precinct["properties"][ele_id] : 
                            [precinct["properties"][rep_key] for rep_key in rep_keys]
                            for precinct in election_data["features"] }
                # create precinct_coords dictionary
                precinct_coords = {precinct["properties"][ele_id]: precinct["geometry"]["coordinates"] 
                                for precinct in election_data}
            else:
                is_election_data = True
                with open(election_data_file, 'r') as f:
                    election_data = f.read().strip()
                
                data_rows = [row.split('\t') for row in election_data.split('\n')]
                if state == "missouri":
                    data_rows = data_rows[:4814]

                # ensure all rows have same length
                for i, row in enumerate(data_rows[:]):
                    if len(row) != (l := len(data_rows[0])):
                        if len(row) > l:
                            lst = row[:l]
                        else:
                            lst = row[:]
                            while len(lst) < l:
                                lst.append("")
                        data_rows[i] = lst

                # 2-d list with each sublist being a column in the
                # election data file
                data_columns = [[data_rows[x][y] for x in range(len(data_rows))]
                                for y in range(len(data_rows[0]))]
                # keys: data categories; values: lists of corresponding values
                # for each precinct
                ele_data = {column[0]: column[1:] for column in data_columns}


                # geodata and election data are in separate files
                # [[precinct_id1, col1], [precinct_id2, col2]]
                precinct_ele_ids = [[tostring(p), n]
                                    for n, p in enumerate(ele_data[ele_id])]
                
                # keys: precinct ids.
                # keys of values: keys in `ele_data` that correspond
                #                 to vote counts.
                # values of values: number of votes for given party
                #                   in that election.
                dem_cols = {
                    precinct[0]: {
                        key: convert_to_int(ele_data[key][precinct[1]])
                        for key in dem_keys
                    } for precinct in precinct_ele_ids
                }
                rep_cols = {
                    precinct[0]: {
                        key: convert_to_int(ele_data[key][precinct[1]])
                        for key in rep_keys
                    } for precinct in precinct_ele_ids
                }
                
                precinct_coords = {}
                for precinct_id in precinct_geo_ids:
                    for precinct in geo_data["features"]:
                        if state == "colorado":
                            geo_data_id = precinct["properties"][json_id][1:]
                        if state == "maryland":
                            geo_data_id = ' 0'.join([precinct["properties"][x] for x in json_id])
                        elif len(json_id) == 1:
                            geo_data_id = precinct["properties"][json_id[0]]
                        else:
                            geo_data_id = ' '.join([precinct["properties"][x] for x in json_id])
                        if tostring(geo_data_id) == str(precinct_id):
                            precinct_coords[precinct_id] = \
                                precinct['geometry']['coordinates']
        else:
            is_election_data = False
            # there is only a json file - no election data

            dem_cols = {}
            rep_cols = {}
            precinct_coords = {}
            for precinct in geo_data['features']:
                if state == 'maryland':
                    precinct_id = ' 0'.join([precinct['properties'][x] for x in json_id])
                if len(json_id) > 1:
                    precinct_id = ' '.join([precinct['properties'][x] for x in json_id])
                else:
                    precinct_id = precinct['properties'][json_id[0]]
                dem_cols[precinct_id] = \
                    {key: convert_to_int(precinct['properties'][key])
                     for key in dem_keys}
                rep_cols[precinct_id] = \
                    {key: convert_to_int(precinct['properties'][key])
                     for key in rep_keys}
                precinct_coords[precinct_id] = \
                    precinct['geometry']['coordinates']

        print('number of precincts before multipolygon splitting:', len(precinct_coords))    
        # Check for multi-polygons and split them into seperate precincts, by area
        multi_polygon_index = []
        new_polygon_ids = {}
        for precinct in precinct_coords:
            total_pop = int(pop[precinct])
            dem_votes = {dem_key: dem_cols[precinct][dem_key] for dem_key in dem_keys}
            rep_votes = {rep_key: rep_cols[precinct][rep_key] for rep_key in rep_keys}
            if len(precinct_coords[precinct]) > 1:
                try:
                    _ = precinct_coords[precinct][0][0][0][0]
                except TypeError:
                    # In the future, if we want to do anything with polygons with holes (but this doesn't include multipolygons with holes)
                    continue
                multi_polygon_index.append(precinct)
                total_area = 0
                for num, precinct_polygon in enumerate(precinct_coords[precinct]):
                    outer_area = area(precinct_polygon[0])
                    inner_area = 0
                    if len(precinct_polygon) > 1:
                        for linear_ring in precinct_polygon[1:]:
                            inner_area += area(linear_ring)
                    polygon_area = outer_area - inner_area
                    total_area += polygon_area
                    # Creates variables polygon0_area, polygon1_area, ... depending 
                    # on number of polygons in multipolygon
                    exec('polygon' + str(num) + '_area = polygon_area')
                for num, precinct_polygon in enumerate(precinct_coords[precinct]):
                    polygon_id = str(precinct) + str(num)
                    # Append new population, vote counts based on area, 
                    # To do this, take the precinct_id of the orginial multipolygon
                    # then add 0, 1,... as needed depending on number of seperate polygons
                    exec('polygon_ratio = (polygon' + str(num) + '_area)/total_area')
                    exec('pop[polygon_id] = polygon_ratio * total_pop')
                    for key in dem_votes:
                        exec('dem_cols[polygon_id] = {key: polygon_ratio * dem_votes[key]}')
                    for key in rep_votes:
                        exec('rep_cols[polygon_id] = {key: polygon_ratio * rep_votes[key]}')
                    exec('new_polygon_ids[polygon_id] = precinct_coords[precinct][num]')
                    exec('precinct_geo_ids.append(polygon_id)')
                # Delete originial multi-precinct from population, election_data
                del pop[precinct]
                del dem_cols[precinct]
                del rep_cols[precinct]
                precinct_geo_ids.remove(precinct)

        for multi in reversed(multi_polygon_index):
            del precinct_coords[multi]
        for id, geo in new_polygon_ids.items():
            precinct_coords[id] = geo

        print('number of precincts after multipolygons, before holes:', len(precinct_coords))
        # Then check for holy precincts (precincts with a hole in them.) 
        # Consider them (and the hole precincts) inside one precinct,
        # adding together vote and population counts.
        number_of_holes = 0
        for precinct in precinct_coords:
            if len(precinct_coords[precinct]) > 1:
                number_of_holes += 1
        print('number of holes at start, ', number_of_holes)

        # Holy precincts: precinct with hole(s) inside. Hole precincts: precinct in hole
        hole_precincts = hole_remover(precinct_coords, pop, dem_keys, rep_keys, dem_cols, rep_cols)
        for hole in hole_precincts[0]:
            precinct_geo_ids.remove(hole)
        print('number of holes removed:', hole_precincts[1])
        print('number of precincts removed:', len(hole_precincts[0]))
        print('total # of precincts after holes:',  len(precinct_coords))
        print('holes without precincts:', hole_precincts[2])
        # get election and geo data (separate processes for whether or
        # not there is an individual election data file)
        if is_election_data:
            # append precinct objects to precinct_dict
            for precinct_id in precinct_geo_ids:
                # if precinct id corresponds to any json obejcts
                if precinct_id in (
                        precinct_ids_only := [precinct[0] for precinct
                                            in precinct_ele_ids]):

                    precinct_row = precinct_ele_ids[
                        precinct_ids_only.index(precinct_id)][1]

                    precinct_dict.append(Precinct(
                        precinct_coords[precinct_id],
                        state,
                        precinct_id,
                        pop[precinct_id],
                        dem_cols[precinct_id],
                        rep_cols[precinct_id]
                    ))
                else:
                    warnings.warn(
                        f"Precinct from {state} with id {precinct_id} was " \
                        + "not found in election data."
                    )
                    precinct_dict.append(Precinct(
                        precinct_coords[precinct_id],
                        state,
                        precinct_id,
                        pop[precinct_id],
                        {"placeholder":-1},
                        {"placeholder":-1}
                    ))

        else:
            # append precinct objects to precinct_dict
            for precinct_id in precinct_coords.keys():
                precinct_dict.append(Precinct(
                    precinct_coords[precinct_id],
                    state,
                    precinct_id,
                    pop[precinct_id],
                    dem_cols[precinct_id],
                    rep_cols[precinct_id]
                ))

        # get district boundary coords
        with open(district_file, 'r') as f:
            district_dict = json.load(f)

        # save precinct list to state file
        try:
            save(precinct_dict[0].state, precinct_dict,
                 district_dict, objects_dir)
        except IndexError:
            raise Exception("No precincts saved to precinct list.")


    def __init__(self, coords, state, vote_id, population,
                 d_election_data, r_election_data):
        
        # coordinate data
        self.coords = polygon_to_shapely(coords)
        
        # meta info
        self.vote_id = vote_id
        self.state = state
        self.population = population

        # election data
        self.d_election_data = d_election_data
        self.r_election_data = r_election_data

        self.d_election_sum = sum(self.d_election_data.values())
        self.r_election_sum = sum(self.r_election_data.values())
        
        try:
            self.dem_rep_ratio = self.d_election_sum / self.r_election_sum
        except ZeroDivisionError:
            # it won't get a ratio as an attribute so we can
            # decide what to do with the dem and rep sums later.
            pass


    

if __name__ == "__main__":

    args = sys.argv[1:]

    if len(args) != 7:
         raise TypeError(
            "Incorrect number of arguments: (see __doc__ for usage)")
    
    Precinct.generate_from_files(*args)

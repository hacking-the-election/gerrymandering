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
import warnings
from collections import Counter


logging.basicConfig(level=logging.INFO, filename="precincts.log")


def customwarn(message, category, filename, lineno, file=None, line=None):
    logging.warning(warnings.formatwarning(message, category, filename, lineno))

warnings.showwarning = customwarn


def save(state, precinct_list, district_dict, objects_dir):
    """
    Save the list of precincts for a state to a file
    """
    file = f'{objects_dir}/{state}.pickle'
    with open(file, 'wb+') as f:
        pickle.dump([precinct_list, district_dict], f)


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

    def __init__(self, coords, state, vote_id, population,
                 d_election_data, r_election_data):
        
        # coordinate data
        self.coords = coords
        
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


    @classmethod
    def generate_from_files(cls, election_data_file, geo_data_file,
                            district_file, state, objects_dir,
                            state_metadata_file, population_file):
        """
        Creates precinct objects for state from necessary information

        params:
            `election_data_file` - path to file containing
                                  election data for precinct (.tab (or .json, if necessary))

            `geo_data_file` - path to file containing geodata
                              for precinct. (.json or .geojson)

            `state` - name of state containing precinct

            `objects_dir` - path to dir where serialized
                            list of precincts is to be stored

            `state_metadata_file` - path to file containing ids to match

            `population_file` - path to file containing population, if applicable

        Note: precincts from geodata that can't be matched to election
              data will be saved with voter counts of -1
        """

        precinct_list = []  # where the final precincts will be stored

        with open(state_metadata_file, 'r') as f:
            STATE_METADATA = json.load(f)

        with open(geo_data_file, 'r') as f:
            geo_data = json.load(f)


        dem_keys = STATE_METADATA[state]["dem_keys"]
        rep_keys = STATE_METADATA[state]["rep_keys"]
        json_id = STATE_METADATA[state]["geo_id"]
        json_pop = STATE_METADATA[state]["pop_key"]


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
            else convert_to_int(p["properties"][json_pop][0])
            for p in geo_data["features"]}

        # whether or not there is an
        # individual election data file
        if election_data_file != "none":
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
            ele_id = STATE_METADATA[state]["ele_id"]
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
            
        # get election and geo data (separate processes for whether or
        # not there is an individual election data file)

        if is_election_data:

            # append precinct objects to precinct_list
            for precinct_id in precinct_geo_ids:
                # if precinct id corresponds to any json obejcts
                if precinct_id in (
                        precinct_ids_only := [precinct[0] for precinct
                                            in precinct_ele_ids]):

                    precinct_row = precinct_ele_ids[
                        precinct_ids_only.index(precinct_id)][1]

                    precinct_list.append(Precinct(
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
                    precinct_list.append(Precinct(
                        precinct_coords[precinct_id],
                        state,
                        precinct_id,
                        pop[precinct_id],
                        {"placeholder":-1},
                        {"placeholder":-1}
                    ))

        else:
            # append precinct objects to precinct_list
            for precinct_id in precinct_coords.keys():
                print(precinct_id)
                precinct_list.append(Precinct(
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
            save(precinct_list[0].state, precinct_list,
                 district_dict, objects_dir)
        except IndexError:
            raise Exception("No precincts saved to precinct list.")


if __name__ == "__main__":

    args = sys.argv[1:]

    if len(args) < 6:
         raise TypeError(
            "Incorrect number of arguments: (see __doc__ for usage)")
    
    Precinct.generate_from_files(*args)

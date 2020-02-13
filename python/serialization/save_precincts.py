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


logging.basicConfig(level=logging.WARNING, filename="precincts.log")


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
                                  election data for precinct (.tab)

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

        with open(state_metadata_file, 'r') as f:
            STATE_METADATA = json.load(f)

        with open(geo_data_file, 'r') as f:
            geo_data = json.load(f)

        if election_data_file != "none":
            seperate_json = 'yes'
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

                
        if population_file != 'none':
            with open(population_file, 'r') as fileobj:
                population_data = fileobj.read().strip()
            data_rows = [row.split('\t') for row in election_data.split('\n')]
            data_columns = [[data_rows[x][y] for x in range(len(data_rows))]
                for y in range(len(data_rows[0]))]
            if state == 'maryland':
                pass

                
        if state in STATE_METADATA.keys():
            seperate_json = 'yes'
            json_id = STATE_METADATA[state]["geo_id"]

            json_pop = STATE_METADATA[state]["pop_key"]

            # [[precinct_id1, col1], [precinct_id2, col2]]
            ele_id = STATE_METADATA[state]["ele_id"]
            precinct_ele_ids = [[tostring(p), n]
                                for n, p in enumerate(ele_data[ele_id])]

            # list of precinct ids that are in geodata and election data
            precinct_geo_ids = []
            for precinct in geo_data['features']:
                if state == "colorado":
                    precinct_geo_ids.append(
                        tostring(precinct['properties'][json_id])[1:])
                else:
                    precinct_geo_ids.append(
                        tostring(precinct['properties'][json_id]))

            dem_keys = STATE_METADATA[state]["dem_keys"]
            rep_keys = STATE_METADATA[state]["rep_keys"]

            pop = {p["properties"][json_id][1:] if state == "colorado"
                   else p["properties"][json_id]:
                   convert_to_int(p["properties"][json_pop])
                   for p in geo_data["features"]}
            
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


        else:
            seperate_json = 'no'
            dem_cols = {}
            rep_cols = {}
            pop = {}
            precinct_geo_ids = []
            if state == 'georgia':
                json_id = 'ID'
                json_pop = 'POPULATION'
                dem_keys = ['PRES16D', 'SEN16D']
                rep_keys = ['PRES16R', 'SEN16R']
                for precinct in geo_data['features']:
                    dem_cols[precinct['properties'][json_id]] = {key: precinct['properties'][key] for key in dem_keys}
                    rep_cols[precinct['properties'][json_id]] = {key: precinct['properties'][key] for key in rep_keys}
                    pop[precinct['properties'][json_id]] = precinct['properties'][json_pop]
                    precinct_geo_ids.append(precinct['properties'][json_id])

        # keys: precinct ids
        # values: lists of coords
        precinct_coords = {}

        for precinct_id in precinct_geo_ids:
            geo_precinct_ids = []
            for precinct in geo_data["features"]:
                if state == "colorado":
                    geo_data_id = precinct["properties"][json_id][1:]
                else:
                    geo_data_id = precinct["properties"][json_id]
                if tostring(geo_data_id) == precinct_id:
                    precinct_coords[precinct_id] = \
                        precinct['geometry']['coordinates']

        precinct_list = []
        print(precinct_coords['6094217'])
        # append precinct objects to precinct_list
        if seperate_json == 'yes':
            for precinct_id in precinct_geo_ids:
                # if precinct id corresponds to any json obejcts
                if precinct_id in (
                        precinct_ids_only := [precinct[0] for precinct
                                            in precinct_ele_ids]):

                    precinct_row = precinct_ele_ids[
                        precinct_ids_only.index(precinct_id)][1]

                    # json object from geojson
                    # that corresponds with precinct_id
                    precinct_geo_data = []
                    for precinct in geo_data['features']:
                        if state == "colorado":
                            if precinct['properties'][json_id][1:] == precinct_id:
                                precinct_geo_data = precinct           
                        else:
                            if precinct['properties'][json_id] == precinct_id:
                                precinct_geo_data = precinct

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
                        f"Precinct from {state} with id {precinct_id} was not found in\
    election data."
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
            for precinct_id in precinct_geo_ids:
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

    if len(args) < 7:
         raise TypeError( "Incorrect number of arguments: (see __doc__ for usage)")
    
    Precinct.generate_from_files(args[0], args[1], args[2],
                                 args[3], args[4], args[5]
                                 args[6])

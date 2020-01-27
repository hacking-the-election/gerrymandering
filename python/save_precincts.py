"""
Usage:
python3 save_precincts.py [election_data_file] [geo_data_file] [state] [json_id] [json_pop] [objects_dir]

`election_data_file` - path to file containing election data for state

`geo_data` - path to json file containing geo data for state

`state` - name of state

`json_id` - name of json attribute that corresponds to precinct id

`json_pop` - name of json attribute that corresponds to precinct total population


When run with above inputs, saves serialized file containing
precint-level election and geodata for a state in `objects_dir`
"""


import json
from os import mkdir, _exit
from os.path import abspath, dirname, isdir
import pickle
import warnings


def customwarn(message, category, filename, lineno, file=None, line=None):
    sys.stdout.write(warnings.formatwarning(message, category, filename, lineno))


def save(state, precinct_list, objects_dir):
    """
    Save the list of precincts for a state to a file
    """
    file = f'{objects_dir}/{state}.pickle'
    with open(file, 'wb+') as f:
        pickle.dump(precinct_list, f)


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


class Precinct:
    """
    Represents a voting precinct

    params:
        `coords` - 2-d list of x and y coordinates of vertices
        `name` - name of precinct
        `state` - state that precinct is from
        `vote_id` - name from id system consistent 
                    between harvard and election-geodata files
        `population` - population of the precinct
        `d_election_data` - dict of name of vote to
                            number of votes.
                            e.g. {"g2002_GOV_dv": 100}
        `r_election_data` - above but for republicans.
    """

    def __init__(self, coords, name, state, vote_id, population,
                 d_election_data, r_election_data):
        
        # coordinate data
        self.coords = coords
        
        # meta info
        self.name = name
        self.vote_id = vote_id
        self.state = state
        self.population = population

        # election data
        self.d_election_data = d_election_data
        self.r_election_data = r_election_data

        self.d_election_sum = 0
        self.r_election_sum = 0

        # only include data for elections for which there is data for both parties
        if state == "alabama":
            # 2d list like a dict
            ordered_d_election_data = [[key, value] for key, value in
                                       self.d_election_data.items()]
            ordered_r_election_data = [[key, value] for key, value in
                                       self.r_election_data.items()]
            dem_elections = [key.replace("_d_", "") for key in
                             [item[0] for item in ordered_d_election_data]]
            rep_elections = [key.replace("_r_", "") for key in
                             [item[0] for item in ordered_r_election_data]]
            for i, election in enumerate(dem_elections):
                if election in rep_elections:
                    self.d_election_sum += ordered_d_election_data[i][1]
                    self.r_election_sum += \
                        ordered_r_election_data[rep_elections.index(election)][1]

        elif state == "alaska":
            self.d_election_sum = self.d_election_data["usp_d_08"]
            self.r_election_sum = self.r_election_data["usp_r_08"]

        elif state == "arizona":
            dem_elections = [key[:-3] for key in self.d_election_data.keys()]
            rep_elections = [key[:-3] for key in self.r_election_data.keys()]

            for election in dem_elections:
                if election in rep_elections:
                    self.d_election_sum += self.d_election_data[election + "dem"]
                    self.r_election_sum += self.r_election_data[election + "rep"]
        
        elif state == "colorado":
            dem_elections = [key[:-1] for key in self.d_election_data.keys()]
            rep_elections = [key[:-1] for key in self.r_election_data.keys()]

            for election in dem_elections:
                if election in rep_elections:
                    self.d_election_sum += self.d_election_data[election + "d"]
                    self.r_election_sum += self.r_election_data[election + "r"]

        elif state == "connecticut":
            self.d_election_sum += self.d_election_data["democrat"]
            self.r_election_sum += self.r_election_data["republican"]

        try:
            self.dem_rep_ratio = self.d_election_sum / self.r_election_sum
        except ZeroDivisionError:
            # it won't get a ratio as an attribute so we can
            # decide what to do with the dem and rep sums later.
            pass


    def __str__(self):
        return (f"name: {self.name}\n"
                f"d_election_sum: {self.d_election_sum}\n"
                f"r_election_sum: {self.r_election_sum}\n"
                f"population: {self.population}\n"
                f"id: {self.vote_id}\n")


    @classmethod
    def generate_from_files(cls, election_data_file, geo_data_file, state, json_id, json_pop, objects_dir):
        """
        Creates precinct objects for state from necessary information

        params:
            `election_data_file` - path to file containing
                                  election data for precinct (.tab)

            `geo_data_file` - path to file containing geodata for precinct
                              (.json or .geojson)

            `state` - name of state containing precinct

            `json_id` - the name of the json attribute the precinct ids 
                        should be matched with

            `json_pop` - the name of the json attribute the total precinct
                         populations are stored in

            `objects_dir` - path to dir where serialized list of precincts
                            is to be stored
        """

        with open(geo_data_file, 'r') as f:
            geo_data = json.load(f)

        with open(election_data_file, 'r') as f:
            election_data = f.read().strip()
        
        data_rows = [row.split('\t') for row in election_data.split('\n')]
        # 2-d list with each sublist being a column in the
        # election data file
        data_columns = [[data_rows[x][y] for x in range(len(data_rows))]
                        for y in range(len(data_rows[0]))]
        # keys: data categories; values: lists of corresponding values
        # for each precinct
        data_dict = {column[0]: column[1:] for column in data_columns}
        
        # keys in `data_dict` that correspond to party vote counts
        if state == "alabama":
            dem_keys = [key for key in data_dict.keys() if "_d_" in key]
            rep_keys = [key for key in data_dict.keys() if "_r_" in key]

        elif state == "alaska":
            dem_keys = ["usp_d_08"]
            rep_keys = ["usp_r_08"]

        elif state == "arizona":
            dem_keys = [key for key in data_dict.keys() if key[-3:] == "dem"]
            rep_keys = [key for key in data_dict.keys() if key[-3:] == "rep"]
        
        elif state == "colorado":
            dem_keys = [key for key in data_dict.keys() if key[-1] == "d"]
            rep_keys = [key for key in data_dict.keys() if key[-1] == "r"]

        elif state == "connecticut":
            dem_keys = ["democrat"]
            rep_keys = ["republican"]

        
        # [[precinct_id1, col1], [precinct_id2, col2]]
        precinct_ids = Precinct.find_precincts(data_dict, state)

        # Looks for precinct name (or if there is one)
        if "precinct_name" in (keys := data_dict.keys()):
            precinct_name_col = "precinct_name"
        elif "precinct" in keys:
            precinct_name_col = "precinct"
        elif "name10" in keys:
            precinct_name_col = "name10"
        else:
            precinct_name_col = False
        
        # keys: precinct ids.
        # keys of values: keys in `data_dict` that correspond
        #                 to vote counts.
        # values of values: number of votes for given party
        #                   in that election.
        dem_cols = {
            precinct[0]: {
                key: convert_to_int(data_dict[key][precinct[1]])
                for key in dem_keys
            } for precinct in precinct_ids
        }
        rep_cols = {
            precinct[0]: {
                key: convert_to_int(data_dict[key][precinct[1]])
                for key in rep_keys
            } for precinct in precinct_ids
        }

        precinct_list = []
        
        # list of precinct ids that are in geodata and election data
        precinct_geo_list = []
        for precinct in geo_data['features']:
            if state == "colorado":
                precinct_geo_list.append(precinct['properties'][json_id][1:])
            else:
                precinct_geo_list.append(precinct['properties'][json_id])

        # append precinct objects to precinct_list
        for precinct_id, precinct_row in precinct_ids:

            # if precinct id corresponds to any json obejcts
            if precinct_id in precinct_geo_list:

                # json object from geojson that corresponds with preinct with precinct_id
                precinct_geo_data = []
                for precinct in geo_data['features']:
                    if (precinct['properties'][json_id][1:]
                            if state == "colorado"
                            else precinct['properties'][json_id]) \
                            == precinct_id:
                        precinct_geo_data = precinct

                # if there is a column for names
                if precinct_name_col:
                    precinct_list.append(Precinct(
                        precinct_geo_data['geometry']['coordinates'],
                        data_dict[precinct_name_col][precinct_row],
                        state,
                        precinct_id,
                        precinct_geo_data['properties'][json_pop],
                        dem_cols[precinct_id],
                        rep_cols[precinct_id]
                    ))
                else:
                    if precinct_row == 0:
                        warnings.warn(f"No precincts names found in state of {state}")
                    precinct_list.append(Precinct(
                        precinct_geo_data['geometry']['coordinates'],
                        "None",
                        state,
                        precinct_id,
                        precinct_geo_data['properties'][json_pop],
                        dem_cols[precinct_id],
                        rep_cols[precinct_id]
                    ))
            else:
                warnings.warn(f"Precinct with id {precinct_id} was not found in geodata.")

        # save precinct list to state file
        try:
            save(precinct_list[0].state, precinct_list, objects_dir)
        except IndexError:
            raise Exception("No precincts saved to precinct list.")

    
    @classmethod
    def find_precincts(cls, data_dict, state):
        """
        Finds precinct id attributes that can be matched with geojson
        """

        if state in ["alabama", "arizona", "connecticut"]:
            precincts = data_dict["geoid10"]
            ids = [[precinct[1:-1], i] for i, precinct in enumerate(precincts)]
            return ids

        elif state == "alaska":
            precincts = data_dict["vtdst10"]
            ids = [[precinct[1:7], i] for i, precinct in enumerate(precincts)]
            return ids

        elif state == "colorado":
            precincts = data_dict["geoid10"]
            ids = [[precinct, i] for i, precinct in enumerate(precincts)]
            return ids


if __name__ == "__main__":

    import sys

    warnings.showwarning = customwarn

    args = sys.argv[1:]

    if len(args) < 6:
        raise TypeError("Incorrect number of arguments: (see __doc__ for usage)")
    
    Precinct.generate_from_files(args[0], args[1], args[2],
                                 args[3], args[4], args[5])
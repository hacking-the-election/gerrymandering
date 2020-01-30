/*=======================================
 state.cpp:                     k-vernooy
 last modified:               Sun, Jan 20

 Definitions of state methods for parsing
 from geodata and election data (see data
 specs in root directory for information)
========================================*/

#include "../include/shape.hpp"  // class definitions
#include "../include/util.hpp"   // array modification functions
#include <algorithm>             // for std::find and std::distance

#define VERBOSE 1  // print progress messages

vector<vector<string> > parse_sv(string tsv, string delimiter) {
    /*
        takes a tsv file as string, returns
        two dimensional array of cells and rows
    */

    vector<vector<string> > parsed; // to be added to + returned

    stringstream file(tsv); // to iterate over lines
    string line;            // to hold current line

    while (getline( file, line ) ) { // loop through lines

        vector<string> row;

        // split by tab
        size_t pos = 0;

        while ((pos = line.find(delimiter)) != string::npos) {
            row.push_back(line.substr(0, pos));       // add current value to row
            line.erase(0, pos + delimiter.length());  // remove current value from string
        }

        row.push_back(line); // add the last value to the row
        parsed.push_back(row); // add the row to the array
    }

    return parsed;
}

bool check_column(vector<vector<string> > data_list, int index) {
    for (int i = 0; i < data_list.size(); i++)
        if (data_list[i][index].size() == 0)
            return false;

    return true;
}

map<string, vector<int> > parse_voter_data(string voter_data) {
    /*
        from a string in the specified format,
        creates a map with the key of the precinct
        name and vector as `"name": {dem_vote, rep vote}`
    */

    vector<vector<string> > data_list // two dimensional
        = parse_sv(voter_data, "\t"); // array of voter data

    vector<string> watchlist;

    int precinct_id_col = -1; // the column index that holds precinct id's
    // int county_id_col = -1; // the column index that holds precinct id's

    string precinct_id_col_header = "geoid10";   // the column header name

    // indices of usable data
    vector<int> d_index;
    vector<int> r_index;

    int index = 0;

    // search for usable data cols
    for (string item : data_list[0]) {

        if (item == precinct_id_col_header)
            precinct_id_col = index;

        // split header by underscore
        vector<string> party = split(item, "_");

        // attempt to identify the party of the column
        string par = party[party.size() - 1];

        if (par == "dv" || par == "rv") {
            // set string `end` to opposite of `par`
            string end = "dv";
            if (par == "dv")
                end = "rv";

            party.pop_back(); // remove the party from the name

            // get the general column name
            vector<string>::iterator old_data = find(watchlist.begin(), watchlist.end(), join(party, "_") + "_" + end);

            // if it's already in the watchlist, it's useable data
            if ( old_data != watchlist.end()) {

                if (check_column(data_list, index)) {
                    if (par == "dv")
                        // since the one we've found is right,
                        // add the one we're at right now
                        d_index.push_back(index);
                    else
                        r_index.push_back(index);
                }

                // find old data already in watchlist, pair it with new data

                int index2 = distance(
                                data_list[0].begin(),

                                find(data_list[0].begin(),
                                    data_list[0].end(),
                                    join(party, "_") + "_" + end
                                )
                            );

                if (check_column(data_list, index2)) {
                    if (par == "dv")
                        // add old watchlisted data
                        r_index.push_back(index2);
                    else
                        d_index.push_back(index2);
                }

            }
            else {
                // otherwise, add it to the watchlist
                watchlist.push_back(join(party, "_") + "_" + par);
            }
        }

        index++; // increment index for tracking purposes
    }


    map<string, vector<int> > parsed_data;

    // iterate over each precinct
    for (int x = 1; x < data_list.size(); x++) {

        string id = split(data_list[x][precinct_id_col], "\"")[1]; // get the precinct id
        // string id = data_list[x][precinct_id_col];

        int demT = 0, repT = 0;

        // get the right voter columns, add to party total
        for (int i = 0; i < d_index.size(); i++) {
            string d = data_list[x][d_index[i]];
            string r = data_list[x][r_index[i]];
            demT += stoi(d);
            repT += stoi(r);
        }
        
        parsed_data[id] = {demT, repT};
        // set the voter data of the precinct in the map
    }

    return parsed_data; // return the filled map
}

vector<vector<float> > string_to_vector(string str) {
    // takes a json array string and returns a parsed vector

    // remove instances of "[" or "]" from the string
    str.erase(remove(str.begin(), str.end(), '['), str.end());
    str.erase(remove(str.begin(), str.end(), ']'), str.end());

    vector<string> list = split(str, ","); // split string by commas
    vector<vector<float> > v;

    // loop through comma split vector
    int i = 1;
    for (int i = 0; i < list.size(); i += 2) {
        // add as pair of floating point coordinates
        v.push_back( { stof(list[i]), stof(list[i + 1]) } );
    }

    return v;
}

vector<Shape> parse_coordinates(string geoJSON) {

    // parses a geoJSON state into an array of shapes

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Shape> shapes_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        string coords;
        string id = "";
        int pop = 0;
        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember("GEOID10")) {
            id = shapes["features"][i]["properties"]["GEOID10"].GetString();
        }
        else {
            cout << "If this is parsing precincts, you have no precinct id." << endl;
            cout << "If future k-vernooy runs into this error, it means that GEOID10 in your geoJSON in your voter data is missing. To fix... maybe try a loose comparison of the names?" << endl;
        }

        // get the population from geodata
        if (shapes["features"][i]["properties"].HasMember("POP100")) {
            pop = shapes["features"][i]["properties"]["POP100"].GetInt();
        }
        else
            cout << "\e[31merror: \e[0mNo population data in geodata" << endl;
        
        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();

        // vector parsed from coordinate string
        vector<vector<float> > border = string_to_vector(coords);

        // create shape from border, add to array
        Shape shape(border, id);
        shape.pop = pop;
        shapes_vector.push_back(shape);
    }

    return shapes_vector;
}

vector<Precinct> merge_data( vector<Shape> precinct_shapes, map<string, vector<int> > voter_data) {

    /*
        returns an array of precinct objects given
        geodata (shape objects) and voter data
        in the form of a map for a list of precincts
    */

    vector<Precinct> precincts;

    for (Shape precinct_shape : precinct_shapes) {
        // iterate over shapes array, get the id of the current shape
        string p_id = precinct_shape.shape_id;
        vector<int> p_data = {0, 0}; // the voter data to be filled

        if ( voter_data.find(p_id) == voter_data.end() ) {
            // there is no matching id in the voter data
            cout << "error: the id in the geodata, \e[41m" << p_id << "\e[0m, has no matching key in voter_data" << endl;
            cout << "the program will continue, but the voter_data for the precinct will be filled with 0,0." << endl;
        }
        else {
            // get the voter data of the precinct
            p_data = voter_data[p_id];
        }

        // create a precinct object and add it to the array
        Precinct precinct = Precinct(precinct_shape.border, p_data[0], p_data[1], p_id);
        precinct.pop = precinct_shape.pop;
        precincts.push_back(precinct);
    }

    return precincts; // return precincts array
}

vector<vector<float> > generate_state_border(vector<Precinct> precincts) {
    // given an array of precincts, return the border of the state
    return {{0,0}}; //
}

State State::generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON) {

    /*
        Parse precinct and district geojson, along with
        precinct voter data, into a State object.
    */

    //! Should probably allocate memory with malloc
    //! Will be some outrageously large vectors here

    // generate shapes from coordinates
    if (VERBOSE) cout << "generating coordinate array from precinct file..." << endl;
    vector<Shape> precinct_shapes = parse_coordinates(precinct_geoJSON);

    if (VERBOSE) cout << "generating coordinate array from district file..." << endl;
    vector<Shape> district_shapes = parse_coordinates(district_geoJSON);

    vector<District> districts;

    for ( Shape district_shape : district_shapes ) {
        // create district objects from shape objects
        districts.push_back(District(district_shape.border));
    }

    // create a vector of precinct objects from border and voter data
    if (VERBOSE) cout << "parsing voter data from tsv..." << endl;
    map<string, vector<int> > precinct_voter_data = parse_voter_data(voter_data);

    if (VERBOSE) cout << "merging parsed geodata with parsed voter data into precinct array..." << endl;
    vector<Precinct> precincts = merge_data(precinct_shapes, precinct_voter_data);

    // a dummy state shape
    vector<vector<float> > state_shape = generate_state_border(precincts);

    // generate state data from files
    if (VERBOSE) cout << "generating state with shape arrays..." << endl;
    State state = State(districts, precincts, state_shape);

    return state; // return the state object
}

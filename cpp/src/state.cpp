/*===========================================
State methods for parsing from geodata and
 election data methods, and for running and 
 writing the entire quantification algorithm
===========================================*/

#include "../include/shape.hpp"
#include "../include/util.hpp"
#include <algorithm>

vector<vector<string> > parse_tsv(string tsv) {
    vector<vector<string> > parsed;
    stringstream file( tsv ) ;
    string line;
    string delimiter = "\t";

    // loop through lines
    while (getline( file, line ) ) {
        vector<string> row;
        // split by tab
        size_t pos = 0;

        while ((pos = line.find(delimiter)) != string::npos) {
            row.push_back(line.substr(0, pos));
            line.erase(0, pos + delimiter.length());
        }

        row.push_back(line);
        parsed.push_back(row);
    }

    return parsed;
}

map<string, vector<int> > parse_voter_data(string voter_data) {
    // from a string in the specified format
    // (see DATA_SPECS.md in the root data directory),
    // creates a map with the key of the precinct
    // name and an array with the data

    // parse data into 2d array
    vector<vector<string> > data_list = parse_tsv(voter_data);
    vector<string> watchlist;
    int ed_precinct = NULL;
    vector<int> d_index;
    vector<int> r_index;
    int index = 0;
    // search for usable data cols
    for (string item : data_list[0]) {
        if (item == "ed_precinct")
            ed_precinct = index;
        // split header by underscore
        vector<string> party = split(item, "_");
        string par = party[party.size() - 1];

        if (par == "dv" || par == "rv") {
            string end = "dv";

            if (par == "dv")
                    end = "rv";
            // get the general column name
            vector<string>::iterator old_data = find(watchlist.begin(), watchlist.end(), join(party, "") + end);
            // if it's already in the watchlist, it's useable data
            if ( old_data != watchlist.end()) {
                cout << "True" << endl;
                party.pop_back();

                // find old data already in watchlist, pair it with new data

                if (par == "dv") {
                    // since the one we've found is right,
                    // ådd the one we're at right now
                    cout << "d data at " << index << endl;
                    d_index.push_back(index);
                }
                else {
                    cout << "r data at " << index << endl;
                    r_index.push_back(index);
                }


                int index2 = distance(
                                data_list[0].begin(), 
                                find(data_list[0].begin(), data_list[0].end(), join(party, "_") + end)
                            );

                if (par == "dv") {
                    cout << "r data at " << index2 << endl;

                    r_index.push_back(index2);
                }
                else {
                    cout << "d data at " << index2 << endl;

                    d_index.push_back(index2);
                }
            }
            else {
                // otherwise, add it to the watchlist
                cout << "Adding " << join(party,"_") << endl;
                watchlist.push_back(join(party, "_"));
            }
        }
        index++;
    }

    // search for ed_precinct col
    
    map<string, vector<int> > parsed_data;

    for (int x = 1; x < data_list.size(); x++) {
        // for each precinct
        // get the precinct id
        string id = data_list[x][ed_precinct];
        cout << "id: " << id << endl;

        int demT, repT;

        // get the right voter columns, add to party total
        for (int i = 0; i < d_index.size(); i++) {
            cout << "adding dem vote: " << data_list[x][d_index[i]] << endl;
            cout << "adding rep vote: " << data_list[x][r_index[i]] << endl;
            // demT += stoi(data_list[x][d_index[i]]);
            // repT += stoi(data_list[x][r_index[i]]);
        }

        parsed_data[id] = {demT, repT};

    }

    return parsed_data;
}

vector<Shape> parse_coordinates(string geoJSON) { 
    // parses a geoJSON state into an array of shapes

    vector<Shape> shapes_vector;  // vector of shapes to be returned
    json shapes = json::parse(geoJSON).at("features"); // parse geoJSON string

    for ( int i = 0; i < shapes.size(); i++ ) {
        json coords;
        string id = "";

        // see if the geoJSON contains the shape id
        try { 
            id = shapes[i].at("properties").at("VTDST10");
        } 
        catch (...) {
            cout << "If this is parsing precincts, you have no precinct id." << endl;
            cout << "If future k-vernooy runs into this error, it means that either VTSD_10 in your geoJSON or ed_precinct in your voter data is missing. To fix... maybe try a loose comparison of the names?" << endl;
        }

        // check for embedded arrays
        if ( shapes[i].at("geometry").at("coordinates").size() == 1 ) 
            coords = shapes[i].at("geometry").at("coordinates")[0];
        else 
            coords = shapes[i].at("geometry").at("coordinates").dump(4);

        // replace following with parsed coords variable
        vector<vector<float> > border = {{2,5}, {2,4}, {1,4}};
        
        // create shape from border, add to array
        Shape shape(border, id);
        shapes_vector.push_back(shape);
    }

    return shapes_vector;
}

vector<Precinct> merge_data( vector<Shape> precinct_shapes, map<string, vector<int> > voter_data) {
    // returns an array of precinct objects given 
    // geodata (shape objects) and voter data
    // in the form of a map for a list of precincts

    vector<Precinct> precincts;

    for (Shape precinct_shape : precinct_shapes) {
        // create a precinct obj, add to array
        string p_id = precinct_shape.shape_id;
        vector<int> p_data = {0, 0};

        if ( voter_data.find(p_id) == voter_data.end() ) {
            cout << "error: the id in the geodata, \e[41m" << p_id << "\e[0m, has no matching key in voter_data" << endl;
            cout << "the program will continue, but the voter_data for the precinct will be filled with 0,0." << endl;
        } 
        else {
            p_data = voter_data[p_id];
        }


        Precinct precinct = Precinct(precinct_shape.border, p_data[0], p_data[1]);
        precincts.push_back(precinct);
    }

    return precincts;
}

vector<vector<float> > generate_state_border(vector<Precinct> precincts) {
    // given an array of precincts, return the border of the state
    return {{0,0}};
}

State State::generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON) {
    // parse the coordinates into shape objects
    //! Should probably allocate memory with malloc here
    //! Will be some outrageously large vectors here
    vector<Shape> precinct_shapes = parse_coordinates(precinct_geoJSON);
    vector<Shape> district_shapes = parse_coordinates(district_geoJSON);

    vector<District> districts;

    for ( Shape district_shape : district_shapes ) {
        districts.push_back(District(district_shape.border));
        // deallocate district_shapes
    }

    // create a vector of precinct objects from border and voter data
    map<string, vector<int> > precinct_voter_data = parse_voter_data(voter_data);
    vector<Precinct> precincts = merge_data(precinct_shapes, precinct_voter_data);

    // a dummy state shape
    vector<vector<float> > state_shape = generate_state_border(precincts);

    // generate state data from files
    State state = State(districts, precincts, state_shape);
    return state;
}
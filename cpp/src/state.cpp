/*===========================================
State methods for parsing from geodata and
 election data methods, and for running and 
 writing the entire quantification algorithm
===========================================*/

#include "../include/shape.hpp"

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
    
    // search for ed_precinct col, write into an
    map<string, vector<int> > parsed_data = {
        {"01-001", {3,3}}
    };

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
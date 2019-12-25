/*===========================================
State methods for parsing from geodata and
 election data methods, and for running and 
 writing the entire quantification algorithm
============================================*/

#include "../include/shape.hpp"

map<string, vector<int> > parse_voter_data(string voter_data) {
    // from a string in the specified format
    // (see DATA_SPECS.md in the root data directory),
    // creates a map with the key of the precinct
    // name and an array with the data

    map<string, vector<int> > parsed_data;

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
        catch (const std::exception& e) { 
            cout << "If this is parsing precincts, you have no precinct id." << endl;
        }

        // check for embedded arrays
        if ( shapes[i].at("geometry").at("coordinates").size() == 1 ) 
            coords = shapes[i].at("geometry").at("coordinates")[0];
        else 
            coords = shapes[i].at("geometry").at("coordinates").dump(4);

        // replace following with parsed coords variable
        vector<vector<int> > border = {{2,5}, {2,4}, {1,4}};
        
        // create shape from border, add to array
        Shape shape(border, id);
        shapes_vector.push_back(shape);
    }

    return shapes_vector;
}

State State::generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON) {
    cout << "generating from file... " << endl;

    vector<Shape> precincts = parse_coordinates(precinct_geoJSON);
    vector<Shape> districts = parse_coordinates(district_geoJSON);

    map<string, vector<int> > precinct_voter_data = parse_voter_data(voter_data);

}

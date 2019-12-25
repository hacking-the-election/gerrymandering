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

vector<Precinct*> parse_coordinates(string geoJSON, map<string, vector<int> > voter_data) { 
    // ===================================
    // parses a geoJSON state with 
    // precincts into an array of precincts
    // ====================================

    vector<Precinct*> precinct_list;

    json precincts = json::parse(geoJSON).at("features");
    int precinct_num = precincts.size();

    for ( int i = 0; i < precincts.size(); i++ ) {
        string id = precincts[i].at("properties").at("VTDST10");
        json coords;

        if ( precincts[i].at("geometry").at("coordinates").size() == 1 ) 
            coords = precincts[i].at("geometry").at("coordinates")[0];
        else 
            coords = precincts[i].at("geometry").at("coordinates").dump(4);

        // placeholder shape, replace with coords string parsed into vector
        vector<vector<int> > shape = {{2,5}, {2,4}, {1,4}};

        // using the id variable or the name of the precinct,
        // replace the following with the correct data
        int demv = 13;
        int repv = 15;

        Precinct* precinct = new Precinct(shape, demv, repv);
        precinct_list.push_back( precinct );
        delete precinct;

        // cout << coords << endl;
        // cout << id << endl;
    }
    
    return precinct_list;
}

State State::generate_from_file(string geoJSON, string voter_data) {
    cout << "generating from file... " << endl;
    // map<string, vector<int> > parsed_voter_data = parse_voter_data(voter_data);
}

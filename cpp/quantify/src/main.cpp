/*===============================================================
Precinct geodata compiled by Nathaniel Kelso and Michal Migurski.

Linked election data from the Harvard Dataverse.

In election-geodata geoJSON files, the "VTDST10" property 
is the same as the Harvard "ed_precinct" col
===============================================================*/

#include "../include/shape.hpp"
#include <SDL2/SDL.h>

void parse_coordinates(string geoJSON) { 
    // =============================
    // parses a geoJSON state with 
    // precincts into a state object
    // =============================
    json precincts = json::parse(geoJSON).at("features");
    int precinct_num = precincts.size();

    for ( int i = 0; i < precincts.size(); i++ ) {
        string id = precincts[i].at("properties").at("VTDST10");
        json coords;

        if ( precincts[i].at("geometry").at("coordinates").size() == 1 ) 
            coords = precincts[i].at("geometry").at("coordinates")[0];
        else 
            coords = precincts[i].at("geometry").at("coordinates").dump(4);

        // cout << id << endl;
    }
}

int main(int argc, char* argv[]) {

    if ( argc != 2 ) {
        cout << "gerry: \e[31merror: \e[0mMust have 2 arguments: geoJSON file and election data" << endl;
        return 1;
    }

    ifstream t(argv[1]);
    stringstream buffer;
    buffer << t.rdbuf();
    string geoJSON = buffer.str();

    cout << "parsing coords..." << endl;
    parse_coordinates(geoJSON);
    

    return 0;
}
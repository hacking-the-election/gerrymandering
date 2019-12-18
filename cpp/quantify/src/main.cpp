/*===============================================================
Precinct geodata compiled by Nathaniel Kelso and Michal Migurski.

Linked election data from the Harvard Dataverse.

In election-geodata geoJSON files, the "VTDST10" property 
is the same as the Harvard "ed_precinct" col
===============================================================*/

#include "../include/shape.hpp"
#include <SDL2/SDL.h>

vector<Precinct> parse_coordinates(string geoJSON) { 
    // =============================
    // parses a geoJSON state with 
    // precincts into a state object
    // =============================

    vector<Precinct> precincts;

    Document json;
    json.Parse(geoJSON.c_str());

    Value& s = json["features"][0]["geometry"]["coords"];
    s.SetInt(s.GetInt() + 1);

    StringBuffer jsbuffer;
    Writer<StringBuffer> writer(jsbuffer);
    json.Accept(writer);
    // return jsbuffer.GetString();

}

int main(int argc, char* argv[]) {

    if ( argc != 2 ) {
        cout << "must have 2 arguments: geoJSON file and election geodata";
    }

    parse_coordinates(string(argv[1]));
    
    // ifstream t(argv[1]);
    // stringstream buffer;
    // buffer << t.rdbuf();
    // string geoJSON = buffer.str();

    return 0;
}
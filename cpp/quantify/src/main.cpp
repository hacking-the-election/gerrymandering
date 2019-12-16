#include "../include/shape.hpp"
#include <SDL2/SDL.h>

string parse_coordinates(string geoJson) { 
    // parses a geoJSON state with precincts into a state object

    // create json object from geojson string
    Document json;
    json.Parse(geoJson.c_str());

    // parse geojson
    Value& s = json["features"][0]["geometry"]["coords"];
    s.SetInt(s.GetInt() + 1);

    StringBuffer jsbuffer;
    Writer<StringBuffer> writer(jsbuffer);
    json.Accept(writer);
    return jsbuffer.GetString();
}

int main(int argc, char* argv[]) {

    if ( argc != 2 ) {
        cout << "must have 2 arguments: geoJSON file and election geodata";
    }
    // ifstream t(argv[1]);
    // stringstream buffer;
    // buffer << t.rdbuf();
    // string geoJSON = buffer.str();

    return 0;
}
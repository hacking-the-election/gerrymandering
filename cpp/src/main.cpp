/*
- precinct geodata compiled by Nathaniel Kelso and Michal Migurski.
- linked election data from the Harvard Dataverse.
- In election-geodata geoJSON files, the "VTDST10" property 
  is the same as the Harvard "ed_precinct" col
*/

#include "../include/shape.hpp"

int main(int argc, char* argv[]) {

    if ( argc != 2 ) {
        cout << "To parse into a state file, please : geoJSON file and election data" << endl;
        return 1;
    }

    // read files into strings
    string geoJSON = readf(argv[1]);
    string voter_data = readf(argv[2]);
    State state = State::generate_from_file(geoJSON, voter_data);    

    return 0;
}
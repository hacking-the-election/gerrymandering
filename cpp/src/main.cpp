/*
- precinct geodata compiled by Nathaniel Kelso and Michal Migurski.
- linked election data from the Harvard Dataverse.
- In election-geodata geoJSON files, the "VTDST10" property 
  is the same as the Harvard "ed_precinct" col
*/

#include "../include/shape.hpp"
#include "../include/util.hpp"

int main(int argc, char* argv[]) {
    /* a test for parsing data into a state object
       takes 3 arguments:
           precinct geoJSON
           precinct voter data
           district geoJSON     
    */

    // read files into strings
    string precinct_geoJSON = readf(argv[1]);
    string voter_data = readf(argv[2]);
    string district_geoJSON = readf(argv[3]);

    State state = State::generate_from_file(precinct_geoJSON, voter_data, district_geoJSON);    
    state.serialize("ak");
    cout << state.to_json();
    return 0;
}
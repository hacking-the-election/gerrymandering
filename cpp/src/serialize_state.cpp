/*
- precinct geodata compiled by Nathaniel Kelso and Michal Migurski.
- linked election data from the Harvard Dataverse.
- In election-geodata geoJSON files, the "VTDST10" property 
  is the same as the Harvard "ed_precinct" col
*/

#include "../include/shape.hpp"
#include "../include/util.hpp"

int main(int argc, char* argv[]) {
    /* parsing data into a binary state object
       takes 3 arguments:
           precinct geoJSON
           precinct voter data
           district geoJSON     
    */

    if (argc != 5) {
        cerr << "serialize_state: usage: <precinct.geojson> <electiob.tab> <district.geojson> <write_path>" << endl;
        return 1;
    }

    // read files into strings
    string precinct_geoJSON = readf(argv[1]);
    string voter_data = readf(argv[2]);
    string district_geoJSON = readf(argv[3]);
    string write_path = string(argv[4]);

    // std::ofstream ofs("ak");
    State state = State::generate_from_file(precinct_geoJSON, voter_data, district_geoJSON);
    state.write_binary(write_path);

    return 0;
}
/*=======================================
 serialize_state.cpp:           k-vernooy
 last modified:               Sun, Jan 19
 
 Given precinct and district geodata, and
 precinct voter data, writes to a binary 
 state file.

 - precinct geodata from github @nvkelso
 - election data from harvard dataverse
========================================*/

#include "../../include/shape.hpp"
#include "../../include/util.hpp"

int main(int argc, char* argv[]) {
    /* parsing data into a binary state object
       takes 3 arguments:
           precinct geoJSON
           precinct voter data
           district geoJSON     
    */

    if (argc != 5) {
        // must provide correct arguments
        cerr << "serialize_state: usage: <precinct.geojson> <electiob.tab> <district.geojson> <write_path>" << endl;
        return 1;
    }

    // read files into strings
    string precinct_geoJSON = readf(argv[1]);
    string voter_data = readf(argv[2]);
    string district_geoJSON = readf(argv[3]);
    
    // path to write binary file to
    string write_path = string(argv[4]);

    // generate state from files
    State state = State::generate_from_file(precinct_geoJSON, voter_data, district_geoJSON);
    state.write_binary(write_path); // write as binary
    
    cout << "State " << state.name << " successfully written to " << write_path << endl;
    cout << "Num of precincts: " << state.precincts.size() << endl;
    cout << "Num of districts: " << state.state_districts.size() << endl;

    int dem, rep, pop;

    for (Precinct pre : state.precincts) {
        dem += pre.dem;
        rep += pre.rep;
        pop += pre.pop;
    }    

    cout << "Total pop: " << pop << endl;
    cout << "Total dem: " << dem << endl;
    cout << "Total rep: " << rep << endl;

    return 0;
}
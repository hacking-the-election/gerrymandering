/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                Sat, Feb 8
 
 Run community generation algorithm and 
 print coordinates as geojson for a given
 state object
========================================*/

#include "../../../include/shape.hpp"
#include "../../../include/util.hpp"
#include "../../../include/geometry.hpp"

int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    if (argc != 2) {
        cerr << "generate_communities: usage: <state.dat>" << endl;
        return 1;
    }

    // read binary file from path
    string read_path = string(argv[1]);
    State state = State::read_binary(read_path);
    state.precincts = state.state_precincts;

    // cout << "generating exterior border" << endl;

    int districts_in_state = state.state_districts.size();
    // vector<Precinct_Group> political_communities = state.generate_communities(districts_in_state, 0.5, 0.2, 0.15);

    // cout << "Finished generating communities for " << read_path 
        //  << ", writing to " << read_path << "_communities.dat" << endl;
    
    // write as binary
    return 0;
}
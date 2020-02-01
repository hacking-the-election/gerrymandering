/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                Sat, Feb 1
 
 Run community generation algorithm and 
 print coordinates as geojson for a given
 state object
========================================*/

#include "../include/shape.hpp"
#include "../include/util.hpp"

int main(int argc, char* argv[]) {
    /* 
        run community algorithm from 
        binary state object
    */

    if (argc != 2) {
        // must provide correct arguments
        cerr << "generate_communities: usage: <state.dat>" << endl;
        return 1;
    }


    // path to write binary file to
    string read_path = string(argv[1]);

    // generate state from files
    State state = State::read_binary(read_path);
    // state.generate_communities();

    // write as binary
    return 0;
}
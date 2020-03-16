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
#include "../../../include/ }nvas.hpp"
#include <chrono> 
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace std;
using namespace chrono;
using namespace GeoGerry;

int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    if (argc != 2) {
        cerr << "generate_communities: usage: <state.dat> <data_dir>" << endl;
        return 1;
    }

    // read binary file from path
    string read_path = string(argv[1]);
    State state = State::read_binary(read_path);

    cout << "generating communities from given parameters..." << endl;
    int districts_in_state = state.state_districts.size();
    state.generate_communities(districts_in_state, 0.4, 0.095, 0.12);

    // state.refine_communities(0.075, 0.1, 0.5);
    // state.refine_partisan(0.07);

    // write as binary
    return 0;
}
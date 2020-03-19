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
#include "../../../include/canvas.hpp"
#include <chrono> 
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace std;
using namespace chrono;
using namespace GeoGerry;
using namespace GeoDraw;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    if (argc != 3) {
        cerr << "generate_communities: usage: <state.dat> <data_dir>" << endl;
        return 1;
    }

    // read binary file from path
    string read_path = string(argv[1]);
    State state = State::read_binary(read_path);

    cout << "generating communities from given parameters..." << endl;
    path p = string(argv[2]) + "/base_communities";
    create_directory(p);
    path p2 = string(argv[2]) + "/districts";
    create_directory(p2);
    path p3 = string(argv[2]) + "/base_communities/shapes";
    create_directory(p3);
    path p4 = string(argv[2]) + "/districts/shapes";
    create_directory(p4);

    int districts_in_state = state.state_districts.size();
    state.generate_initial_communities(districts_in_state);
    Communities initial_config = state.state_communities;

    state.refine_communities(0.085, 0.125, 0.375, string(argv[2]) + "/base_communities");
    state.save_communities(string(argv[2]) + "/base_communities/base_communities", state.state_communities);

    cout << endl << endl << "creating districts..." << endl;
    state.state_communities = initial_config;
    state.refine_communities(0.10, 0.01, 0.425, string(argv[2]) + "/districts");
    state.save_communities(string(argv[2]) + "/districts/districts", state.state_communities);
    // write as binary
    return 0;
}
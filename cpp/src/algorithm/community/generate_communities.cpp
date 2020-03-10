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

    cout << "generating communities from given parameters..." << endl;
    int districts_in_state = 2;  // state.state_districts.size();
    // state.generate_communities(districts_in_state, 0.5, 0.2, 0.15);

    state.read_communities("community_vt");
    double sum = 0;

    // GeoDraw::Canvas c(800, 800);
    // c.add_shape(state.state_communities);
    // c.draw();

    state.refine_compactness(0);


    // write as binary
    return 0;
}
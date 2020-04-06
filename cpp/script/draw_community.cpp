/*=======================================
 draw_community.cpp:            k-vernooy
 last modified:                Sun, Apr 5
 
 Reads a state object and draws its
 community object list to a canvas
========================================*/

#include "../../include/shape.hpp"
#include "../../include/util.hpp"
#include "../../include/geometry.hpp"
#include "../../include/canvas.hpp"
#include <boost/filesystem.hpp>

using namespace std;
using namespace GeoGerry;
using namespace GeoDraw;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    if (argc != 3) {
        cerr << "draw_community: usage: <state.dat> <saved_community>" << endl;
        return 1;
    }

    // read binary file from path
    State state = State::read_binary(string(argv[1]));
    state.read_communities(string(argv[2]));

    // draw communities
    Canvas canvas(900, 900);
    canvas.add_shape(state.state_communities);
    canvas.draw();

    return 0;
}
/*=======================================
 draw_community.cpp:            k-vernooy
 last modified:                Sun, Apr 5
 
 Reads a state object and draws its
 community object list to a canvas
========================================*/

#include <iostream>
#include <boost/filesystem.hpp>
#include <iomanip>

#include "../include/shape.hpp"
#include "../include/util.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"

using namespace std;
using namespace Geometry;
using namespace Graphics;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    // draw communities
    Canvas canvas(900, 900);

    // read binary file from path
    State state = State::read_binary(argv[1]);
    canvas.add_graph(state.network);

    cout << "drawing" << endl;
    canvas.draw();
    return 0;
}
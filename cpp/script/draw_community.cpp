/*=======================================
 draw_community.cpp:            k-vernooy
 last modified:                Sun, Apr 5
 
 Reads a state object and draws its
 community object list to a canvas
========================================*/

#include <iostream>
#include <boost/filesystem.hpp>
#include <iomanip>

#include "../include/community.hpp"
#include "../include/shape.hpp"
#include "../include/util.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include <chrono> 

using namespace std::chrono; 

using namespace std;
using namespace Gerrymandering::Geometry;
using namespace Gerrymandering::Graphics;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    // draw communities
    Canvas canvas(900, 900);

    // read binary file from path
    // vector<string> states = {"connecticut", "indiana", "iowa", "mass_connected", "new_hampshire", "new_jersey", "new_york", "north_carolina", "pennsylvania", "vermont"};

    // for (string st : states) {
        State state = State::from_binary(argv[1]);
        canvas.add_graph(state.network);
    // }

    canvas.draw();

    return 0;
}
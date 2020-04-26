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
#include <chrono> 

using namespace std::chrono; 

using namespace std;
using namespace Geometry;
using namespace Graphics;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    // draw communities
    cout << argc << endl;
    Canvas canvas(900, 900);

    // read binary file from path
    vector<string> states = {"../../data/bin/cpp/new_hampshire.dat","../../data/bin/cpp/new_york.dat", "../../data/bin/cpp/vermont.dat", "../../data/bin/cpp/massachusetts.dat", "../../data/bin/cpp/connecticut.dat", "../../data/bin/cpp/pennsylvania.dat", "../../data/bin/cpp/new_jersey.dat"};
    for (string st : states) {
        State state = State::read_binary(st);
        canvas.add_graph(state.network);
    }

    canvas.draw();

    return 0;
}
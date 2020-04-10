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
using namespace GeoGerry;
using namespace GeoDraw;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    // draw communities
    Canvas canvas(900, 900);

    vector<string> files = {"../../data/bin/cpp/vermont.dat", "../../data/bin/cpp/new_hampshire.dat", "../../data/bin/cpp/delaware.dat"};
    LinearRing test_poly = State::read_binary(files[0]).precincts[0].hull;

    cout << test_poly.get_center()[0] << ", "  << test_poly.get_center()[1] << endl;

    for (string file : files) {
        // read binary file from path
        State state = State::read_binary(file);
        canvas.add_graph(state.network);
    }

    cout << "drawing" << endl;
    canvas.draw();
    return 0;
}
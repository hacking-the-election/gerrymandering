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
using namespace hte::Geometry;
using namespace hte::Graphics;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    // draw communitie
    vector<string> states = {"alabama", "arizona", "delaware", "illinois", "indiana", "iowa", "kansas", "massachusetts", "michigan", "mississippi", "colorado", "nebraska", "nevada", "ohio", "oklahoma", "missouri", "pennsylvania", "vermont"};
    Canvas canvas(700, 700);
    
    for (string st : states) {
        State state = State::from_binary("../../data/bin/cpp/" + st + ".dat");
        canvas.add_outlines(to_outline(state));
    }

    canvas.draw_to_window();

    return 0;
}

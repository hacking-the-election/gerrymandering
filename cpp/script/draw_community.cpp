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
    State state = State::from_binary(argv[1]);
    Canvas canvas(700, 700);
    canvas.add_outlines(to_outline(state));
    // canvas.save_image(ImageFmt::BMP, "test");
    canvas.save_image(ImageFmt::SVG, "test");
    // canvas.draw_to_window();

    return 0;
}

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
using namespace boost::filesystem;

int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    Canvas canvas(700, 700);
    State state = State::from_binary(argv[1]);
    canvas.add_outlines(to_outline(state.network));
    canvas.draw_to_window();
    // path p ("../../data/bin/cpp");
    // directory_iterator end_itr;

    // // cycle through the directory
    // for (directory_iterator itr(p); itr != end_itr; ++itr) {
    //     // If it's not a directory, list it. If you want to list directories too, just remove this check.
    //     if (is_regular_file(itr->path())) {
    //         // assign current file name to current_file and echo it out to the console.
    //         string current_file = itr->path().string();
    //         if (current_file != "../../data/bin/cpp/README.md" && current_file != "../../data/bin/cpp/hawaii.state" && current_file != "../../data/bin/cpp/alaska.state" && current_file != "../../data/bin/cpp/texas.state" && current_file != "../../data/bin/cpp/wisconsin.state") {
    //             cout << current_file << endl;
    //             State state = State::from_binary(current_file);
    //             canvas.add_outlines(to_outline(state));
    //         }
    //     }
    // }
    

    // canvas.draw_to_window();

    return 0;
}

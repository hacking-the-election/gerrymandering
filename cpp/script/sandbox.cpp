/*=======================================
 sandbox.cpp:                   k-vernooy
 last modified:               Sun May 31
 
 A simple testing environment with the
 hacking-the-election library
========================================*/

#include <boost/filesystem.hpp>
#include <iostream>
#include <chrono>

#include "../include/util.hpp"
#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/community.hpp"
#include "../include/quantification.hpp"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace std;
using namespace hte::Geometry;
using namespace hte::Graphics;


int main(int argc, char* argv[]) {
    
    /*
        A loaded environment with included namespaces ready for 
        graphics programs testing, compiling random binaries, and other scripts
    */

    srand(time(NULL));
    State state = State::from_binary(string(argv[1]));
    return 0;
}

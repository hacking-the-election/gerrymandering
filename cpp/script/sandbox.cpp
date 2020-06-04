/*=======================================
 sandbox.cpp:                   k-vernooy
 last modified:               Sun May 31
 
 A simple testing environment with the
 hacking-the-election library
========================================*/

#include <chrono>
#include <iostream>
#include <boost/filesystem.hpp>

#include "../include/util.hpp"
#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/community.hpp"
#include "../include/quantification.hpp"

// for the rapidjson parser
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

using namespace boost::filesystem;
using namespace std;
using namespace hte::Geometry;
using namespace hte::Graphics;
using namespace rapidjson;


int main(int argc, char* argv[]) {
    
    /*
        A loaded environment with included namespaces ready for 
        graphics programs testing, compiling random binaries, and other scripts
    */

    srand(time(NULL));

    State state = State::from_binary(argv[1]);
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(state));
    canvas.save_image(ImageFmt::SVG, argv[2]);
    return 0;
}

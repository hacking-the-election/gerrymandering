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
    
    vector<string> states = {"alabama","arizona","colorado","connecticut","georgia","idaho","illinois","indiana","iowa","kansas","louisiana","minnesota","mississippi","missouri","nebraska","nevada","new_hampshire","new_jersey","new_mexico","new_york","north_carolina","oklahoma","pennsylvania","virginia","washington"};
    Canvas canvas(900, 900);
    Canvas canvas_x(900, 900);

    for (string state_str : states) {
        cout << state_str << endl;
        State state = State::from_binary("../../data/bin/cpp/" + state_str + ".state");
        
        for (int i = 0; i < state.districts.size(); i++) {
            canvas.add_outline_group(to_outline(state.districts[i], stod(split(split(readf("../output/" + state_str + "/stats/districts/statewide.tsv"), "\n")[1], "\t")[0]), true));
            canvas_x.add_outline_group(to_outline(state.districts[i], stod(split(split(readf("../output/" + state_str + "/stats/districts/statewide.tsv"), "\n")[1], "\t")[1]), false));
        }
    }

    canvas.draw_to_window();
    canvas_x.draw_to_window();
    return 0;
}

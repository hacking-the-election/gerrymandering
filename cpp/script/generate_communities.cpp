/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                Sat, Feb 8
 
 Run community generation algorithm and 
 print coordinates as geojson for a given
 state object
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

using namespace boost::filesystem;
using namespace std;
using namespace hte::Geometry;
using namespace hte::Graphics;

#define MAX_TIME 40000


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    if (argc != 2) {
        cerr << "generate_communities: usage: <state.dat>" << endl;
        return 1;
    }

    // read binary file from path
    string read_path = string(argv[1]);
    State state = State::from_binary(read_path);
    int n_communities = state.districts.size();

    srand(time(NULL));
    Communities init_config = karger_stein(state.network, n_communities);
    // Communities init_config = load("ia_init.txt", state.network);
    // get random configuration
    Communities communities = get_communities(state.network, init_config, 0.1);
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(communities));
    canvas.save_image(ImageFmt::SVG, "test");

    // // get districts, save to district output
    // Communities districts = get_communities(state.network, communities, 0.01);
    // canvas.clear();
    // canvas.add_outlines(to_outline(districts));
    // canvas.draw_to_window();
    
    // // using 
    return 0;
}

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


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    if (argc != 3) {
        cerr << "generate_communities: usage: <state.dat> num_communities" << endl;
        return 1;
    }

    // read binary file from path
    string read_path = string(argv[1]);
    State state = State::from_binary(read_path);
    int n_communities = stoi(string(argv[2]));
    Communities cs = get_communities(state.network, n_communities);
    Canvas canvas(900, 900);
    for (Multi_Polygon district : state.districts) {
        array<double, 2> q = get_quantification(state.network, cs, district);
        cout << collapse_vals(q[0], q[1]) << endl;
        canvas.add_outlines(to_outline(district, collapse_vals(q[0], q[1])));
    }

    canvas.draw_to_window();
    return 0;
}

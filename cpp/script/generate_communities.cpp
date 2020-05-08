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

using namespace boost::filesystem;
using namespace std;
using namespace hte::Geometry;
using namespace hte::Graphics;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    // if (argc != 3) {
    //     cerr << "generate_communities: usage: <state.dat> <data_dir>" << endl;
    //     return 1;
    // }

    // read binary file from path
    string read_path = string(argv[1]);
    State state = State::from_binary(read_path);
    int n_communities = stoi(string(argv[2]));
    
    // LinearRing lr = state.precincts[14].hull;
    // bounding_box box = Polygon(lr).get_bounding_box();// ;

    // cout << "M";
    // cout << (double)(lr.border[0][0] - box[2]) / 200.0 << "," << (double)(lr.border[0][0] - box[1]) / 200.0;
    // // translate(-box[2], -box[1], true);

    // for (segment g : lr.get_segments()) {
    //     array<double, 4> s;
    //     for (int i = 0; i < 4; i++) {
    //         if (i == 0 || i == 2) {
    //             s[i] = (g[i] - box[2]) / 200.0;
    //         }
    //         else {
    //             s[i] = (g[i] - box[1]) / 200.0;
    //         }
    //         // s[i] = ((double) g[i] / (double)pow(2, 17)) + 150;
    //     }

    //     cout << "L";
    //     cout << s[0] << "," << s[1];
    //     cout << "," << s[2] << "," << s[3];
    // }

    // cout << "z" << endl;

    Communities s = get_initial_configuration(state.network, n_communities);
    
    for (int i = 0; i < s.size(); i++) {
        writef(s[i].shape.to_json(), to_string(i) + ".json");
    }

    // Canvas canvas(400, 400);
    // canvas.add_shape(s, state.network);
    // canvas.draw();
    return 0;
}

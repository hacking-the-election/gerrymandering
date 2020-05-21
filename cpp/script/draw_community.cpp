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

    if (argc < 2) cout << "get more argcs plz" << endl;
    State state = State::from_binary(argv[1]);
    Canvas canvas(900, 900);
    // for (Precinct p : state.precincts) {
    //     cout << p.get_centroid()[0] << ", " << p.get_centroid()[1] << endl;
    //     if (p.shape_id == "16049049005") {
    //         cout << p.get_centroid()[0] << ", " << p.get_centroid()[1] << endl;
    //         // canvas.add_outline(to_outline(p.hull));
    //         // canvas.draw_to_window();

    //         // canvas.clear();
    //         canvas.add_outline(to_outline(p.hull));
    //         canvas.add_outline(to_outline(generate_gon(p.get_centroid(), 2000, 50).hull));
    //         canvas.draw_to_window();
    //         break;
    //     }
    //     // if (!point_in_ring(p.get_centroid(), p.hull)) {
    //     //     cout << p.shape_id << endl;
    //     //     canvas.clear();
    //     //     canvas.add_outline(to_outline(p.hull));
    //     //     canvas.add_outline(to_outline(generate_gon(p.get_centroid(), 600, 50).hull));
    //     //     canvas.draw_to_window();
    //     // }
    // }

    cout << state.precincts[0].get_centroid()[0] << ", " << state.precincts[0].get_centroid()[1] << endl;
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
    //         if (current_file != "../../data/bin/cpp/README.md") {
    //             State state = State::from_binary(current_file);
    //             vector<string> t = split(current_file, "/");
    //             vector<string> names = split(t[t.size() - 1], ".");
    //             string name = names[0];

    //             if (name != "alaska" && name != "texas" && name != "wisconsin" && name != "hawaii" && name != "idaho" && name != "arizona" && name != "nevada")
    //                 US.add_outlines(to_outline(state.network));
    //             // cout << name << endl;

    //             // Canvas canvas(900, 900);
    //             // canvas.add_outlines(to_outline(state));
    //             // canvas.save_image(ImageFmt::BMP,"docs/renders/geo/" + name);

    //             // canvas.clear();
    //             // canvas.add_outlines(to_outline(state.network));
    //             // canvas.save_image(ImageFmt::BMP, "docs/renders/networks/" + name);
    //         }
    //     }
    // }
    

    return 0;
}

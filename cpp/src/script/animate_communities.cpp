/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                Sat, Feb 8
 
 Run community generation algorithm and 
 print coordinates as geojson for a given
 state object
========================================*/

#include "../../include/shape.hpp"
#include "../../include/util.hpp"
#include "../../include/geometry.hpp"
#include "../../include/canvas.hpp"
#include <boost/filesystem.hpp>

using namespace std;
using namespace GeoGerry;
using namespace GeoDraw;
using namespace boost::filesystem;


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    if (argc != 3) {
        cerr << "draw_community: usage: <state.dat> <anim_dir>" << endl;
        return 1;
    }

    // read binary file from path
    State state = State::read_binary(string(argv[1]));
    Anim animation(500);

    path p(argv[2]);
    directory_iterator end_itr;

    // cycle through the directory
    for (directory_iterator itr(p); itr != end_itr; ++itr) {
        if (is_regular_file(itr->path())) {
            string current_file = itr->path().string();
            state.read_communities(current_file);
            // draw communities
            cout << current_file << endl;
            Canvas canvas(900, 900);
            canvas.add_shape(state.state_communities);
            animation.frames.push_back(canvas);
            canvas.draw();
        }
    }

    cout << "Animating.." << endl;
    animation.playback();
    return 0;
}
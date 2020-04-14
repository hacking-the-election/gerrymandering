/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                Sat, Feb 8
 
 Run community generation algorithm and 
 print coordinates as geojson for a given
 state object
========================================*/

#include <chrono> 
#include <boost/filesystem.hpp>
#include <iostream>


#include "../include/shape.hpp"
#include "../include/util.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"

using namespace boost::filesystem;
using namespace std;
using namespace chrono;
using namespace Geometry;
using namespace Graphics;


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
    State state = State::read_binary(read_path);

    
    return 0;
}

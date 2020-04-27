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

#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/community.hpp"

using namespace boost::filesystem;
using namespace std;
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
    int n_communities = stoi(string(argv[2]));

    using namespace std::chrono; 
    auto start = high_resolution_clock::now(); 
    Communities s = get_initial_configuration(state.network, n_communities);

    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stop - start); 
    
    cout << duration.count() << endl; 

    Canvas canvas(300, 300);
    canvas.add_shape(s, state.network);
    canvas.draw();
    
    return 0;
}

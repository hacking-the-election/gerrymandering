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

    if (argc != 3) {
        cerr << "generate_communities: usage: <state.dat> num_communities" << endl;
        return 1;
    }

    int iter = 10;
    
    // read binary file from path
    int read_average = 0;
    string read_path = string(argv[1]);

    for (int i = 0; i < iter; i++) {
        auto start = chrono::high_resolution_clock::now();
        State state = State::from_binary(read_path);
        auto stop = chrono::high_resolution_clock::now();
        read_average += chrono::duration_cast<chrono::microseconds>(stop - start).count();
    }
    read_average /= iter;
    cout << "file read in average: " << read_average << endl;
    int n_communities = stoi(string(argv[2]));
    State state = State::from_binary(read_path);
    Communities s = get_communities(state.network, n_communities);
    return 0;
}

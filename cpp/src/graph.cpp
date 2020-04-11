/*=======================================
 graph.cpp:                     k-vernooy
 last modified:               Sat, Apr 11
 
 Definitions for graph theory related
 algorithm implementations such as
 searches and component counts.
========================================*/

#include "../include/shape.hpp"

using namespace Geometry;


int Graph::get_num_components() {
    return 0;
}


// 1) Initialize all vertices as not visited.
// 2) Do following for every vertex 'v'.
//        (a) If 'v' is not visited before, call DFSUtil(v)
//        (b) Print new line character

// DFSUtil(v)
// 1) Mark 'v' as visited.
// 2) Print 'v'
// 3) Do following for every adjacent 'u' of 'v'.
//      If 'u' is not visited, then recursively call DFSUtil(u)
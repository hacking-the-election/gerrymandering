/*===============================================
 community.cpp:                        k-vernooy
 last modified:                     Fri, Feb 28

 Definition of the gerrymandering quantification
 algorithm, given a district plan and base
 communities.

 Visit https://hacking-the-election.github.io for
 more information on this project.
 
 This algorithm has been implemented in C++ and in
 Python. If you're interested in contributing to the
 open source political movement, please contact us
 at: hacking.the.election@gmail.com
===============================================*/

#include <math.h>    // for rounding functions

#include "../../../include/shape.hpp"    // class definitions
#include "../../../include/util.hpp"     // array modification functions
#include "../../../include/geometry.hpp" // geometry modification, border functions
// #include "../../../include/canvas.hpp" // geometry modification, border functions

using namespace std;
using namespace GeoGerry;


vector<unit_interval> State::quantify_gerrymandering(vector<Multi_Shape> districts, Communities base_communities) {
    // use 
    vector<unit_interval> vals;

    for (Multi_Shape district : districts) {
        unit_interval score = 0;
        vals.push_back(score);
    }

    return vals;
}
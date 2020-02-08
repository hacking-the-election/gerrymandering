/*=======================================
 shape.cpp:                     k-vernooy
 last modified:                Sat, Feb 8
 
 Definition of generic methods for shapes, 
 precincts, districts and states that do
 not include geometric and/or mathematical
 algorithms (see geometry.cpp).
========================================*/

#include "../include/shape.hpp"   // class definitions
#include "../include/gui.hpp"     // for the draw function

double Precinct::get_ratio() {
    // retrieve ratio from precinct
    return dem / (dem + rep);
}

vector<int> Precinct::voter_data() {
    // get vector of voting data
    return {dem, rep};
}
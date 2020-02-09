/*=======================================
 shape.cpp:                     k-vernooy
 last modified:                Sun, Feb 9
 
 Definition of generic methods for shapes, 
 precincts, districts and states that do
 not include geometric and/or mathematical
 algorithms (see geometry.cpp).
========================================*/

#include "../include/shape.hpp"   // class definitions

#define REP true

double Precinct::get_ratio() {
    // retrieve ratio from precinct
    #ifdef REP
        return rep / (dem + rep);
    #endif
    #ifndef REP
        return dem / (dem + rep);
    #endif
}

vector<int> Precinct::voter_data() {
    // get vector of voting data
    return {dem, rep};
}
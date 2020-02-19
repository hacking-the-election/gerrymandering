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

double GeoGerry::Precinct::get_ratio() {
    // retrieve ratio from precinct
    #ifdef REP
        return rep / (dem + rep);
    #endif
    #ifndef REP
        return dem / (dem + rep);
    #endif
}

std::vector<int> GeoGerry::Precinct::get_voter_data() {
    // get vector of voting data
    return {dem, rep};
}

int GeoGerry::Precinct_Group::get_population() {
    // Returns total population in a Precinct_Group

    int total = 0;
    for (GeoGerry::Precinct p : precincts)
        total += p.pop;

    return total;
}
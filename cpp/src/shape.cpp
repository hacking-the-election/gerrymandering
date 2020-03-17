/*=======================================
 shape.cpp:                     k-vernooy
 last modified:                Thu Mar 12
 
 Definition of generic methods for shapes, 
 precincts, districts and states that do
 not include geometric and/or mathematical
 algorithms (see geometry.cpp).
========================================*/

#include "../include/shape.hpp"      // class definitions
#include "../include/geometry.hpp"   // class definitions
#include "../include/canvas.hpp"   // class definitions

#define REP  // use rep / (dem + rep) as ratio

double GeoGerry::Precinct::get_ratio() {
    // retrieve ratio from precinct
    if (rep == 0 && dem == 0)
        return -1;

    #ifdef REP
        return (double) rep / ((double) dem + (double) rep);
    #endif
    #ifndef REP
        return dem / (dem + rep);
    #endif
}


double GeoGerry::Precinct_Group::get_ratio() {
    
    double average = 0;
    for (Precinct p : precincts)
        average += p.get_ratio();
    average /= precincts.size();

    return average;
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

void GeoGerry::Precinct_Group::remove_precinct(GeoGerry::Precinct pre) {
    /*
        @desc: Removes a precinct from a Precinct Group and updates the border with a difference
        @params: `Precinct` pre: Precinct to be removed
        @return: none
    */

    if (std::find(precincts.begin(), precincts.end(), pre) != precincts.end()) {
        precincts.erase(std::remove(precincts.begin(), precincts.end(), pre), precincts.end());

        ClipperLib::Paths subj;
        for (Shape shape : this->border)
            subj.push_back(ring_to_path(shape.hull));

        ClipperLib::Paths clip;
        clip.push_back(ring_to_path(pre.hull));

        ClipperLib::Paths solutions;
        ClipperLib::Clipper c; // the executor

        // execute union on paths array
        c.AddPaths(subj, ClipperLib::ptSubject, true);
        c.AddPaths(clip, ClipperLib::ptClip, true);
        c.Execute(ClipperLib::ctDifference, solutions, ClipperLib::pftNonZero);

        this->border = paths_to_multi_shape(solutions).border;
    }
    else {
        GeoDraw::Canvas canvas(900, 900);
        canvas.add_shape(generate_exterior_border(*this));
        canvas.add_shape(pre);
        canvas.draw();
        
        throw GeoGerry::Exceptions::PrecinctNotInGroup();
    }
}

void GeoGerry::Precinct_Group::add_precinct(GeoGerry::Precinct pre) {
    /*
        @desc: Adds a precinct to a Precinct Group and updates the border with a union
        @params: `Precinct` pre: Precinct to be added
        @return: none
    */
   
    precincts.push_back(pre);
    if (border.size() == 0)
        border.push_back(pre.hull);
    else {
        ClipperLib::Paths subj;
        for (Shape shape : border)
            subj.push_back(ring_to_path(shape.hull));

        ClipperLib::Paths clip;
        clip.push_back(ring_to_path(pre.hull));

        ClipperLib::Paths solutions;
        ClipperLib::Clipper c; // the executor

        // execute union on paths array
        c.AddPaths(subj, ClipperLib::ptSubject, true);
        c.AddPaths(clip, ClipperLib::ptClip, true);
        c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

        this->border = paths_to_multi_shape(solutions).border;
    }
};
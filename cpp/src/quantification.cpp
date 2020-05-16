/*=======================================
 quantification.cpp:            k-vernooy
 last modified:               Sat May 16
 
 Determines how gerrymandered a district
 is given a community map
========================================*/


#include "../include/shape.hpp"
#include "../include/community.hpp"
#include "../include/geometry.hpp"

using namespace hte::Geometry;


double get_quantification(Graph& graph, Communities& communities, Multi_Polygon district) {
    // determines how gerrymandered `district` is with the `communities` map.
    // communities had better already be updated.

    double val = 0.0;
    bounding_box db = district.get_bounding_box();

    // get all communities that overlap the current district
    for (Community& c : communities) {
        if (bound_overlap(c.shape.get_bounding_box(), db)) {
            // get intersection between the two shapes

            // create paths array from polygon
            ClipperLib::Paths subj;
            for (Precinct p : c.shape.precincts)
                for (ClipperLib::Path path : shape_to_paths(p))
                    subj.push_back(path);

            ClipperLib::Paths clip;
            for (Polygon p : district.border)
                for (ClipperLib::Path path : shape_to_paths(p))
                    clip.push_back(path);

            ClipperLib::Paths solutions;
            ClipperLib::Clipper c; // the executor

            // execute union on paths array
            c.AddPaths(subj, ClipperLib::ptSubject, true);
            c.AddPaths(clip, ClipperLib::ptClip, true);
            c.Execute(ClipperLib::ctIntersection, solutions, ClipperLib::pftNonZero);
            Multi_Polygon intersection = paths_to_multi_shape(solutions);
        }
    }
    
   return val;
}

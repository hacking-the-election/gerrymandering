/*=======================================
 quantification.cpp:            k-vernooy
 last modified:               Sat May 16
 
 Determines how gerrymandered a district
 is given a community map
========================================*/

#include "../include/quantification.hpp"

double hte::Geometry::collapse_vals(double a, double b) {
    // where `a` is the republican ratio and b is the general quantification
    assert((a < 1.0 && a > 0.0));
    assert((b < 1.0 && b > 0.0));
    return (b * (2.0 * (0.5 - a)));
}


double get_population(Precinct_Group pg, Multi_Polygon mp) {
    int pop = 0;
    for (Precinct p : pg.precincts) {
        // get the overlap between mp and p
        // create paths array from polygon
        ClipperLib::Path subj = ring_to_path(p.hull);

        ClipperLib::Paths clip;
        for (Polygon p : mp.border)
            clip.push_back(ring_to_path(p.hull));

        ClipperLib::Paths solutions;
        ClipperLib::Clipper c; // the executor

        // execute union on paths array
        c.AddPath(subj, ClipperLib::ptSubject, true);
        c.AddPaths(clip, ClipperLib::ptClip, true);
        c.Execute(ClipperLib::ctIntersection, solutions, ClipperLib::pftNonZero);
        Multi_Polygon intersection = paths_to_multi_shape(solutions);
        double ratio = (intersection.get_area() / p.get_area());
        pop += ((double)p.pop * ratio);
    }

    return pop;
}


std::array<double, 2> Geometry::get_quantification(Graph& graph, Communities& communities, Multi_Polygon district) {
    // determines how gerrymandered `district` is with the `communities` map.
    // communities had better already be updated.

    double val = 0.0;
    bounding_box db = district.get_bounding_box();

    // get all communities that overlap the current district
    // double district_population = 
    

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

   return {val, 0.0};
}

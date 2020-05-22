/*=======================================
 quantification.cpp:            k-vernooy
 last modified:               Sat May 16
 
 Determines how gerrymandered a district
 is given a community map
========================================*/

#include "../include/quantification.hpp"
#include <iostream>

using namespace hte;
using namespace Geometry;
using namespace Graphics;

double hte::Geometry::collapse_vals(double a, double b) {
    // where `a` is the republican ratio and b is the general quantification
    assert((a <= 1.0 && a >= 0.0));
    assert((b <= 1.0 && b >= 0.0));
    return (a * (2.0 * (0.5 - b)));
}


double get_pop(Precinct& pre) {
    return pre.pop;
}


double hte::Geometry::get_attr_from_mask(Precinct_Group pg, Multi_Polygon mp, double (*measure)(Precinct&)) {
    double pop = 0;
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
        pop += ((double)measure(p) * ratio);
    }

    return pop;
}

LinearRing bound_to_shape(bounding_box box) {
    return (LinearRing({{box[3], box[0]}, {box[3], box[1]}, {box[2], box[1]}, {box[2], box[0]}}));
}


std::array<double, 2> Geometry::get_quantification(Graph& graph, Communities& communities, Multi_Polygon district) {
    // determines how gerrymandered `district` is with the `communities` map.
    // communities had better already be updated.
    double val = 0.0;
    bounding_box db = district.get_bounding_box();
    // get all communities that overlap the current district
    double district_population = get_attr_from_mask(Precinct_Group::from_graph(graph), district, get_pop);
    int largest_index = -1;
    int largest_pop = -1;

    for (int i = 0; i < communities.size(); i++) {
        communities[i].update_shape(graph);
        if (bound_overlap(communities[i].shape.get_bounding_box(), db)) {
            // get intersection between the two shapes
            double pop = get_attr_from_mask(communities[i].shape, district, get_pop);
            if (pop > largest_pop) {
                largest_pop = pop;
                largest_index = i;
            }
        }
    }

    return {1 - (largest_pop / district_population), 0.8};
}

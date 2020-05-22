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
    assert((a < 1.0 && a > 0.0));
    assert((b < 1.0 && b > 0.0));
    return (b * (2.0 * (0.5 - a)));
}


double hte::Geometry::get_population(Precinct_Group pg, Multi_Polygon mp) {
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
    std::cout << "getting quantification of district" << std::endl;
    double val = 0.0;
    std::cout << "bound" << std::endl;
    bounding_box db = district.get_bounding_box();

    // get all communities that overlap the current district
    std::cout << "district_population" << std::endl;
    double district_population = get_population(Precinct_Group::from_graph(graph), district);
    std::cout << district_population << std::endl;
    int largest_index = -1;
    int largest_pop = -1;

    for (int i = 0; i < communities.size(); i++) {
        if (bound_overlap(communities[i].shape.get_bounding_box(), db)) {
            // get intersection between the two shapes
            double pop = get_population(communities[i].shape, district);
            if (pop > largest_pop) {
                largest_pop = pop;
                largest_index = i;
            }
        }
    }

    return {1 - (largest_pop / district_population)};
//    return {val, 0.0};
}

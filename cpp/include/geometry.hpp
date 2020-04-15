/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Fri, Feb 28
 
 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#pragma once

#include "shape.hpp"
#include "community.hpp"
#include "../lib/clipper/clipper.hpp"

#define PI 3.14159265358979323846264338327950288


// segment and coordiante manipulation
Geometry::segment coords_to_seg(Geometry::coordinate c1, Geometry::coordinate c2);
double get_distance(Geometry::coordinate c1, Geometry::coordinate c2);
double get_distance(std::array<long long int, 2> c1, std::array<long long int, 2> c2);
double get_distance(Geometry::segment s);

// two shapes have adjacent and colinear segments
bool get_bordering(Geometry::Polygon s0, Geometry::Polygon s1);

// for clipper conversions
ClipperLib::Path ring_to_path(Geometry::LinearRing ring);
Geometry::LinearRing path_to_ring(ClipperLib::Path path);
ClipperLib::Paths shape_to_paths(Geometry::Polygon shape);
Geometry::Multi_Polygon paths_to_multi_shape(ClipperLib::Paths paths);

// get outside border from a group of precincts
Geometry::Multi_Polygon generate_exterior_border(Geometry::Precinct_Group p);

// get precincts on the inside border of a precinct group
Geometry::p_index_set get_inner_boundary_precincts(Geometry::Precinct_Group shape);
Geometry::p_index_set get_inner_boundary_precincts(Geometry::p_index_set precincts, Geometry::State state);
Geometry::p_index_set get_bordering_precincts(Geometry::Precinct_Group shape, int p_index);
Geometry::p_index_set get_ext_bordering_precincts(Geometry::Precinct_Group precincts, Geometry::State state);
Geometry::p_index_set get_ext_bordering_precincts(Geometry::Precinct_Group precincts, Geometry::p_index_set available_pre, Geometry::State state);

// overload get_bordering_shapes for vector inheritance problem
Geometry::p_index_set get_bordering_shapes(std::vector<Geometry::Polygon> shapes, Geometry::Polygon shape);
Geometry::p_index_set get_bordering_shapes(std::vector<Geometry::Precinct_Group> shapes, Geometry::Polygon shape);
Geometry::p_index_set get_bordering_shapes(std::vector<Geometry::Community> shapes, Geometry::Community shape);
Geometry::p_index_set get_bordering_shapes(std::vector<Geometry::Community> shapes, Geometry::Polygon shape);

bool point_in_ring(Geometry::coordinate coord, Geometry::LinearRing lr);
bool get_inside(Geometry::LinearRing s0, Geometry::LinearRing s1);
bool get_inside_first(Geometry::LinearRing s0, Geometry::LinearRing s1);

Geometry::Polygon generate_gon(Geometry::coordinate c, double radius, int n);
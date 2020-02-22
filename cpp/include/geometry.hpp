/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Wed, Feb 19
 
 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#pragma once // avoid multiple includes

#include "shape.hpp"
#include <math.h>  // for rounding functions
#include <cmath>
#include <algorithm> // for reverse, unique
#include <iostream>
#include <string>

#include "../lib/clipper/clipper.hpp"

// coordinate manipulation for gui draw methods
GeoGerry::bounding_box normalize_coordinates(GeoGerry::Shape* shape);
GeoGerry::coordinate_set resize_coordinates(GeoGerry::bounding_box box, GeoGerry::coordinate_set shape, int screenX, int screenY);

// get outside border from a group of precincts
GeoGerry::Multi_Shape generate_exterior_border(GeoGerry::Precinct_Group p);

// get precincts on the inside border of a precinct group
GeoGerry::p_index_set get_inner_boundary_precincts(GeoGerry::Precinct_Group shape);
GeoGerry::p_index_set get_bordering_precincts(GeoGerry::Precinct_Group shape, int p_index);

// overload get_bordering_shapes for vector inheritance problem
GeoGerry::p_index_set get_bordering_shapes(std::vector<GeoGerry::Shape> shapes, GeoGerry::Shape shape);
GeoGerry::p_index_set get_bordering_shapes(std::vector<GeoGerry::Precinct_Group> shapes, GeoGerry::Shape shape);

// implementation of the Schwartzberg compactness score
GeoGerry::unit_interval compactness(GeoGerry::Shape shape);

// for clipper conversions
ClipperLib::Path ring_to_path(GeoGerry::LinearRing ring);
GeoGerry::LinearRing path_to_ring(ClipperLib::Path path);
ClipperLib::Paths shape_to_paths(GeoGerry::Shape shape);
GeoGerry::Shape paths_to_shape(ClipperLib::Paths paths);
GeoGerry::Multi_Shape paths_to_multi_shape(ClipperLib::Paths paths);

// for algorithm helper methods
double get_standard_deviation_partisanship(GeoGerry::Precinct_Group pg);
double get_median_partisanship(GeoGerry::Precinct_Group pg);

bool point_in_ring(GeoGerry::coordinate coord, GeoGerry::LinearRing lr);
bool get_inside(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1);
bool get_inside_first(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1);
bool get_inside_d(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1);
/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Mon, Feb 10
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

GeoGerry::unit_interval compactness(GeoGerry::Shape shape);

ClipperLib::Path shape_to_path(GeoGerry::Shape shape);
GeoGerry::Shape poly_to_shape(ClipperLib::Path path);
ClipperLib::Paths multi_shape_to_paths(GeoGerry::Multi_Shape ms);
GeoGerry::Multi_Shape paths_to_multi_shape(ClipperLib::Paths paths);

// for algorithm helper methods
double get_standard_deviation_partisanship(GeoGerry::Precinct_Group pg);
double get_median_partisanship(GeoGerry::Precinct_Group pg);
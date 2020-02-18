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

// coordinate manipulation for gui draw methods
bounding_box normalize_coordinates(Shape* shape);
coordinate_set resize_coordinates(bounding_box box, coordinate_set shape, int screenX, int screenY);

// get outside border from a group of precincts
Multi_Shape generate_exterior_border(Precinct_Group p);

// get precincts on the inside border of a precinct group
p_index_set get_inner_boundary_precincts(Precinct_Group shape);
p_index_set get_bordering_precincts(Precinct_Group shape, int p_index);

// overload get_bordering_shapes for vector inheritance problem
p_index_set get_bordering_shapes(vector<Shape> shapes, Shape shape);
p_index_set get_bordering_shapes(vector<Precinct_Group> shapes, Shape shape);

unit_interval compactness(Shape shape);

Geometry* shape_to_poly(Shape shape);
Shape poly_to_shape(const Geometry* path);
MultiPolygon* multi_shape_to_poly(Multi_Shape ms);
Multi_Shape multi_poly_to_shape(MultiPolygon* paths);

// for algorithm helper methods
float get_standard_deviation_partisanship(Precinct_Group pg);
float get_median_partisanship(Precinct_Group pg);
/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Mon, Feb 10

 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#include <math.h>  // for rounding functions
#include <cmath>
#include "../lib/clipper/clipper.hpp"
#include <algorithm> // for reverse, unique
#include <iostream>
#include <string>
#include "shape.hpp"

using namespace std;
using namespace ClipperLib;

typedef vector<DoublePoint> DoublePath;
typedef vector<DoublePath> DoublePaths;

// coordinate manipulation for gui draw methods
bounding_box normalize_coordinates(Shape* shape);
coordinate_set resize_coordinates(bounding_box box, coordinate_set shape, int screenX, int screenY);

// get outside border from a group of precincts
Multi_Shape generate_exterior_border(Precinct_Group p);

// get precincts on the inside border of a precinct group
p_index_set get_inner_boundary_precincts(Precinct_Group shape);
p_index_set get_bordering_precincts(Precinct_Group shape, int p_index);
unit_interval compactness(Shape shape);

Path shape_to_clipper_int(Shape shape);
Shape clipper_int_to_shape(Path path);
Multi_Shape clipper_mult_int_to_shape(Paths paths);

// for algorithm helper methods
float get_standard_deviation_partisanship(Precinct_Group pg);
float get_median_partisanship(Precinct_Group pg);
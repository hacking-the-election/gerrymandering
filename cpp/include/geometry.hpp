/*=======================================
 geometry.hpp                   k-vernooy
 last modified:                Sun, Feb 9

 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#include "shape.hpp"
using namespace std;

// coordinate manipulation for gui draw methods
bounding_box normalize_coordinates(Shape* shape);
coordinate_set resize_coordinates(bounding_box box, coordinate_set shape, int screenX, int screenY);

Shape generate_exterior_border(Precinct_Group p);
p_index_set get_inner_boundary_precincts(Precinct_Group shape);
p_index_set get_bordering_precincts(Precinct_Group shape, int p_index);
unit_interval compactness(Shape shape);
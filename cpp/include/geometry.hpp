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

coordinate_set generate_exterior_border(Precinct_Group test);
index_set get_boundary_precincts(Precinct_Group shape);
index_set get_bordering_precincts(Precinct_Group shape, int index);
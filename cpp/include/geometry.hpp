/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Mon, Feb 10

 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <math.h>  // for rounding functions
#include <cmath>

#include "shape.hpp"
using namespace std;
#include <algorithm> // for reverse, unique
#include <iostream>
#include <string>

#include <boost/assign/std/vector.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/io/dsv/write.hpp>

using namespace boost::geometry;
using namespace boost::assign;

typedef model::d2::point_xy<double> boost_point;
typedef model::polygon<boost_point> boost_polygon;
typedef model::box<boost_point> boost_box;

// coordinate manipulation for gui draw methods
bounding_box normalize_coordinates(Shape* shape);
coordinate_set resize_coordinates(bounding_box box, coordinate_set shape, int screenX, int screenY);

// get outside border from a group of precincts
Shape generate_exterior_border(Precinct_Group p);

// get precincts on the inside border of a precinct group
p_index_set get_inner_boundary_precincts(Precinct_Group shape);
p_index_set get_bordering_precincts(Precinct_Group shape, int p_index);
unit_interval compactness(Shape shape);
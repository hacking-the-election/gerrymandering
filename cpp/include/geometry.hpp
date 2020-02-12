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

// #include <boost/assign/std/vector.hpp>
// #include <boost/geometry.hpp>
// #include <boost/geometry/algorithms/area.hpp>
// #include <boost/geometry/algorithms/assign.hpp>
// #include <boost/geometry/geometries/point_xy.hpp>
// #include <boost/geometry/geometries/polygon.hpp>
// #include <boost/geometry/geometries/multi_polygon.hpp>
// #include <boost/geometry/io/dsv/write.hpp>
// #include <boost/geometry/geometries/adapted/c_array.hpp>

#ifndef BOOST_POLY_TYPES_HH
#define BOOST_POLY_TYPES_HH
#endif

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>
#include <boost/geometry/geometries/register/point.hpp>


using namespace boost::geometry;
// using namespace boost::assign;
BOOST_GEOMETRY_REGISTER_POINT_2D(legacy_point, double, cs::cartesian, x, y)

typedef model::d2::point_xy<double> boost_point;
typedef model::polygon<boost_point> boost_polygon;
typedef model::multi_polygon<boost_polygon> boost_multi_polygon;
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

// for printing shape in a "x1,y1 x2,y2" format
void print_shape(boost_polygon poly);
void print_shape(Shape shape);

// for converting to and from boost objects + types
boost_polygon shape_to_poly(Shape shape);
Shape boost_poly_to_shape(boost_polygon poly);
Shape boost_poly_to_shape(boost_multi_polygon poly);
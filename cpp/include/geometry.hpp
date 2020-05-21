/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Fri, Feb 28
 
 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#pragma once

#include "shape.hpp"
#include "community.hpp"
#include "../lib/Clipper/cpp/clipper.hpp"
#include "../lib/Miniball.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

typedef boost::geometry::model::d2::point_xy<long long int> boost_point;
typedef boost::geometry::model::polygon<boost_point> boost_polygon;
using namespace boost::geometry;

#define PI 3.14159265358979323846264338327950288
enum clip_type {UNION, INTERSECTION, DIFFERENCE, XOR};

// typedefs for using miniball (which is not very fast)
typedef std::vector<std::vector<double> >::const_iterator PointIterator; 
typedef std::vector<double>::const_iterator CoordIterator;
typedef Miniball::Miniball <Miniball::CoordAccessor<PointIterator, CoordIterator> > MB;
boost_polygon ring_to_boost_poly(hte::Geometry::LinearRing);

// segment and coordiante manipulation
hte::Geometry::segment coords_to_seg(hte::Geometry::coordinate c1, hte::Geometry::coordinate c2);
double get_distance(hte::Geometry::coordinate c1, hte::Geometry::coordinate c2);
double get_distance(std::array<long long int, 2> c1, std::array<long long int, 2> c2);
double get_distance(hte::Geometry::segment s);

// two shapes have adjacent and colinear segments
bool get_bordering(hte::Geometry::Polygon s0, hte::Geometry::Polygon s1);

// for clipper conversions
ClipperLib::Path ring_to_path(hte::Geometry::LinearRing ring);
hte::Geometry::LinearRing path_to_ring(ClipperLib::Path path);
ClipperLib::Paths shape_to_paths(hte::Geometry::Polygon shape);
hte::Geometry::Multi_Polygon paths_to_multi_shape(ClipperLib::Paths paths);

// get outside border from a group of precincts
hte::Geometry::Multi_Polygon generate_exterior_border(hte::Geometry::Precinct_Group p);
hte::Geometry::Polygon generate_gon(hte::Geometry::coordinate c, double radius, int n);

// testing bounds and overlaps
bool bound_overlap(hte::Geometry::bounding_box b1, hte::Geometry::bounding_box b2);
bool bound_inside(hte::Geometry::bounding_box b1, hte::Geometry::bounding_box b2);
bool point_in_ring(hte::Geometry::coordinate coord, hte::Geometry::LinearRing lr);
bool get_inside(hte::Geometry::LinearRing s0, hte::Geometry::LinearRing s1);
bool get_inside_first(hte::Geometry::LinearRing s0, hte::Geometry::LinearRing s1);
bool point_in_circle(hte::Geometry::coordinate center, double radius, hte::Geometry::coordinate point);

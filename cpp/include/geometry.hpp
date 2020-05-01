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
#include "../lib/Miniball.hpp"

#define PI 3.14159265358979323846264338327950288

// typedefs for using miniball (which is not very fast)
typedef std::vector<std::vector<double> >::const_iterator PointIterator; 
typedef std::vector<double>::const_iterator CoordIterator;
typedef Miniball::Miniball <Miniball::CoordAccessor<PointIterator, CoordIterator> > MB;

// segment and coordiante manipulation
Gerrymandering::Geometry::segment coords_to_seg(Gerrymandering::Geometry::coordinate c1, Gerrymandering::Geometry::coordinate c2);
double get_distance(Gerrymandering::Geometry::coordinate c1, Gerrymandering::Geometry::coordinate c2);
double get_distance(std::array<long long int, 2> c1, std::array<long long int, 2> c2);
double get_distance(Gerrymandering::Geometry::segment s);

// two shapes have adjacent and colinear segments
bool get_bordering(Gerrymandering::Geometry::Polygon s0, Gerrymandering::Geometry::Polygon s1);

// for clipper conversions
ClipperLib::Path ring_to_path(Gerrymandering::Geometry::LinearRing ring);
Gerrymandering::Geometry::LinearRing path_to_ring(ClipperLib::Path path);
ClipperLib::Paths shape_to_paths(Gerrymandering::Geometry::Polygon shape);
Gerrymandering::Geometry::Multi_Polygon paths_to_multi_shape(ClipperLib::Paths paths);

// get outside border from a group of precincts
Gerrymandering::Geometry::Multi_Polygon generate_exterior_border(Gerrymandering::Geometry::Precinct_Group p);
Gerrymandering::Geometry::Polygon generate_gon(Gerrymandering::Geometry::coordinate c, double radius, int n);

// testing bounds and overlaps
bool bound_overlap(Gerrymandering::Geometry::bounding_box b1, Gerrymandering::Geometry::bounding_box b2);
bool point_in_ring(Gerrymandering::Geometry::coordinate coord, Gerrymandering::Geometry::LinearRing lr);
bool get_inside(Gerrymandering::Geometry::LinearRing s0, Gerrymandering::Geometry::LinearRing s1);
bool get_inside_first(Gerrymandering::Geometry::LinearRing s0, Gerrymandering::Geometry::LinearRing s1);
bool point_in_circle(Gerrymandering::Geometry::coordinate center, double radius, Gerrymandering::Geometry::coordinate point);
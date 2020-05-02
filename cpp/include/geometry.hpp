/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Fri, Feb 28
 
 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#pragma once
#define PI 3.14159265358979323846264338327950288

#include "shape.hpp"
#include "community.hpp"
#include "../lib/clipper/clipper.hpp"
#include "../lib/Miniball.hpp"


typedef std::vector<std::vector<double> >::const_iterator PointIterator; 
typedef std::vector<double>::const_iterator CoordIterator;
typedef Miniball::Miniball <Miniball::CoordAccessor<PointIterator, CoordIterator> > MB;


// segment and coordiante manipulation
Geometry::segment coords_to_seg(Geometry::coordinate c1, Geometry::coordinate c2);
double get_distance(Geometry::coordinate c1, Geometry::coordinate c2);
double get_distance(std::array<long long int, 2> c1, std::array<long long int, 2> c2);
double get_distance(Geometry::segment s);

// two shapes have adjacent and colinear segments
bool get_bordering(Geometry::Polygon s0, Geometry::Polygon s1);

// for clipper conversions
ClipperLib::Path ring_to_path(Geometry::LinearRing ring);
Geometry::LinearRing path_to_ring(ClipperLib::Path path);
ClipperLib::Paths shape_to_paths(Geometry::Polygon shape);
Geometry::Multi_Polygon paths_to_multi_shape(ClipperLib::Paths paths);

// get outside border from a group of precincts
Geometry::Multi_Polygon generate_exterior_border(Geometry::Precinct_Group p);
Geometry::Polygon generate_gon(Geometry::coordinate c, double radius, int n);

// testing bounds and overlaps
bool bound_overlap(Geometry::bounding_box b1, Geometry::bounding_box b2);
bool point_in_ring(Geometry::coordinate coord, Geometry::LinearRing lr);
bool get_inside(Geometry::LinearRing s0, Geometry::LinearRing s1);
bool get_inside_first(Geometry::LinearRing s0, Geometry::LinearRing s1);
bool point_in_circle(Geometry::coordinate center, double radius, Geometry::coordinate point);
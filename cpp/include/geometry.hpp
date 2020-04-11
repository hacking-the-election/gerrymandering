/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Fri, Feb 28
 
 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

typedef boost::geometry::model::d2::point_xy<long int> boost_point;
typedef boost::geometry::model::polygon<boost_point> boost_polygon;

#include "shape.hpp"
#include "../lib/clipper/clipper.hpp"

using namespace boost::geometry;
#define PI 3.14159265358979323846264338327950288

enum clip_type {UNION, INTERSECTION, DIFFERENCE, XOR};


boost_polygon ring_to_boost_poly(Geometry::LinearRing shape);
Geometry::segment coords_to_seg(Geometry::coordinate c1, Geometry::coordinate c2);

double get_distance(Geometry::coordinate c1, Geometry::coordinate c2);
double get_distance(std::array<long long int, 2> c1, std::array<long long int, 2> c2);
double get_distance(Geometry::segment s);

// get outside border from a group of precincts
Geometry::Multi_Polygon generate_exterior_border(Geometry::Precinct_Group p);

// get precincts on the inside border of a precinct group
Geometry::p_index_set get_inner_boundary_precincts(Geometry::Precinct_Group shape);
Geometry::p_index_set get_inner_boundary_precincts(Geometry::p_index_set precincts, Geometry::State state);

Geometry::p_index_set get_bordering_precincts(Geometry::Precinct_Group shape, int p_index);
Geometry::p_index_set get_ext_bordering_precincts(Geometry::Precinct_Group precincts, Geometry::State state);
Geometry::p_index_set get_ext_bordering_precincts(Geometry::Precinct_Group precincts, Geometry::p_index_set available_pre, Geometry::State state);

bool get_bordering(Geometry::Polygon s0, Geometry::Polygon s1);

// overload get_bordering_shapes for vector inheritance problem
Geometry::p_index_set get_bordering_shapes(std::vector<Geometry::Polygon> shapes, Geometry::Polygon shape);
Geometry::p_index_set get_bordering_shapes(std::vector<Geometry::Precinct_Group> shapes, Geometry::Polygon shape);
Geometry::p_index_set get_bordering_shapes(std::vector<Geometry::Community> shapes, Geometry::Community shape);
Geometry::p_index_set get_bordering_shapes(std::vector<Geometry::Community> shapes, Geometry::Polygon shape);

// for clipper conversions
ClipperLib::Path ring_to_path(Geometry::LinearRing ring);
Geometry::LinearRing path_to_ring(ClipperLib::Path path);
ClipperLib::Paths shape_to_paths(Geometry::Polygon shape);
Geometry::Multi_Polygon paths_to_multi_shape(ClipperLib::Paths paths);

Geometry::p_index_set get_giveable_precincts(Geometry::Community c, Geometry::Communities cs);
std::vector<std::array<int, 2>> get_takeable_precincts(Geometry::Community c, Geometry::Communities cs);

// geos::geom::LinearRing* create_linearring(Geometry::coordinate_set coords);
// geos::geom::Point* create_point(double x, double y);
// geos::geom::Geometry* shape_to_poly(Geometry::LinearRing shape);
// Geometry::Polygon poly_to_shape(const geos::geom::Geometry* path);
// Geometry::Multi_Polygon* multipoly_to_shape(geos::geom::MultiPolygon* paths);
// geos::geom::Geometry::NonConstVect multi_shape_to_poly(Geometry::Multi_Polygon ms);

// for algorithm helper methods
double get_standard_deviation_partisanship(Geometry::Precinct_Group pg);
double get_standard_deviation_partisanship(Geometry::Communities cs);

double get_median_partisanship(Geometry::Precinct_Group pg);

bool point_in_ring(Geometry::coordinate coord, Geometry::LinearRing lr);
bool get_inside(Geometry::LinearRing s0, Geometry::LinearRing s1);
bool get_inside_first(Geometry::LinearRing s0, Geometry::LinearRing s1);

bool creates_island(Geometry::Precinct_Group set, Geometry::p_index remove);
bool creates_island(Geometry::p_index_set set, Geometry::p_index remove, Geometry::State precincts);
bool creates_island(Geometry::Precinct_Group set, Geometry::Precinct precinct);

Geometry::p_index get_first_precinct(Geometry::Precinct_Group available_precincts, Geometry::Communities communities);
Geometry::Polygon generate_gon(Geometry::coordinate c, double radius, int n);
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


boost_polygon ring_to_boost_poly(GeoGerry::LinearRing shape);
GeoGerry::segment coords_to_seg(GeoGerry::coordinate c1, GeoGerry::coordinate c2);

double get_distance(GeoGerry::coordinate c1, GeoGerry::coordinate c2);
double get_distance(std::array<long long int, 2> c1, std::array<long long int, 2> c2);
double get_distance(GeoGerry::segment s);

// get outside border from a group of precincts
GeoGerry::Multi_Polygon generate_exterior_border(GeoGerry::Precinct_Group p);

// get precincts on the inside border of a precinct group
GeoGerry::p_index_set get_inner_boundary_precincts(GeoGerry::Precinct_Group shape);
GeoGerry::p_index_set get_inner_boundary_precincts(GeoGerry::p_index_set precincts, GeoGerry::State state);

GeoGerry::p_index_set get_bordering_precincts(GeoGerry::Precinct_Group shape, int p_index);
GeoGerry::p_index_set get_ext_bordering_precincts(GeoGerry::Precinct_Group precincts, GeoGerry::State state);
GeoGerry::p_index_set get_ext_bordering_precincts(GeoGerry::Precinct_Group precincts, GeoGerry::p_index_set available_pre, GeoGerry::State state);

bool get_bordering(GeoGerry::Polygon s0, GeoGerry::Polygon s1);

// overload get_bordering_shapes for vector inheritance problem
GeoGerry::p_index_set get_bordering_shapes(std::vector<GeoGerry::Polygon> shapes, GeoGerry::Polygon shape);
GeoGerry::p_index_set get_bordering_shapes(std::vector<GeoGerry::Precinct_Group> shapes, GeoGerry::Polygon shape);
GeoGerry::p_index_set get_bordering_shapes(std::vector<GeoGerry::Community> shapes, GeoGerry::Community shape);
GeoGerry::p_index_set get_bordering_shapes(std::vector<GeoGerry::Community> shapes, GeoGerry::Polygon shape);

// for clipper conversions
ClipperLib::Path ring_to_path(GeoGerry::LinearRing ring);
GeoGerry::LinearRing path_to_ring(ClipperLib::Path path);
ClipperLib::Paths shape_to_paths(GeoGerry::Polygon shape);
GeoGerry::Multi_Polygon paths_to_multi_shape(ClipperLib::Paths paths);

GeoGerry::p_index_set get_giveable_precincts(GeoGerry::Community c, GeoGerry::Communities cs);
std::vector<std::array<int, 2>> get_takeable_precincts(GeoGerry::Community c, GeoGerry::Communities cs);

// geos::geom::LinearRing* create_linearring(GeoGerry::coordinate_set coords);
// geos::geom::Point* create_point(double x, double y);
// geos::geom::Geometry* shape_to_poly(GeoGerry::LinearRing shape);
// GeoGerry::Polygon poly_to_shape(const geos::geom::Geometry* path);
// GeoGerry::Multi_Polygon* multipoly_to_shape(geos::geom::MultiPolygon* paths);
// geos::geom::Geometry::NonConstVect multi_shape_to_poly(GeoGerry::Multi_Polygon ms);

// for algorithm helper methods
double get_standard_deviation_partisanship(GeoGerry::Precinct_Group pg);
double get_standard_deviation_partisanship(GeoGerry::Communities cs);

double get_median_partisanship(GeoGerry::Precinct_Group pg);

bool point_in_ring(GeoGerry::coordinate coord, GeoGerry::LinearRing lr);
bool get_inside(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1);
bool get_inside_first(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1);

bool creates_island(GeoGerry::Precinct_Group set, GeoGerry::p_index remove);
bool creates_island(GeoGerry::p_index_set set, GeoGerry::p_index remove, GeoGerry::State precincts);
bool creates_island(GeoGerry::Precinct_Group set, GeoGerry::Precinct precinct);

GeoGerry::p_index get_first_precinct(GeoGerry::Precinct_Group available_precincts, GeoGerry::Communities communities);
GeoGerry::Polygon generate_gon(GeoGerry::coordinate c, double radius, int n);
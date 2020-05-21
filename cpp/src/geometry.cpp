/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:              Fri, Feb 28
 
 Definition of useful functions for
 computational geometry. Basic 
 calculation, area, bordering - no
 algorithmic specific methods.
========================================*/

#include <iostream>
#include <chrono>
#include <random>
#include <math.h>

#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/shape.hpp"   // class definitions
#include "../include/util.hpp"

// define geometric constants
const long int c = pow(10, 18);
const long int d = pow(10, 9);
const long int l = pow(2, 6);

using namespace hte;
using namespace Geometry;
using namespace std;

/*
    Following methods utilize the segment of segments typedefs.
    These are useful for summing distances, or checking colinearity,
    which is used for below border functions.
*/

double round_d(double value) {
    return round(value * d) / d;
}


segment coords_to_seg(coordinate c1, coordinate c2) {
    /*
        @desc: combines coordinates into a segment array
        @params: `c1`, `c2`: coordinates 1 and 2 in segment
        @return: `Geometry::segment` a segment with the coordinates provided
    */

    segment s = {{ c1[0], c1[1], c2[0], c2[1] }};
    return s;
}


double get_distance(segment s) {
    /* 
        @desc: Distance formula on a segment array
        @params: `s`: a segment to get the distance of
        @return: `double` the distance of the segment
    */
   
    return sqrt(pow((s[2] - s[0]), 2) + pow((s[3] - s[1]), 2));
}


double get_distance(std::array<long long int, 2> c1, std::array<long long int, 2> c2) {
    /* 
        @desc: Distance formula on a segment array
        @params: `c1`, `c2`: coordinates to get the distance of
        @return: `double` the distance of the segment
    */

    return sqrt(pow((c2[0] - c1[0]), 2) + pow((c2[1] - c1[1]), 2));
}


double get_distance(coordinate c0, coordinate c1) {
    /*
        @desc: Distance formula on two separate points
        @params: `c1`, `c2`: coordinates 1 and 2 in segment
        @return: `double` the distance between the coordinates
    */

    return get_distance(coords_to_seg(c0, c1));
}


vector<long int> get_equation(segment s) {
    /*
        @desc:
            Use slope/intercept form and substituting coordinates
            in order to determine equation of a line segment

        @params: `s`: the segment to calculate
        @return: `vector` slope and intercept
        @warn: Need handlers for div by 0
    */

    long int dy = s[3] - s[1];
    long int dx = s[2] - s[0];

    long int m;
    
    if (dx != 0) m = (dy * l) / dx;
    else m = l;

    long int b = -1 * ((m * s[0]) - s[1]); // @warn - multiply by -1 for value

    return {m, b};
}


bool get_colinear(segment s0, segment s1) {
    /*
        @desc: gets whether or not two lines have the same equation
        @params: `s0`, `s1`: two segments to check colinearity
        @return: `bool` are colinear
    */

    return (get_equation(s0) == get_equation(s1));
}


bool point_on_segment(coordinate c, segment s) {
    /*
        @desc: checks if a coordinate lines within a line segment
        @params:
            `coordinate` c: coordinate to check
            `segment` s: segment to contain point

        @return: `bool` segment contains line
    */

    if ((c[0] == s[0] && c[1] == s[1]) || (c[0] == s[2] && c[1] == s[3]))
        // point on endpoints of segment
        return true;

    int crossproduct = (c[1] - s[1]) * (s[2] - s[0]) - (c[0] - s[0]) * (s[3] - s[1]);
    if (abs(crossproduct) != 0)
        return false;

    int dotproduct = (c[0] - s[0]) * (s[2] - s[0]) + (c[1] - s[1]) * (s[3] - s[1]);
    if (dotproduct < 0)
        return false;

    int squaredlengthba = (s[2] - s[0]) * (s[2] - s[0]) + (s[3] - s[1]) * (s[3] - s[1]);

    if (dotproduct > squaredlengthba)
        return false;

    return true;
}


bool get_overlap(segment s0, segment s1) {
    /*
        @desc:
            Returns whether or not two segments' bounding boxes
            overlap, meaning one of the extremes of a segment are
            within the range of the other's. One shared point does
            not count to overlap.
        
        @params: `s0`, `s1`: two segments to check overlap
        @return: `bool` segments overlap
        @warn: This function is untested!
    */
    
    long int s0min = s0[0];
    long int s0max = s0[2];

    if (s0[2] < s0min) {
        s0min = s0[2];
        s0max = s0[0];
    }

    long int s1min = s1[0];
    long int s1max = s1[2];

    if (s1[2] < s1min) {
        s1min = s1[2];
        s1max = s1[0];
    }

    return (s0min <= s1max && s1min <= s0max);
}


segments Geometry::LinearRing::get_segments() {
    /*
        @desc:
            returns a vector of segments from the
            coordinate array of a LinearRing.border property

        @params: none
        @return: segments of a ring
    */

    segments segs;
    vector<coordinate> border_x = border;

    for (int i = 0; i < border_x.size(); i++) {
        coordinate c1 = border_x[i];  // starting coord
        coordinate c2;                // ending coord

        // find position of ending coordinate
        if (i == border_x.size() - 1) c2 = border_x[0];
        else c2 = border_x[i + 1];

        if (c1 != c2) segs.push_back(coords_to_seg(c1, c2)); // add to list
    }

    return segs;
}


segments Geometry::Polygon::get_segments() {
    /*
        @desc: get list of all segments in a shape, including hole LinearRings array
        @params: none
        @return: segments of a shape
    */

    segments segs = this->hull.get_segments();
    
    for (LinearRing hole : this->holes)
        for (segment seg : hole.get_segments())
            segs.push_back(seg);

    return segs;
}


segments Geometry::Multi_Polygon::get_segments() {
    /*
        @desc: get a list of all segments in a multi_shape, for each shape, including holes
        @params: none
        @return: segments array of each shape
    */

    segments segs;
    for (Polygon s : this->border)
        for (segment seg : s.get_segments())
            segs.push_back(seg);

    return segs;
}


coordinate Geometry::LinearRing::get_centroid() {
    /* 
        @desc: Gets the centroid of a polygon with coords
        @ref: https://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
        @params: none
        @return: coordinate of centroid
    */

    // if (centroid[0] == NULL) {
    //     long int Cx = 0, Cy = 0;

    //     if (border[0] != border[border.size() - 1])
    //         border.push_back(border[0]);

    //     for (int i = 0; i < border.size() - 1; i++) {
    //         long int x1 = border[i][0];
    //         long int y1 = border[i][1];
    //         long int x2 = border[i + 1][0];
    //         long int y2 = border[i + 1][1];

    //         Cx += (x1 + x2) * ((x1 * y2) - (x2 * y1));
    //         Cy += (y1 + y2) * ((x1 * y2) - (x2 * y1));
    //     }

    //     centroid[0] = (long int) round(1.0 / (6.0 * this->get_area()) * (double) Cx);
    //     centroid[1] = (long int) round(1.0 / (6.0 * this->get_area()) * (double) Cy);
    // }

    return centroid;
}


boost_polygon ring_to_boost_poly(LinearRing shape) {
    /*
        Converts a shape object into a boost polygon object
        by looping over each point and manually adding it to a 
        boost polygon using assign_points and vectors
    */

    boost_polygon poly;
    // create vector of boost points
    std::vector<boost_point> points;
    for (coordinate c : shape.border) 
        points.push_back(boost_point(c[0],c[1])),

    assign_points(poly, points);
    return poly;
}


double Geometry::LinearRing::get_area() {
    /*
        @desc:
            returns the area of a linear ring, using latitude * long
            area - an implementation of the shoelace theorem

        @params: none
        @ref: https://www.mathopenref.com/coordpolygonarea.html
        @return: area of linear ring as a double
    */

    double area = 0;

    for ( int i = 0; i < border.size(); i++ ) {
        int j;
        if (i == border.size() - 1) j = 0;
        else j = i + 1;

        area += (border[i][0] * border[j][1]) - (border[i][1] * border[j][0]);
    }

    return area / 2.0;
}


double Geometry::LinearRing::get_perimeter() {
    /*
        @desc: returns the perimeter of a LinearRing object by summing distance
        @params: none
        @return: `double` perimeter
    */

    double t = 0;
    for (segment s : get_segments())
        t += get_distance(s);    

    return t;
}


coordinate Geometry::Polygon::get_centroid() {
    /*
        @desc:
            returns average centroid from list of `holes`
            and `hull` by calling `LinearRing::get_centroid`

        @params: none
        @return: `coordinate` average centroid of shape
    */

    return (hull.get_centroid());
}


coordinate Geometry::Multi_Polygon::get_centroid() {
    /*
        @desc:
            returns average centroid from list of `holes`
            and `hull` by calling `LinearRing::get_centroid`

        @params: none
        @return: `coordinate` average centroid of shape
    */

    coordinate average = {0,0};

    for (Polygon s : border) {
        coordinate center = s.hull.get_centroid();

        for (Geometry::LinearRing lr : s.holes) {
            coordinate nc = lr.get_centroid();
            center[0] += nc[0];
            center[1] += nc[1];
        }

        int size = 1 + s.holes.size();
        average[0] += center[0] / size;
        average[1] += center[1] / size;
    }

    return {(int)(average[0] / border.size()), (int)(average[1] / border.size())};
}


coordinate Geometry::Precinct_Group::get_centroid() {
    /*
        @desc:
            returns average centroid from list of `holes`
            and `hull` by calling `LinearRing::get_centroid`

        @params: none
        @return: `coordinate` average centroid of shape
    */

    coordinate average = {0,0};

    for (Precinct s : precincts) {
        coordinate center = s.hull.get_centroid();
        average[0] += center[0];
        average[1] += center[1];
    }

    return {((long int)average[0] / (long int)precincts.size()), ((long int)average[1] / (long int)precincts.size())};
}


double Geometry::Polygon::get_area() {
    /*
        @desc:
            gets the area of the hull of a shape
            minus the combined area of any holes

        @params: none
        @return: `double` totale area of shape
    */

    double area = hull.get_area();
    for (Geometry::LinearRing h : holes)
        area -= h.get_area();

    return area;
}


double Geometry::Precinct_Group::get_area() {
    double sum = 0;
    
    for (Precinct p : precincts)
        sum += abs(p.get_area());
    return sum;
}


double Geometry::Polygon::get_perimeter() {
    /*
        @desc:
            gets the sum perimeter of all LinearRings
            in a shape object, including holes

        @params: none
        @return: `double` total perimeter of shape
    */

    double perimeter = hull.get_perimeter();
    for (Geometry::LinearRing h : holes)
        perimeter += h.get_perimeter();

    return perimeter;
}


double Multi_Polygon::get_area() {
    /*
        @desc: gets sum area of all Polygon objects in border
        @params: none
        @return: `double` total area of shapes
    */

    double total = 0;
    for (Polygon s : border)
        total += s.get_area();

    return total;
}


double Geometry::Multi_Polygon::get_perimeter() {
    /*
        @desc:
            gets sum perimeter of a multi shape object
            by looping through each shape and calling method
            
        @params: none
        @return `double` total perimeter of shapes array
    */

    double p = 0;
    for (Polygon shape : border)
        p += shape.get_perimeter();

    return p;
}


bool get_bordering(Polygon s0, Polygon s1) {
    /*
        @desc: gets whether or not two shapes touch each other
        @params: `Polygon` s0, `Polygon` s1: shapes to check bordering
        @return: `bool` shapes are boording
    */
    
    // create paths array from polygon
	ClipperLib::Paths subj;
    subj.push_back(ring_to_path(s0.hull));

    ClipperLib::Paths clip;
    clip.push_back(ring_to_path(s1.hull));

    ClipperLib::Paths solutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

    Multi_Polygon ms = paths_to_multi_shape(solutions);
    return (ms.border.size() == 1);
}


bool get_bordering(Multi_Polygon s0, Polygon s1) {
    /*
        @desc: gets whether or not two shapes touch each other
        @params: `Multi_Polygon` s0, `Polygon` s1: shapes to check bordering
        @return: `bool` shapes are boording
    */

    // create paths array from polygon
	ClipperLib::Paths subj;
    for (Polygon s : s0.border)
        subj.push_back(ring_to_path(s.hull));

    ClipperLib::Paths clip;
    clip.push_back(ring_to_path(s1.hull));

    ClipperLib::Paths usolutions;
    ClipperLib::Paths xsolutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctXor, xsolutions, ClipperLib::pftNonZero);
    c.Execute(ClipperLib::ctUnion, usolutions, ClipperLib::pftNonZero);

    Multi_Polygon msx = paths_to_multi_shape(xsolutions);
    Multi_Polygon msu = paths_to_multi_shape(usolutions);

    return (msu.border.size() == s0.border.size() && msx.holes.size() == 0);
}


bool get_bordering(Multi_Polygon s0, Multi_Polygon s1) {
    /*
        @desc: gets whether or not two shapes touch each other
        @params: `Multi_Polygon` s0, `Multi_Polygon` s1: shapes to check bordering
        @return: `bool` shapes are boording
    */

    // create paths array from polygon
	ClipperLib::Paths subj;
    for (Polygon s : s0.border)
        subj.push_back(ring_to_path(s.hull));

    ClipperLib::Paths clip;
    for (Polygon s : s1.border)
        clip.push_back(ring_to_path(s.hull));

    ClipperLib::Paths usolutions;
    ClipperLib::Paths xsolutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctXor, xsolutions, ClipperLib::pftNonZero);
    c.Execute(ClipperLib::ctUnion, usolutions, ClipperLib::pftNonZero);

    Multi_Polygon msx = paths_to_multi_shape(xsolutions);
    Multi_Polygon msu = paths_to_multi_shape(usolutions);

    return (msu.border.size() < s0.border.size() + s1.border.size() && msx.holes.size() <= s0.holes.size() + s1.holes.size());
}


bool point_in_ring(Geometry::coordinate coord, Geometry::LinearRing lr) {
    /*
        @desc:
            gets whether or not a point is in a ring using
            the ray intersection method (clipper implementation)

        @ref: http://www.angusj.com/delphi/Clipper/documentation/Docs/Units/ClipperLib/Functions/PointInPolygon.htm
        @params: 
            `coordinate` coord: the point to check
            `LinearRing` lr: the shape to check the point against
        
        @return: `bool` point is in/on polygon
    */

    // close ring for PIP problem
    if (lr.border[0] != lr.border[lr.border.size() - 1])
        lr.border.push_back(lr.border[0]);

    // convert to clipper types for builtin function
    ClipperLib::Path path = ring_to_path(lr);
    ClipperLib::IntPoint p(coord[0], coord[1]);
    return (!(ClipperLib::PointInPolygon(p, path) == 0));
}


bool get_inside(Geometry::LinearRing s0, Geometry::LinearRing s1) {
    /*
        @desc:
            gets whether or not s0 is inside of 
            s1 using the intersection point method

        @params:
            `LinearRing` s0: ring inside `s1`
            `LinearRing` s1: ring containing `s0`

        @return: `bool` `s0` inside `s1`
    */

    for (coordinate c : s0.border)
        if (!point_in_ring(c, s1)) return false;

    return true;
}


bool get_inside_first(Geometry::LinearRing s0, Geometry::LinearRing s1) {
    /*
        @desc:
            gets whether or not the first point of s0 is
            inside of s1 using the intersection point method

        @params:
            `LinearRing` s0: ring inside `s1`
            `LinearRing` s1: ring containing `s0`

        @return: `bool` first coordinate of `s0` inside `s1`
    */

    return (point_in_ring(s0.border[0], s1));
}


Multi_Polygon generate_exterior_border(Precinct_Group precinct_group) {
    /*
        Get the exterior border of a shape with interior components.
        Equivalent to 'dissolve' in mapshaper - remove bordering edges.
        Uses the Clipper library by Angus Johnson to union many polygons
        efficiently.

        @params:
            `precinct_group`: A precinct group to generate the border of

        @return:
            Multi_Polygon: exterior border of `precinct_group`
    */ 

    // create paths array from polygon
	ClipperLib::Paths subj;

    for (Precinct p : precinct_group.precincts)
        for (ClipperLib::Path path : shape_to_paths(p))
            subj.push_back(path);


    ClipperLib::Paths solutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

    return paths_to_multi_shape(solutions);
}


ClipperLib::Path ring_to_path(Geometry::LinearRing ring) {
    /*
        Creates a clipper Path object from a
        given Polygon object by looping through points
    */

    ClipperLib::Path p;
    for (coordinate point : ring.border )
        p.push_back(ClipperLib::IntPoint(point[0], point[1]));

    return p;
}


Geometry::LinearRing path_to_ring(ClipperLib::Path path) {
    /*
        Creates a shape object from a clipper Path
        object by looping through points
    */

    Geometry::LinearRing s;

    for (ClipperLib::IntPoint point : path ) {
        coordinate p = {point.X, point.Y};
        if (p[0] != 0 && p[1] != 0) s.border.push_back(p);
    }

    if (s.border[0] != s.border[s.border.size() - 1])
        s.border.push_back(s.border[0]);

    return s;
}


ClipperLib::Paths shape_to_paths(Geometry::Polygon shape) {

    if (shape.hull.border[0] != shape.hull.border[shape.hull.border.size() - 1])
        shape.hull.border.push_back(shape.hull.border[0]);

    ClipperLib::Paths p;
    p.push_back(ring_to_path(shape.hull));
    
    for (Geometry::LinearRing ring : shape.holes) {
        if (ring.border[0] != ring.border[ring.border.size() - 1])
            ring.border.push_back(ring.border[0]);

        ClipperLib::Path path = ring_to_path(ring);
        ReversePath(path);
        p.push_back(path);
    }

    return p;
}


Geometry::Multi_Polygon paths_to_multi_shape(ClipperLib::Paths paths) {
    /*
        @desc: 
              Create a Multi_Polygon object from a clipper Paths
              (multi path) object through nested iteration

        @params: `ClipperLib::Paths` paths: A
        @warn:
            `ClipperLib::ReversePath` is arbitrarily called here,
            and there should be better ways to check whether or not
            it's actually needed
    */

    Multi_Polygon ms;
    ReversePaths(paths);

    for (ClipperLib::Path path : paths) {
        if (!ClipperLib::Orientation(path)) {
            Geometry::LinearRing border = path_to_ring(path);
            if (border.border[0] == border.border[border.border.size() - 1]) {
                // border.border.insert(border.border.begin(), border.border[border.border.size() - 1]);
                Geometry::Polygon s(border);
                ms.border.push_back(s);
            }
        }
        else {
            // std::cout << "hole" << std::endl;
            ReversePath(path);
            Geometry::LinearRing hole = path_to_ring(path);
            ms.holes.push_back(hole);
        }
    }

    return ms;
}

Multi_Polygon poly_tree_to_shape(ClipperLib::PolyTree tree) {
    /*
        Loops through top-level children of a
        PolyTree to access outer-level polygons. Returns
        a multi_shape object containing these outer polys.
    */
   
    Multi_Polygon ms;
    
    for (ClipperLib::PolyNode* polynode : tree.Childs) {
        // if (polynode->IsHole()) x++;
        Geometry::LinearRing s = path_to_ring(polynode->Contour);
        Polygon shape(s);
        ms.border.push_back(shape);
    }

    return ms;
}

Polygon generate_gon(coordinate c, double radius, int n) {
    /*
        Takes a radius, center, and number of sides to generate
        a regular polygon around that center with that radius
    */

    double angle = 360 / n;
    coordinate_set coords;

    for (int i = 0; i < n; i++) {
        double x = radius * std::cos((angle * i) * PI/180);
        double y = radius * std::sin((angle * i) * PI/180);
        coords.push_back({(int)x + c[0], (int)y + c[1]});
    }

    LinearRing lr(coords);
    return Polygon(lr);
}


bool point_in_circle(Geometry::coordinate center, double radius, Geometry::coordinate point) {
    /*
        @desc:
            Determines whetehr or not a point is inside a
            circle by checking distance to the center

        @params:
            `Geometry::coordinate` center: x/y coords of the circle center
            `double` radius: radius of the circle
            `Geometry::coordinate` point: point to check
    
        @return: `bool` point is inside
    */

    return (get_distance(center, point) <= radius);
}



Geometry::bounding_box Geometry::LinearRing::get_bounding_box() {
    int top = border[0][1], 
    bottom = border[0][1], 
    left = border[0][0], 
    right = border[0][0];

    // loop through and find actual corner using ternary assignment
    for (Geometry::coordinate coord : border) {
        if (coord[1] > top) top = coord[1];
        if (coord[1] < bottom) bottom = coord[1];
        if (coord[0] < left) left = coord[0];
        if (coord[0] > right) right = coord[0];
    }

    return {top, bottom, left, right}; // return bounding box
}


Geometry::bounding_box Geometry::Polygon::get_bounding_box() {
    // set dummy extremes
    int top = hull.border[0][1], 
        bottom = hull.border[0][1], 
        left = hull.border[0][0], 
        right = hull.border[0][0];

    // loop through and find actual corner using ternary assignment
    for (Geometry::coordinate coord : hull.border) {
        if (coord[1] > top) top = coord[1];
        if (coord[1] < bottom) bottom = coord[1];
        if (coord[0] < left) left = coord[0];
        if (coord[0] > right) right = coord[0];
    }

    return {top, bottom, left, right}; // return bounding box
}


bool bound_overlap(Geometry::bounding_box b1, Geometry::bounding_box b2) {
    /*
        @desc: Determines whether or not two rects overlap
        @params: `Geometry::bounding_box` b1, b2: bounding boxes to check overlap
        @return: `bool` do rects overlap
    */

    if (b1[2] > b2[3] || b2[2] > b1[3]) return false;
    if (b1[1] > b2[0] || b2[1] > b1[0]) return false;
    return true;
}

bool bound_inside(hte::Geometry::bounding_box b1, hte::Geometry::bounding_box b2) {
    // gets whether or not b1 is inside b2
    return (b1[0] < b2[0] && b1[1] > b2[1] && b1[2] > b2[2] && b1[3] < b2[3]);
}

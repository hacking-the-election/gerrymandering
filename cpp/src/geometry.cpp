/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:                  Sun, Jun 21
 
 Definition of useful functions for
 computational geometry. Basic 
 calculation, area, bordering - no
 algorithmic specific methods.
========================================*/

#include <iostream>
#include <chrono>
#include <random>
#include <math.h>
#include "../include/hte.h"

using namespace hte;
using namespace std;


Segment hte::PointsToSegment(Point2d c1, Point2d c2) {
    /*
        @desc: combines coordinates into a segment array
        @params: `c1`, `c2`: coordinates 1 and 2 in segment
        @return: `hte::segment` a segment with the coordinates provided
    */
    return {{c1.x, c1.y, c2.x, c2.y}};
}


double hte::GetDistance(Segment s) {
    /* 
        @desc: Distance formula on a segment array
        @params: `s`: a segment to get the distance of
        @return: `double` the distance of the segment
    */
   
    return sqrt(pow((s[2] - s[0]), 2) + pow((s[3] - s[1]), 2));
}


double hte::GetDistance(Point2d c0, Point2d c1) {
    /*
        @desc: Distance formula on two separate points
        @params: `c1`, `c2`: coordinates 1 and 2 in segment
        @return: `double` the distance between the coordinates
    */

    return GetDistance(PointsToSegment(c0, c1));
}


vector<long> hte::GetEquation(Segment s) {
    long dy = s[3] - s[1], dx = s[2] - s[0], m;
    if (dx != 0) m = dy / dx;
    else m = INFINITY;
    long b = -1 * ((m * s[0]) - s[1]);
    return {m, b};
}


SegmentVec hte::LinearRing::getSegments() {
    SegmentVec segs;

    for (int i = 0; i < border.size(); i++) {
        Point2d c1 = border[i];   // starting coord
        Point2d c2;               // ending coord

        // find position of ending coordinate
        if (i == border.size() - 1) c2 = border[0];
        else c2 = border[i + 1];

        if (c1 != c2) segs.push_back(PointsToSegment(c1, c2)); // add to list
    }

    return segs;
}


SegmentVec hte::Polygon::getSegments() {
    SegmentVec segs = this->hull.getSegments();
    
    for (LinearRing hole : this->holes)
        for (Segment seg : hole.getSegments())
            segs.push_back(seg);

    return segs;
}


SegmentVec hte::MultiPolygon::getSegments() {
    SegmentVec segs;
    for (Polygon s : this->border)
        for (Segment seg : s.getSegments())
            segs.push_back(seg);

    return segs;
}


Point2d hte::LinearRing::getCentroid() {
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


BoostPolygon hte::RingToBoostPoly(LinearRing shape) {
    /*
        Converts a shape object into a boost polygon object
        by looping over each point and manually adding it to a 
        boost polygon using assign_points and vectors
    */

    BoostPolygon poly;
    // create vector of boost points
    std::vector<BoostPoint2d> points;
    points.reserve(shape.border.size());
    for (Point2d c : shape.border) 
        points.emplace_back(BoostPoint2d(c.x, c.y)),

    boost::geometry::assign_points(poly, points);
    return poly;
}


double hte::LinearRing::getSignedArea() {
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

        area += (border[i].x * border[j].y) - (border[i].y * border[j].x);
    }

    return area / 2.0;
}


double hte::LinearRing::getPerimeter() {
    /*
        @desc: returns the perimeter of a LinearRing object by summing distance
        @params: none
        @return: `double` perimeter
    */

    double t = 0;
    for (Segment s : getSegments())
        t += GetDistance(s);    

    return t;
}


Point2d hte::Polygon::getCentroid() {
    /*
        @desc:
            returns average centroid from list of `holes`
            and `hull` by calling `LinearRing::getCentroid`

        @params: none
        @return: `coordinate` average centroid of shape
    */

    return (hull.getCentroid());
}


Point2d hte::MultiPolygon::getCentroid() {
    /*
        @desc:
            returns average centroid from list of `holes`
            and `hull` by calling `LinearRing::getCentroid`

        @params: none
        @return: `coordinate` average centroid of shape
    */

    Point2d average = {0,0};
    for (Polygon p : border) {
        BoostPoint2d center;
        boost::geometry::centroid(hte::RingToBoostPoly(p.hull), center);
        average.x += center.x();
        average.y += center.y();
    }

    return {static_cast<long>(average.x / border.size()), static_cast<long>(average.y / border.size())};
}


Point2d hte::PrecinctGroup::getCentroid() {
    /*
        @desc:
            returns average centroid from list of `holes`
            and `hull` by calling `LinearRing::getCentroid`

        @params: none
        @return: `coordinate` average centroid of shape
    */

    Point2d average = {0,0};

    for (Precinct s : precincts) {
        Point2d center = s.hull.getCentroid();
        average.x += center.x;
        average.y += center.y;
    }

    return {(static_cast<long>(average.x) / static_cast<long>(precincts.size())), (static_cast<long>(average.y) / static_cast<long>(precincts.size()))};
}


double hte::Polygon::getSignedArea() {
    /*
        @desc:
            gets the area of the hull of a shape
            minus the combined area of any holes

        @params: none
        @return: `double` totale area of shape
    */

    double area = hull.getSignedArea();
    for (hte::LinearRing h : holes)
        area -= h.getSignedArea();

    return area;
}


double hte::PrecinctGroup::getArea() {
    double sum = 0;
    
    for (Precinct p : precincts)
        sum += abs(p.getSignedArea());
    return sum;
}


double hte::Polygon::getPerimeter() {
    /*
        @desc:
            gets the sum perimeter of all LinearRings
            in a shape object, including holes

        @params: none
        @return: `double` total perimeter of shape
    */

    double perimeter = hull.getPerimeter();
    for (hte::LinearRing h : holes)
        perimeter += h.getPerimeter();

    return perimeter;
}


double MultiPolygon::getSignedArea() {
    /*
        @desc: gets sum area of all Polygon objects in border
        @params: none
        @return: `double` total area of shapes
    */

    double total = 0;
    for (Polygon s : border)
        total += s.getSignedArea();

    return total;
}


double hte::MultiPolygon::getPerimeter() {
    /*
        @desc:
            gets sum perimeter of a multi shape object
            by looping through each shape and calling method
            
        @params: none
        @return `double` total perimeter of shapes array
    */

    double p = 0;
    for (Polygon shape : border)
        p += shape.getPerimeter();

    return p;
}


bool hte::GetBordering(Polygon s0, Polygon s1) {
    /*
        @desc: gets whether or not two shapes touch each other
        @params: `Polygon` s0, `Polygon` s1: shapes to check bordering
        @return: `bool` shapes are boording
    */
    
    // create paths array from polygon
	ClipperLib::Paths subj;
    subj.push_back(RingToPath(s0.hull));

    ClipperLib::Paths clip;
    clip.push_back(RingToPath(s1.hull));

    ClipperLib::Paths solutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

    MultiPolygon ms = PathsToMultiPolygon(solutions);
    return (ms.border.size() == 1);
}


bool hte::GetPointInRing(hte::Point2d coord, hte::LinearRing lr) {
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
    ClipperLib::Path path = RingToPath(lr);
    ClipperLib::IntPoint p(coord.x, coord.y);
    return (!(ClipperLib::PointInPolygon(p, path) == 0));
}


bool hte::GetInside(hte::LinearRing s0, hte::LinearRing s1) {
    /*
        @desc:
            gets whether or not s0 is inside of 
            s1 using the intersection point method

        @params:
            `LinearRing` s0: ring inside `s1`
            `LinearRing` s1: ring containing `s0`

        @return: `bool` `s0` inside `s1`
    */

    for (Point2d c : s0.border)
        if (!GetPointInRing(c, s1)) return false;

    return true;
}


bool hte::GetInsideFirst(hte::LinearRing s0, hte::LinearRing s1) {
    /*
        @desc:
            gets whether or not the first point of s0 is
            inside of s1 using the intersection point method

        @params:
            `LinearRing` s0: ring inside `s1`
            `LinearRing` s1: ring containing `s0`

        @return: `bool` first coordinate of `s0` inside `s1`
    */

    return (GetPointInRing(s0.border[0], s1));
}


MultiPolygon hte::GenerateExteriorBorder(PrecinctGroup pg) {
    /*
        Get the exterior border of a shape with interior components.
        Equivalent to 'dissolve' in mapshaper - remove bordering edges.
        Uses the Clipper library by Angus Johnson to union many polygons
        efficiently.

        @params:
            `precinct_group`: A precinct group to generate the border of

        @return:
            MultiPolygon: exterior border of `precinct_group`
    */ 

    // create paths array from polygon
	ClipperLib::Paths subj;

    for (Precinct p : pg.precincts)
        for (ClipperLib::Path path : PolygonToPaths(p))
            subj.push_back(path);


    ClipperLib::Paths solutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

    return PathsToMultiPolygon(solutions);
}


ClipperLib::Path hte::RingToPath(hte::LinearRing ring) {
    /*
        Creates a clipper Path object from a
        given Polygon object by looping through points
    */

    ClipperLib::Path p;
    for (Point2d point : ring.border)
        p.push_back(ClipperLib::IntPoint(point.x, point.y));

    return p;
}


hte::LinearRing hte::PathToRing(ClipperLib::Path path) {
    /*
        Creates a shape object from a clipper Path
        object by looping through points
    */

    hte::LinearRing s;

    for (ClipperLib::IntPoint point : path) {
        Point2d p = {point.X, point.Y};
        // @warn i have no idea what the below line was trying to do?
        // if (p.x != 0 && p.y != 0)
        s.border.push_back(p);
    }

    if (s.border[0] != s.border[s.border.size() - 1])
        s.border.push_back(s.border[0]);

    return s;
}


ClipperLib::Paths hte::PolygonToPaths(hte::Polygon shape) {

    if (shape.hull.border[0] != shape.hull.border[shape.hull.border.size() - 1])
        shape.hull.border.push_back(shape.hull.border[0]);

    ClipperLib::Paths p;
    p.push_back(RingToPath(shape.hull));
    
    for (hte::LinearRing ring : shape.holes) {
        if (ring.border[0] != ring.border[ring.border.size() - 1])
            ring.border.push_back(ring.border[0]);

        ClipperLib::Path path = RingToPath(ring);
        ReversePath(path);
        p.push_back(path);
    }

    return p;
}


MultiPolygon hte::PathsToMultiPolygon(ClipperLib::Paths paths) {
    /*
        @desc: 
              Create a MultiPolygon object from a clipper Paths
              (multi path) object through nested iteration

        @params: `ClipperLib::Paths` paths: A
        @warn:
            `ClipperLib::ReversePath` is arbitrarily called here,
            and there should be better ways to check whether or not
            it's actually needed
    */

    MultiPolygon ms;
    ReversePaths(paths);

    for (ClipperLib::Path path : paths) {
        if (!ClipperLib::Orientation(path)) {
            hte::LinearRing border = PathToRing(path);
            if (border.border[0] == border.border[border.border.size() - 1]) {
                hte::Polygon s(border);
                ms.border.push_back(s);
            }
        }
        else {
            ClipperLib::ReversePath(path);
            hte::LinearRing hole = hte::PathToRing(path);
            ms.holes.push_back(hole);
        }
    }

    return ms;
}


Polygon hte::GenerateGon(Point2d c, double radius, int n) {
    /*
        Takes a radius, center, and number of sides to generate
        a regular polygon around that center with that radius
    */

    double angle = 360 / n;
    Point2dVec coords;

    for (int i = 0; i < n; i++) {
        double x = radius * std::cos((angle * i) * PI/180);
        double y = radius * std::sin((angle * i) * PI/180);
        coords.push_back({(int)x + c.x, (int)y + c.y});
    }

    LinearRing lr(coords);
    return Polygon(lr);
}


bool hte::GetPointInCircle(hte::Point2d center, double radius, hte::Point2d point) {
    /*
        @desc:
            Determines whetehr or not a point is inside a
            circle by checking distance to the center

        @params:
            `hte::coordinate` center: x/y coords of the circle center
            `double` radius: radius of the circle
            `hte::coordinate` point: point to check
    
        @return: `bool` point is inside
    */

    return (GetDistance(center, point) <= radius);
}



BoundingBox hte::LinearRing::getBoundingBox() {
    int top = border[0].y, 
    bottom = border[0].y, 
    left = border[0].x, 
    right = border[0].x;

    // loop through and find actual corner using ternary assignment
    for (hte::Point2d coord : border) {
        if (coord.y > top) top = coord.y;
        if (coord.y < bottom) bottom = coord.y;
        if (coord.x < left) left = coord.x;
        if (coord.x > right) right = coord.x;
    }

    return {top, bottom, left, right}; // return bounding box
}


BoundingBox hte::Polygon::getBoundingBox() {
    // set dummy extremes
    int top = hull.border[0].y, 
        bottom = hull.border[0].y, 
        left = hull.border[0].x, 
        right = hull.border[0].x;

    // loop through and find actual corner using ternary assignment
    for (hte::Point2d coord : hull.border) {
        if (coord.y > top) top = coord.y;
        if (coord.y < bottom) bottom = coord.y;
        if (coord.x < left) left = coord.x;
        if (coord.x > right) right = coord.x;
    }

    return {top, bottom, left, right}; // return bounding box
}


BoundingBox hte::MultiPolygon::getBoundingBox() {
    // set dummy extremes
    int top = border[0].hull.border[0].y, 
        bottom = border[0].hull.border[0].y, 
        left = border[0].hull.border[0].x, 
        right = border[0].hull.border[0].x;

    for (Polygon p : border) {
        // loop through and find actual corner using ternary assignment
        for (hte::Point2d coord : p.hull.border) {
            if (coord.y > top) top = coord.y;
            if (coord.y < bottom) bottom = coord.y;
            if (coord.x < left) left = coord.x;
            if (coord.x > right) right = coord.x;
        }
    }

    return {top, bottom, left, right}; // return bounding box
}


BoundingBox hte::PrecinctGroup::getBoundingBox() {
    // set dummy extremes
    if (precincts.size() != 0) {
        int top = precincts[0].hull.border[0].y, 
            bottom = precincts[0].hull.border[0].y, 
            left = precincts[0].hull.border[0].x, 
            right = precincts[0].hull.border[0].x;

        for (Precinct p : precincts) {
            // loop through and find actual corner using ternary assignment
            for (hte::Point2d coord : p.hull.border) {
                if (coord.y > top) top = coord.y;
                if (coord.y < bottom) bottom = coord.y;
                if (coord.x < left) left = coord.x;
                if (coord.x > right) right = coord.x;
            }
        }
        return {top, bottom, left, right}; // return bounding box
    }
    else {
        cout  << "lol no precincts here bro" << endl;
        return {0,0,0,0};
    }
}


bool hte::GetBoundOverlap(BoundingBox b1, BoundingBox b2) {
    /*
        @desc: Determines whether or not two rects overlap
        @params: `BoundingBox` b1, b2: bounding boxes to check overlap
        @return: `bool` do rects overlap
    */

    if (b1[2] > b2[3] || b2[2] > b1[3]) return false;
    if (b1[1] > b2[0] || b2[1] > b1[0]) return false;
    return true;
}

bool hte::GetBoundInside(BoundingBox b1, BoundingBox b2) {
    // gets whether or not b1 is inside b2
    return (b1[0] < b2[0] && b1[1] > b2[1] && b1[2] > b2[2] && b1[3] < b2[3]);
}

#include <iostream>
#include <chrono>
#include <random>
#include <math.h>

#include "../include/hte_common.h"


namespace hte
{

template<>
Point2d<ClipperCoord>::operator ClipperLib::IntPoint() const
{
    return ClipperLib::IntPoint(x, y);
}


template<typename T>
inline double GetDistanceSquared(const Point2d<T>& p1, const Point2d<T>& p2)
{
    return (p2.x - p1.x) * (p2.x - p1.y) + (p2.y - p1.y) * (p2.y - p1.y);
}


// TODO: Untested, and likely implemented poorly / incorrectly
template<typename T>
void LinearRing<T>::updateCentroid()
{
    forceValid();
    T Cx = 0., Cy = 0.;

    for (const auto& [first, second] : GetSegments(*this))
    {
        Cx += (first.x + second.x) * ((first.x * second.y) - (second.x * first.y));
        Cy += (first.y + second.y) * ((first.x * second.y) - (second.x * first.y));
    }

    centroid[0] = static_cast<T>(round(1.0 / (6.0 * this->get_area()) * (double) Cx));
    centroid[1] = static_cast<T>(round(1.0 / (6.0 * this->get_area()) * (double) Cy));
}


template<typename T>
void LinearRing<T>::updateSignedArea()
{
    signedArea = 0.;

    for (const auto& [first, second] : GetSegments(*this))
    {
        signedArea += (first.x * second.y) - (first.y * second.x);
    }

    signedArea /= 2.0;
}


template<typename T>
void LinearRing<T>::updatePerimeter()
{
    double t = 0.0;
    for (const auto& [first, second] : GetSegments(*this)) t += sqrt(GetDistanceSquared(first, second));
}


template<typename T>
void Polygon<T>::updateSignedArea()
{

    signedArea = getHull().getSignedArea();
    for (const auto& h : getHoles()) signedArea -= h.getSignedArea();
}


ClipperLib::Path ClipperBuffer::LinearRingToPath(const LinearRing<ClipperCoord>& ring)
{
    ClipperLib::Path p;
    p.reserve(ring.size());
    for (const Point2d<ClipperCoord>& point : ring)
        p.emplace_back(point.x, point.y);
    return p;
}


void ClipperBuffer::addLinearRing(const LinearRing<ClipperCoord>& ring, PolyType polyType)
{
    clipper.AddPath(LinearRingToPath(ring), static_cast<ClipperLib::PolyType>(polyType), true);
}


void ClipperBuffer::addPolygon(const Polygon<ClipperCoord>& polygon, PolyType polyType)
{
    ClipperLib::Paths paths;
    paths.reserve(polygon.size());
    for (const LinearRing<ClipperCoord>& ring : polygon)
        paths.push_back(LinearRingToPath(ring));
    clipper.AddPaths(paths, static_cast<ClipperLib::PolyType>(polyType), true);
}


void ClipperBuffer::performClip(ClipType clipType, ClipperLib::Paths& solution, PolyFillType subjFillType, PolyFillType clipFillType)
{
    clipper.Execute(static_cast<ClipperLib::ClipType>(clipType),
                    solution,
                    static_cast<ClipperLib::PolyFillType>(subjFillType),
                    static_cast<ClipperLib::PolyFillType>(clipFillType));
}


bool GetBordering(const Polygon<ClipperCoord>& a, const Polygon<ClipperCoord>& b)
{
    ClipperLib::Paths abUnion;
    ClipperBuffer buffer;
    buffer.addLinearRing(a.getHull(), PolyType::SUBJ);
    buffer.addLinearRing(b.getHull(), PolyType::CLIP);
    buffer.performClip(ClipType::UNION, abUnion,
                       PolyFillType::NONZERO, PolyFillType::NONZERO);
    /* PrintPaths(abUnion); */
    return (abUnion.size() == 1);
}

// double PrecinctGroup::getArea() {
//     double sum = 0;
    
//     for (Precinct p : precincts)
//         sum += abs(p.getSignedArea());
//     return sum;
// }


// double Polygon::getPerimeter() {
//     /*
//         @desc:
//             gets the sum perimeter of all LinearRings
//             in a shape object, including holes

//         @params: none
//         @return: `double` total perimeter of shape
//     */

//     double perimeter = hull.getPerimeter();
//     for (LinearRing h : holes)
//         perimeter += h.getPerimeter();

//     return perimeter;
// }


// double MultiPolygon::getSignedArea() {
//     /*
//         @desc: gets sum area of all Polygon objects in border
//         @params: none
//         @return: `double` total area of shapes
//     */

//     double total = 0;
//     for (Polygon s : border)
//         total += s.getSignedArea();

//     return total;
// }


// double MultiPolygon::getPerimeter() {
//     /*
//         @desc:
//             gets sum perimeter of a multi shape object
//             by looping through each shape and calling method
            
//         @params: none
//         @return `double` total perimeter of shapes array
//     */

//     double p = 0;
//     for (Polygon shape : border)
//         p += shape.getPerimeter();

//     return p;
// }


// bool GetBordering(Polygon s0, Polygon s1) {
//     /*
//         @desc: gets whether or not two shapes touch each other
//         @params: `Polygon` s0, `Polygon` s1: shapes to check bordering
//         @return: `bool` shapes are boording
//     */
    
//     // create paths array from polygon
// 	ClipperLib::Paths subj;
//     subj.push_back(RingToPath(s0.hull));

//     ClipperLib::Paths clip;
//     clip.push_back(RingToPath(s1.hull));

//     ClipperLib::Paths solutions;
//     ClipperLib::Clipper c; // the executor

//     // execute union on paths array
//     c.AddPaths(subj, ClipperLib::ptSubject, true);
//     c.AddPaths(clip, ClipperLib::ptClip, true);
//     c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

//     MultiPolygon ms = PathsToMultiPolygon(solutions);
//     return (ms.border.size() == 1);
// }


// bool GetPointInRing(Point2d coord, LinearRing lr) {
//     /*
//         @desc:
//             gets whether or not a point is in a ring using
//             the ray intersection method (clipper implementation)

//         @ref: http://www.angusj.com/delphi/Clipper/documentation/Docs/Units/ClipperLib/Functions/PointInPolygon.htm
//         @params: 
//             `coordinate` coord: the point to check
//             `LinearRing` lr: the shape to check the point against
        
//         @return: `bool` point is in/on polygon
//     */

//     // close ring for PIP problem
//     if (lr.border[0] != lr.border[lr.border.size() - 1])
//         lr.border.push_back(lr.border[0]);

//     // convert to clipper types for builtin function
//     ClipperLib::Path path = RingToPath(lr);
//     ClipperLib::IntPoint p(coord.x, coord.y);
//     return (!(ClipperLib::PointInPolygon(p, path) == 0));
// }


// bool GetInside(LinearRing s0, LinearRing s1) {
//     /*
//         @desc:
//             gets whether or not s0 is inside of 
//             s1 using the intersection point method

//         @params:
//             `LinearRing` s0: ring inside `s1`
//             `LinearRing` s1: ring containing `s0`

//         @return: `bool` `s0` inside `s1`
//     */

//     for (Point2d c : s0.border)
//         if (!GetPointInRing(c, s1)) return false;

//     return true;
// }


// bool GetInsideFirst(LinearRing s0, LinearRing s1) {
//     /*
//         @desc:
//             gets whether or not the first point of s0 is
//             inside of s1 using the intersection point method

//         @params:
//             `LinearRing` s0: ring inside `s1`
//             `LinearRing` s1: ring containing `s0`

//         @return: `bool` first coordinate of `s0` inside `s1`
//     */

//     return (GetPointInRing(s0.border[0], s1));
// }


// MultiPolygon GenerateExteriorBorder(PrecinctGroup pg) {
//     /*
//         Get the exterior border of a shape with interior components.
//         Equivalent to 'dissolve' in mapshaper - remove bordering edges.
//         Uses the Clipper library by Angus Johnson to union many polygons
//         efficiently.

//         @params:
//             `precinct_group`: A precinct group to generate the border of

//         @return:
//             MultiPolygon: exterior border of `precinct_group`
//     */ 

//     // create paths array from polygon
// 	ClipperLib::Paths subj;

//     for (Precinct p : pg.precincts)
//         for (ClipperLib::Path path : PolygonToPaths(p))
//             subj.push_back(path);


//     ClipperLib::Paths solutions;
//     ClipperLib::Clipper c; // the executor

//     // execute union on paths array
//     c.AddPaths(subj, ClipperLib::ptSubject, true);
//     c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

//     return PathsToMultiPolygon(solutions);
// }


// ClipperLib::Path RingToPath(LinearRing ring) {
//     /*
//         Creates a clipper Path object from a
//         given Polygon object by looping through points
//     */

//     ClipperLib::Path p;
//     for (Point2d point : ring.border)
//         p.push_back(ClipperLib::IntPoint(point.x, point.y));

//     return p;
// }


// LinearRing PathToRing(ClipperLib::Path path) {
//     /*
//         Creates a shape object from a clipper Path
//         object by looping through points
//     */

//     LinearRing s;

//     for (ClipperLib::IntPoint point : path) {
//         Point2d p = {point.X, point.Y};
//         // @warn i have no idea what the below line was trying to do?
//         // if (p.x != 0 && p.y != 0)
//         s.border.push_back(p);
//     }

//     if (s.border[0] != s.border[s.border.size() - 1])
//         s.border.push_back(s.border[0]);

//     return s;
// }


// ClipperLib::Paths PolygonToPaths(Polygon shape) {

//     if (shape.hull.border[0] != shape.hull.border[shape.hull.border.size() - 1])
//         shape.hull.border.push_back(shape.hull.border[0]);

//     ClipperLib::Paths p;
//     p.push_back(RingToPath(shape.hull));
    
//     for (LinearRing ring : shape.holes) {
//         if (ring.border[0] != ring.border[ring.border.size() - 1])
//             ring.border.push_back(ring.border[0]);

//         ClipperLib::Path path = RingToPath(ring);
//         ReversePath(path);
//         p.push_back(path);
//     }

//     return p;
// }


// MultiPolygon PathsToMultiPolygon(ClipperLib::Paths paths) {
//     /*
//         @desc: 
//               Create a MultiPolygon object from a clipper Paths
//               (multi path) object through nested iteration

//         @params: `ClipperLib::Paths` paths: A
//         @warn:
//             `ClipperLib::ReversePath` is arbitrarily called here,
//             and there should be better ways to check whether or not
//             it's actually needed
//     */

//     MultiPolygon ms;
//     ReversePaths(paths);

//     for (ClipperLib::Path path : paths) {
//         if (!ClipperLib::Orientation(path)) {
//             LinearRing border = PathToRing(path);
//             if (border.border[0] == border.border[border.border.size() - 1]) {
//                 Polygon s(border);
//                 ms.border.push_back(s);
//             }
//         }
//         else {
//             ClipperLib::ReversePath(path);
//             LinearRing hole = PathToRing(path);
//             ms.holes.push_back(hole);
//         }
//     }

//     return ms;
// }

template<typename T>
Polygon<double> GenerateGon(Point2d<T> c, double radius, int n)
{
    double angle = 360.0 / n;

    LinearRing<double> lr(n);

    for (int i = 0; i < n; i++)
    {
        double x = radius * std::cos((angle * i) * (PI / 180.0));
        double y = radius * std::sin((angle * i) * (PI / 180.0));
        lr.push_back({x + c.x, y + c.y});
    }

    return Polygon(lr);
}


// template<typename T>
// bool GetPointInCircle(Point2d<T> center, double radius, Point2d<T> point)
// {
//     return (GetDistance(center, point) <= radius);
// }


// bool GetBoundOverlap(BoundingBox b1, BoundingBox b2)
// {
//     if (b1[2] > b2[3] || b2[2] > b1[3]) return false;
//     if (b1[1] > b2[0] || b2[1] > b1[0]) return false;
//     return true;
// }


// bool GetBoundInside(BoundingBox b1, BoundingBox b2)
// {
//     return (b1[0] < b2[0] && b1[1] > b2[1] && b1[2] > b2[2] && b1[3] < b2[3]);
// }



std::ostream& operator<<(std::ostream& out, const ClipperLib::IntPoint& pt)
{
    out << "{" << pt.X << ", " << pt.Y << "}";
    return out;
}

}

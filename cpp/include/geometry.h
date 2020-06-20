/*=======================================
 geometry.hpp                   k-vernooy
 last modified:               Fri, Jun 19
 
 Declarations of functions for geometrical
 manipulations and searching algorithms.
========================================*/

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "util.h"

/** \cond */
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
/** \cond */

#include "../lib/Clipper/cpp/clipper.hpp"
#include "../lib/Miniball.hpp"

#define PI 3.14159265358979323


// typedefs for other libraries, not to be included in the
typedef boost::geometry::model::d2::point_xy<long long int> BoostPoint2d;
typedef boost::geometry::model::polygon<BoostPoint2d> BoostPolygon;
typedef std::vector<std::vector<double> >::const_iterator PointIterator; 
typedef std::vector<double>::const_iterator CoordIterator;
typedef Miniball::Miniball <Miniball::CoordAccessor<PointIterator, CoordIterator> > MB;


namespace hte {
namespace Geometry {
    /*
     * Geometry classes. These all define groups of points
     * (can be converted to segments) that make up different
     * types of geometries.
     */

    class  LinearRing;      //!< a group of lines
    class  Polygon;         //!< Exterior and optional interior LinearRings (for holes)
    class  MultiPolygon;    //!< A group of Polygons
    struct Point2d;

    typedef std::vector<Point2d>   Point2dVec;    //!< list of coordinates: {{x1, y1} ... {xn, yn}};
    typedef std::array<long, 4>    BoundingBox;   //!< an array of 4 max/mins: {top, bottom, left, right};
    typedef std::array<long, 4>    Segment;       //!< a set of two coordinates:
    typedef std::vector<Segment>   SegmentVec;    //!< list of multiple segments
    typedef double                 UnitInterval;  //!< for values between [0, 1]


    /**
     * \brief Types of geometric boolean operations 
     */
    enum class ClipType {
        UNION,
        INTERSECTION,
        DIFFERENCE,
        XOR
    };

    /**
     * A 2-dimensional cartesian point, consisting
     * of public x and y integer coordinates.
     */
    struct Point2d {
        long x;
        long y;
        Point2d() {};
        Point2d(long int x, long int y)
            : x(x), y(y) {}

        Point2d(std::array<long, 2> list) {
            x = list[0];
            y = list[1];
        }

        // add operator overloading for object equality
        friend bool operator== (const Point2d& l1, const Point2d& l2);
        friend bool operator!= (const Point2d& l1, const Point2d& l2);
    };


    /**
     * \brief A closed set of points
     * 
     * A class containing a closed coordinate_set, analagous to
     * the `LinearRing` property in most GeoJson. 
     * A wrapper for the coordinate_set typedef
     * with extended method functionality for geometric properties
     * such as area, perimeter, and centroid calculation
     * 
     * \throw Must be passed a closed coordinate set in the 
     * constructor or will throw Exceptions::LinearRingOpen
     */
    class LinearRing {
        public:

            LinearRing() : centroid(NULL, NULL) {}
            LinearRing(Point2dVec b) : centroid(NULL, NULL) {
                if (b[0] != b[b.size() - 1]) {
                    throw ::hte::Util::Exceptions::LinearRingOpen();
                }

                border = b;
            }

            Point2dVec  border;   //!< A closed set of coordinates
            Point2d     centroid;    //!< The centroid (default NULL,NULL) of the ring

            /**
             * \brief Get the signed area of the linear ring
             * Use the shoelace theorem to return the signed area
             * of the `border` property. Sign is based on coordinate order.
             * 
             * @return Signed area of `border`
            */
            virtual double       getArea();
            virtual std::string  toJson();
            virtual double       getPerimeter();        // sum distance of segments
            virtual Point2d      getCentroid();         // average of all points in shape
            virtual SegmentVec   getSegments();         // return a segment list with shape's segments
            virtual BoundingBox  getBoundingBox();

            // add operator overloading for object equality
            friend bool operator== (const LinearRing& l1, const LinearRing& l2);
            friend bool operator!= (const LinearRing& l1, const LinearRing& l2);
    };


    /**
     * \brief A shape with a hull and holes
     * 
     * Contains a public border of a LinearRing object
     * and a public vector of LinearRings for holes.
     * Polygon objects may contain holes, but do not have
     * multiple exterior borders
     */
    class Polygon {
        public:

            Polygon(){}
            Polygon(LinearRing hull)
                : hull(hull) {}
            Polygon(LinearRing hull, std::string shape_id)
                : hull(hull), shapeId(shape_id) {}

            Polygon(LinearRing hull, std::vector<LinearRing> holes) 
                : hull(hull), holes(holes) {}

            Polygon(LinearRing hull, std::vector<LinearRing> holes, std::string shape_id) 
                : hull(hull), holes(holes), shapeId(shape_id) {}


            LinearRing               hull;      //!< Ring of exterior hull coordinates
            std::vector<LinearRing>  holes;     //!< List of exterior holes in shape
            std::string              shapeId;  //!< The shape's GEOID, if applicable

            virtual std::string  toJson();          // get the coordinate data of the polygon as GeoJson
            virtual double       getSignedArea();   // return (area of shape - area of holes)
            virtual double       getPerimeter();    // total perimeter of holes + hull
            virtual Point2d      getCentroid();     // average centers of holes + hull
            virtual SegmentVec   getSegments();     // return a segment list with shape's segments
            virtual BoundingBox  getBoundingBox();  // get the bounding box of the hull

            // add operator overloading for object equality
            friend bool operator== (const Polygon& p1, const Polygon& p2);
            friend bool operator!= (const Polygon& p1, const Polygon& p2);
            
            int pop = 0;                     //!< Total population of the polygon
            int isPartOfMultiPolygon = -1;   //!< Internal data for parsing rules
    };

    
    
    /**
     * \brief A polygon object with multiple polygons.
     * 
     * Used for geographical objects similar to the entire discontiguous
     * US and Alaska within a single object, useful for graphics.
     * Each polygon has a mandatory `hull` property, and may contain holes.
     */
    class MultiPolygon : public Polygon {
        public:

            MultiPolygon(){} // default constructor
            MultiPolygon(std::vector<Polygon> s) {
                // constructor with assignment
                border = s;
            }
            
            MultiPolygon(std::vector<Polygon> s, std::string t_id) {
                // constructor with assignment
                border = s;
                shapeId = t_id;
            }

            MultiPolygon(std::vector<Data::Precinct> s) {
                // constructor with assignment
                for (Data::Precinct p : s) {
                    // copy precinct data to shape object
                    Polygon s = Polygon(p.hull, p.holes, p.shapeId);
                    border.push_back(s);
                }
            }

            double        getPerimeter();         // total perimeter of border array
            double        getSignedArea();        // total area of the border shape array
            virtual       std::string toJson();   // get a json string of the borders and holes
            Point2d       getCentroid();          // average centroid of inner polys
            SegmentVec    getSegments();          // return a segment list with shape's segments
            BoundingBox   getBoundingBox();

            std::vector<Polygon> border;

            // add operator overloading for object equality
            friend bool operator== (const MultiPolygon& s1, const MultiPolygon& s2);
            friend bool operator!= (const MultiPolygon& s1, const MultiPolygon& s2);
    };


    bool GetBordering(Polygon, Polygon);
    bool GetBoundOverlap(BoundingBox, BoundingBox);
    bool GetBoundInside(BoundingBox, BoundingBox);
    bool GetPointInRing(Point2d, LinearRing);
    bool GetInside(LinearRing, LinearRing);
    bool GetInsideFirst(LinearRing s0, LinearRing s1);
    bool GetPointInCircle(Point2d center, double radius, Point2d point);

    double             GetDistance(Point2d c1, Point2d c2);
    double             GetDistance(Segment s);
    
    Polygon            GenerateGon(Point2d center, double radius, int nSides);
    MultiPolygon       GenerateExteriorBorder(std::vector<Polygon> p);

    Segment            CoordsToSegment(Point2d c1, Point2d c2);
    LinearRing         PathToRing(ClipperLib::Path path);
    BoostPolygon       RingToBoostPoly(hte::Geometry::LinearRing);
    MultiPolygon       PathsToMultiPolygon(ClipperLib::Paths paths);
    ClipperLib::Path   RingToPath(LinearRing ring);
    ClipperLib::Paths  ShapeToPaths(Polygon shape);
}
}

#endif

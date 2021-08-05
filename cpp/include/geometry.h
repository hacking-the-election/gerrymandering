#ifndef _HTE_GEOMETRY_H
#define _HTE_GEOMETRY_H

#include <iostream>
#include <vector>
#include <utility>
#include <unordered_map>
#include <array>

#include "../lib/Clipper/cpp/clipper.hpp"
#include "util.h"

#define PI 3.14159265358979


namespace hte {


template<typename T>
struct Point2d
{
    Point2d() : x(), y() {}
    Point2d(T x, T y) : x(x), y(y)
    {
        static_assert(std::is_arithmetic<T>::value, "Point2d must be instantiated with an arithmetic type.");
    }

    T x, y;

template<typename Ts>
friend bool operator== (const Point2d<Ts>& l1, const Point2d<Ts>& l2);

template<typename Ts>
friend bool operator!= (const Point2d<Ts>& l1, const Point2d<Ts>& l2);

operator ClipperLib::IntPoint () const;
};


enum class Bounds
{
    TopLeft, TopRight, BottomLeft, BottomRight
};


template <class It>
struct PairwiseIterator
{
    It it;

    std::tuple<It, It> operator*() const
    {
        return std::tie(*it, *std::next(it));
    }

    PairwiseIterator& operator++()
    {
        ++it; return *this;
    }

    bool operator!=(PairwiseIterator cmp) const
    {
        return it != cmp.it;
    }
};


template <class Container>
class PairwiseRange
{
public:
    using it = typename Container::iterator;

    PairwiseRange(const Container& container) : begin_(std::begin(container)), end_(std::end(container)) {}

    PairwiseIterator<it> begin() const
    {
        return PairwiseIterator(begin_);
    }

    PairwiseIterator<it> end() const
    {
        if (begin_ == end_) return PairwiseIterator(end_);
        return PairwiseIterator(std::prev(end_));
    }

private:
    typename Container::iterator begin_, end_;
};


template <class C>
PairwiseRange<C> GetSegments(C&& container)
{
    return PairwiseRange<C>(std::forward<C>(container));
};


template<typename T>
class LinearRing : public std::vector<Point2d<T>>
{
public:
    LinearRing(std::initializer_list<Point2d<T>> points) : std::vector<Point2d<T>>(std::move(points)) {/*initialize();*/}

    template<class... Construct>
    LinearRing(Construct&&... args) : std::vector<Point2d<T>>(std::forward<Construct>(args)...) {/*initialize();*/}

    inline double getSignedArea() const {return signedArea;}
    inline double getPerimeter() const {return perimeter;}
    inline Point2d<T> getCentroid() const {return centroid;}

    void forceValid()
    {
        if (!isValid()) push_back(this->at(0));
    };

    inline bool isValid() const
    {
        return (*(this->begin()) == *(this->rbegin()));
    };

private:
    void initialize()
    {
        updateCentroid();
        updateSignedArea();
    };

    void updateCentroid();
    void updateSignedArea();
    void updatePerimeter();

    bool isUpdated;
    Point2d<T> centroid;
    double signedArea;
    double perimeter;

template<typename Ts>
friend bool operator== (const LinearRing<Ts>& l1, const LinearRing<Ts>& l2);

template<typename Ts>
friend bool operator!= (const LinearRing<Ts>& l1, const LinearRing<Ts>& l2);
};


template<typename T>
class Polygon : public std::vector<LinearRing<T>>
{
public:
    Polygon(std::initializer_list<LinearRing<T>> rings) : std::vector<LinearRing<T>>(std::move(rings)) {}
    
    template<class... Arg>
    Polygon(Arg&&... args) : std::vector<LinearRing<T>>(std::forward<Arg>(args)...) {}

    const LinearRing<T>& getHull() const 
    {
        return this->at(0);
    }

    std::vector<LinearRing<T>&> getHoles()
    {
        return std::vector<LinearRing<T>&> (this->begin() + 1, this->end());
    }

    inline double getSignedArea() const {return signedArea;}
    inline double getPerimeter() const {return perimeter;}
    inline Point2d<T> getCentroid() const {return centroid;}

    std::unordered_map<Bounds, Point2d<T>> getBoundingBox();

private:
    void updateSignedArea();
    void updatePerimeter();
    void updateCentroid();

    double signedArea;
    double perimeter;
    Point2d<T> centroid;

template<typename Ts>
friend bool operator== (const Polygon<Ts>& p1, const Polygon<Ts>& p2);

template<typename Ts>
friend bool operator!= (const Polygon<Ts>& p1, const Polygon<Ts>& p2);
};


template<typename T>
class MultiPolygon : public std::vector<Polygon<T>>
{
public:
    MultiPolygon(std::initializer_list<const Polygon<T>> polys) : std::vector<Polygon<T>>(std::move(polys)) {}

    template<class... Arg>
    MultiPolygon(Arg&&... args) : std::vector<Polygon<T>>(std::forward<Arg>(args)...) {}

    double getPerimeter();
    double getSignedArea();
    Point2d<T> getCentroid();

    std::unordered_map<Bounds, Point2d<T>> getBoundingBox();

private:
    Point2d<T> centroid;

template<typename Tl>
friend bool operator== (const MultiPolygon<Tl>& s1, const MultiPolygon<Tl>& s2);

template<typename Tl>
friend bool operator!= (const MultiPolygon<Tl>& s1, const MultiPolygon<Tl>& s2);
};


enum class ClipType {UNION, INTERSECTION, DIFFERENCE, XOR};
enum class PolyFillType {EVENODD, NONZERO, POSITIVE, NEGATIVE};
enum class PolyType {SUBJ, CLIP};

typedef signed long long ClipperCoord;

/**
 * A buffer for performing clipping operations on Polygons.
 * Assumes all linear rings are closed.
 */
class ClipperBuffer
{
public:
    ClipperBuffer() {}

    /**
     * Adds a linear ring to the buffer.
     * This can either be a subject, or a clipping object.
     *
     * @param ring A linear ring
     * @param type A PolyType (subject or clip)
     */
    void addLinearRing(const LinearRing<ClipperCoord>& ring, PolyType polyType);

    /**
     * Adds a polygon to the buffer.
     * This can either be a subject, or a clipping object.
     *
     * @param polygon A polygon
     * @param type A PolyType (subject or clip)
     */
    void addPolygon(const Polygon<ClipperCoord>& polygon, PolyType polyType);

    /**
     * Performs a clipping operation on the shapes in the buffer.
     *
     * @param clipType The clipping operation to perform.
     * @param solution Reference to a 2D vector of coords where the
     *                 result of the operation will be stored.
     * @param subjFillType The polygon filling method of the subjects.
     * @param clipFillType The polygon filling method of the clip objects.
     */
    void performClip(ClipType clipType, ClipperLib::Paths& solution, PolyFillType subjFillType, PolyFillType clipFillType);

private:
    ClipperLib::Clipper clipper;

    /**
     * Converts hte::LinearRing to ClipperLib::Path.
     *
     * @param ring A closed linear ring.
     */
    static ClipperLib::Path LinearRingToPath(const LinearRing<ClipperCoord>& ring);
};


// double GetSlope(Segment s);

template<typename T>
double GetDistance(const Point2d<T>& c0, const Point2d<T>& c1);

/**
 * Returns whether or not two polygons are bordering.
 * Uses clipping, and polygons that are bordering by a single point
 * are not considered bordering by this function.
 *
 * @param a A polygon.
 * @param b A polygon.
 */
bool GetBordering(const Polygon<ClipperCoord>& a, const Polygon<ClipperCoord>& b);
// bool GetBoundOverlap(BoundingBox, BoundingBox);
// bool GetBoundInside(BoundingBox, BoundingBox);
// bool GetPointInRing(Point2d, LinearRing);
// bool GetInside(LinearRing, LinearRing);
// bool GetInsideFirst(LinearRing s0, LinearRing s1);
// bool GetPointInCircle(Point2d center, double radius, Point2d point);

// double GetDistance(Point2d c1, Point2d c2);
// double GetDistance(Segment s);

// Polygon GetNgon(Point2d center, double radius, int nSides);
// MultiPolygon GenerateExteriorBorder(PrecinctGroup pg);

// Segment CoordsToSegment(Point2d c1, Point2d c2);
// LinearRing PathToRing(ClipperLib::Path path);
// BoostPolygon RingToBoostPoly(LinearRing);
// MultiPolygon PathsToMultiPolygon(ClipperLib::Paths paths);

// ClipperLib::Path RingToPath(LinearRing ring);
// ClipperLib::Paths PolygonToPaths(Polygon shape);
}

#endif

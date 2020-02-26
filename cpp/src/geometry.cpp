/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:               Wed, Feb 19
 
 Definition of useful functions for
 computational geometry. Basic 
 calculation, area, bordering - no
 algorithmic specific methods.
========================================*/

#include "../include/geometry.hpp"
#include "../include/shape.hpp"   // class definitions
#include "../include/gui.hpp"     // for the draw function
#include <chrono>

// define geometric constants
#define PI M_PI
const int c = pow(10, 18);

using namespace GeoGerry;
using std::vector;
using std::array;
using std::string;
using namespace std::chrono;

/*
    Following methods utilize the segment of segments typedefs.
    These are useful for summing distances, or checking colinearity,
    which is used for below border functions.
*/

segment coords_to_seg(coordinate c1, coordinate c2) {
    // combines coordinates into a segment array
    segment s = {{c1[0], c1[1], c2[0], c2[1]}};
    return s;
}

double get_distance(segment s) {
    // Distance formula on a segment array
    return sqrt(pow((s[2] - s[0]), 2) + pow((s[3] - s[1]), 2));
}

double get_distance(coordinate c0, coordinate c1) {
    // Distance formula on two separate points
    return sqrt(pow((c1[0] - c0[0]), 2) + pow((c1[1] - c0[1]), 2));
}

vector<double> calculate_line(segment s) {
    /*
        Use slope/intercept form and substituting
        coordinates in order to determine equation
        of a line segment
    */

    double m = (s[3] - s[1]) / (s[2] - s[0]);
    double b = -m * s[0] + s[1];

    return {m, b};
}

bool get_colinear(segment s0, segment s1) {
    // returns whether or not two lines have the same equation
    return (calculate_line(s0) == calculate_line(s1));
}

bool get_overlap(segment s0, segment s1) {
    /*
        Returns whether or not two segments' bounding boxes
        overlap, meaning one of the extremes of a segment are
        within the range of the other's. One shared point does
        not count to overlap.
    */
    
    if (s0[0] > s0[2])
        return (
                ((s1[0] < s0[0]) && (s1[0] > s0[2]))
                || ((s1[2] < s0[0]) && (s1[2] > s0[2])) 
               );
    else
        return (
                ((s1[0] < s0[2]) && (s1[0] > s0[0]))
                || ((s1[2] < s0[2]) && (s1[2] > s0[0]))
               );
}

bool get_bordering(segment s0, segment s1) {
    return (get_colinear(s0, s1) && get_overlap(s0, s1));
}

segments GeoGerry::LinearRing::get_segments() {
    /*
        Returns a vector of segments from the
        coordinate array of a LinearRing.border property
    */

    segments segs;
    
    // loop through segments
    if (border[border.size() - 1] == border[0])
        border.pop_back();

    for (int i = 0; i < border.size(); i++) {
        coordinate c1 = border[i];  // starting coord
        coordinate c2;              // ending coord

        // find position of ending coordinate
        if (i == border.size() - 1)
            c2 = border[0];
        else
            c2 = border[i + 1];

        segs.push_back(coords_to_seg(c1, c2)); // add to list
    }

    return segs;
}

coordinate GeoGerry::LinearRing::get_center() {
    /* 
        Returns the average {x,y} of a linear ring (set of points).
        In the future, could use centroid algorithm for determining
        center - may be a better measure of center
    */

    coordinate centroid = {0, 0};

    for (int i = 0; i < border.size() - 1; i++) {
        double x0 = border[i][0];
        double y0 = border[i][1];
        double x1 = border[i + 1][0];
        double y1 = border[i + 1][1];

        double factor = ((x0 * y1) - (x1 * y0));
        centroid[0] += (x0 + x1) * factor;
        centroid[1] += (y0 + y1) * factor;
    }

    centroid[0] /= (6 * this->get_area());
    centroid[1] /= (6 * this->get_area());

    return centroid;
}

double GeoGerry::LinearRing::get_area() {
    /*
        Returns the area of a linear ring, using latitude * long area
        An implementation of the shoelace theorem, found at 
        https://www.mathopenref.com/coordpolygonarea.html
    */

    double area = 0;
    int points = border.size() - 1; // last index of border

    for ( int i = 0; i < border.size(); i++ ) {
        area += (border[points][0] + border[i][0]) * (border[points][1] - border[i][1]);
        points = i;
    }

    return (area / 2);
}

double GeoGerry::LinearRing::get_perimeter() {
    /*
        Returns the perimeter of a LinearRing object using
        latitude and longitude coordinates by summing distance
        formula distances for all segments
    */

    double t = 0;
    for (segment s : get_segments())
        t += get_distance(s);    

    return t;
}

coordinate GeoGerry::Shape::get_center() {
    /*
        Returns average center from list of holes
        and hull by calling LinearRing::get_center.
    */

    coordinate center = hull.get_center();
    
    for (GeoGerry::LinearRing lr : holes) {
        coordinate nc = lr.get_center();
        center[0] += nc[0];
        center[1] += nc[1];
    }

    int size = 1 + holes.size();
    return {center[0] / size, center[1] / size};
}

double GeoGerry::Shape::get_area() {
    /*
        Returns the area of the hull of a shape
        minus the combined area of any holes
    */

    double area = hull.get_area();
    for (GeoGerry::LinearRing h : holes) {
        area -= h.get_area();
    }

    return area;
}


double GeoGerry::Shape::get_perimeter() {
    /*
        Returns the sum perimeter of all LinearRings
        in a shape object, including holes
    */

    double perimeter = hull.get_perimeter();
    for (GeoGerry::LinearRing h : holes) {
        perimeter += h.get_perimeter();
    }

    return perimeter;
}

double GeoGerry::Multi_Shape::get_perimeter() {
    /*
        Returns sum perimeter of a multi shape object
        by looping through each shape and calling method
    */

    double p = 0;
    for (Shape shape : border) {
        p += shape.get_perimeter();
    }

    return p;
}

double Multi_Shape::get_area() {
    /*
        Returns sum area of all Shape objects within
        border data member as double type.
    */

    double total = 0;
    for (Shape s : border)
        total += s.get_area();

    return total;
}

segments GeoGerry::Shape::get_segments() {
    // return a segment list with shape's segments, including holes
    segments segs = hull.get_segments();
    for (GeoGerry::LinearRing h : holes) {
        segments hole_segs = h.get_segments();
        for (segment s : hole_segs) {
            segs.push_back(s);
        }
    }

    return segs;
}

bool get_bordering(Shape s0, Shape s1) {
    // returns whether or not two shapes touch each other

    for (segment seg0 : s0.get_segments()) {
        for (segment seg1 : s1.get_segments()) {
            if (calculate_line(seg0) == calculate_line(seg1) && get_overlap(seg0, seg1)) {
                return true;
            }
        }
    }

    return false;
}

bool point_in_ring(GeoGerry::coordinate coord, GeoGerry::LinearRing lr) {
    /*
        returns whether or not a point is in a ring using
        the ray intersection method - counts number of times
        a ray hits the polygon

        see the documentation for this implementation at
        http://geomalgorithms.com/a03-_inclusion.html.
    */

    // int cn = 0;

    std::cout << duration.count() << std::endl;

    return t;
    // ClipperLib::Path path = ring_to_path(lr);
    // ClipperLib::IntPoint p(coord[0] * c, coord[1] * c);
    // int ret = ClipperLib::PointInPolygon(p, path);

    // return (ret != 0);
    // loop through all edges of the polygon
    // for (segment seg : lr.get_segments()) {

    //    if (((seg[1] <= coord[1]) && (seg[3] > coord[1])) ||  // an upward crossing
    //        ((seg[1] > coord[1]) && (seg[3] <= coord[1]))) {  // a downward crossing
    //         double vt = (double)(coord[1]  - seg[1]) / (seg[3] - seg[1]);
    //         if (coord[0] < seg[0] + vt * (seg[2] - seg[0])) // coord[0] < intersect
    //             ++cn;   // a valid crossing of y = coord[1] right of coord[0]
    //     }
    // }

    // return (cn & 1);    // 0 if even (out), and 1 if  odd (in)
}

bool get_inside(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    /*
        returns whether or not s0 is inside of 
        s1 using the intersection point method
    */

    for (coordinate c : s0.border)
        if (!point_in_ring(c, s1)) return false;

    return true;
}

bool get_inside_first(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    /*
        returns whether or not s0 is inside of 
        s1 using the intersection point method
    */

    return (point_in_ring(s0.border[0], s1));
}

bool get_inside_or(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    for (coordinate c : s0.border)
        if (point_in_ring(c, s1)) return true;
    return false;
}

p_index_set get_inner_boundary_precincts(Precinct_Group shape) {
   
    /*
        returns an array of indices that correspond
        to precincts on the inner edge of a Precinct Group
    */

    p_index_set boundary_precincts;
    Multi_Shape exterior_border = generate_exterior_border(shape);
    
    int i = 0;
    
    for (Precinct p : shape.precincts) {
        if (get_bordering(p, exterior_border)) {
            boundary_precincts.push_back(i);
        }
        i++;
    }

    return boundary_precincts;
}



p_index_set get_bordering_shapes(vector<Shape> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if ( ( shapes[i] != shape ) && get_bordering(shapes[i], shape)) vec.push_back(i);
    
    return vec;
}

p_index_set get_bordering_shapes(vector<Precinct_Group> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if ( ( shapes[i] != shape ) && get_bordering(shapes[i], shape)) vec.push_back(i);
    
    return vec;
}

p_index_set get_bordering_shapes(vector<Community> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if ( ( shapes[i] != shape ) && get_bordering(shapes[i], shape)) vec.push_back(i);
    
    return vec;
}

p_index_set get_bordering_precincts(Precinct_Group shape, int p_index) {
    return {1};
}

unit_interval Shape::get_compactness() {
    /*
        An implementation of the Schwartzberg compactness score.
        Returns the ratio of the perimeter of a shape to the
        circumference of a circle with the same area as that shape.
    */

    double circle_radius = sqrt(this->get_area() / PI);
    double circumference = 2 * circle_radius * PI;
    std::cout << this->get_area() << ", " << circumference << ", " << this->get_perimeter();
    return (circumference / this->get_perimeter());
}

unit_interval Multi_Shape::get_compactness() {
    /*
        An implementation of the Schwartzberg compactness score.
        Returns the ratio of the perimeter of a shape to the
        circumference of a circle with the same area as that shape.
    */

    double circle_radius = sqrt(this->get_area() / PI);
    double circumference = 2 * circle_radius * PI;
    std::cout << this->get_area() << ", " << circumference << ", " << this->get_perimeter() << std::endl;
    return (circumference / this->get_perimeter());
}

double get_standard_deviation_partisanship(Precinct_Group pg) {
    /*
        Returns the standard deviation of the partisanship
        ratio for a given array of precincts
    */

    vector<Precinct> p = pg.precincts;
    double mean = p[0].get_ratio();

    for (int i = 1; i < pg.precincts.size(); i++)
        mean += p[i].get_ratio();

    mean /= p.size();
    double dev_mean = pow(p[0].get_ratio() - mean, 2);

    for (int i = 1; i < p.size(); i++)
        dev_mean += pow(p[i].get_ratio() - mean, 2);

    return (sqrt(dev_mean));
}

double get_median_partisanship(Precinct_Group pg) {
    /*
        Returns the median partisanship ratio
        for a given array of precincts
    */

    double median;
    vector<double> ratios;
    int s = pg.precincts.size();
    
    // get array of ratios
    for (Precinct p : pg.precincts)
        ratios.push_back(p.get_ratio());
    sort(ratios.begin(), ratios.end()); // sort array

    // get median from array of ratios
    if (s % 2 == 0)
        median = (ratios[(s - 1) / 2] + ratios[s / 2]) / 2.0;
    else
        median = ratios[s / 2];

    return median;
}

Multi_Shape generate_exterior_border(Precinct_Group precinct_group) {
    /*
        Get the exterior border of a shape with interior components.
        Equivalent to 'dissolve' in mapshaper - remove bordering edges
    */ 

	ClipperLib::Paths subj;

    for (Precinct p : precinct_group.precincts) {
        for (ClipperLib::Path path : shape_to_paths(p)) {
            subj.push_back(path);
        }
    }

    // Paths solutions
    ClipperLib::Paths solutions;
    ClipperLib::Clipper c;

    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

    return paths_to_multi_shape(solutions);
    // return clipper_mult_int_to_shape(solutions);
}

p_index State::get_addable_precinct(p_index_set available_precincts, p_index current_precinct) {
    /*
        A method for the initial generation of the
        communities algorithm - returns the next addable
        precinct to a given community, to avoid creating islands
    */

    p_index ret;
    return ret;
}

ClipperLib::Path ring_to_path(GeoGerry::LinearRing ring) {
    /*
        Creates a clipper Path object from a
        given Shape object by looping through points
    */

    ClipperLib::Path p(ring.border.size());

    for (coordinate point : ring.border ) {
        p << ClipperLib::IntPoint(point[0] * c, point[1] * c);
    }

    return p;
}

GeoGerry::LinearRing path_to_ring(ClipperLib::Path path) {
    /*
        Creates a shape object from a clipper Path
        object by looping through points
    */

    GeoGerry::LinearRing s;

    for (ClipperLib::IntPoint point : path ) {
        coordinate p = {(static_cast<long double>(point.X) / c), (static_cast<long double>(point.Y) / c)};
        if (p[0] != 0 && p[1] != 0) s.border.push_back(p);
    }

    return s;
}

ClipperLib::Paths shape_to_paths(GeoGerry::Shape shape) {
    ClipperLib::Paths p;
    p.push_back(ring_to_path(shape.hull));
    
    for (GeoGerry::LinearRing ring : shape.holes) {
        ClipperLib::Path path = ring_to_path(ring);
        ReversePath(path);
        p.push_back(path);
    }

    return p;
}

GeoGerry::Multi_Shape paths_to_multi_shape(ClipperLib::Paths paths) {
    /*
        Create a Multi_Shape object from a clipper Paths
        (multi path) object through nested iteration
    */

    Multi_Shape ms;
    
    for (ClipperLib::Path path : paths) {
        if (ClipperLib::Orientation(path)) {
            GeoGerry::LinearRing border = path_to_ring(path);
            GeoGerry::Shape s(border);
            ms.border.push_back(s);
        }
        // else {
        //     std::cout << "hole" << std::endl;
        //     ReversePath(path);
        //     GeoGerry::LinearRing hole = path_to_ring(path);
        //     ms.border[ms.border.size() - 1].holes.push_back(hole);
        // }
    }

    return ms;
}

Multi_Shape poly_tree_to_shape(ClipperLib::PolyTree tree) {
    /*
        Loops through top-level children of a
        PolyTree to access outer-level polygons. Returns
        a multi_shape object containing these outer polys.
    */
   
    Multi_Shape ms;
    
    for (ClipperLib::PolyNode* polynode : tree.Childs) {
        // if (polynode->IsHole()) x++;
        LinearRing s = path_to_ring(polynode->Contour);
        Shape shape(s);
        ms.border.push_back(shape);
    }

    return ms;
}

LinearRing* create_linearring(coordinate_set coords) {
    // We will use a coordinate list to build the linearring
    CoordinateArraySequence* cl = new CoordinateArraySequence();

    for (coordinate c : coords) {
        cl->add(Coordinate(c[0], c[1]));
    }

    LinearRing* lr = global_factory->createLinearRing(cl);
    return lr; // our LinearRing
}

Point* create_point(double x, double y) {
    /* given coordinates creates a point object */
    Coordinate c(x, y);
    Point* p = global_factory->createPoint(c);
    return p;
}

Geometry* shape_to_poly(Shape shape) {
    /*
        Creates a GEOS library polygon object from a
        given Shape object by looping through points
    */
   cout << "b" << endl;

    // blank holes vector
    vector<LinearRing*>* holes = new vector<LinearRing*>;
       cout << "b" << endl;

    // outer ring generated by looping through coordinates
    LinearRing* outer = create_linearring(shape.border);
       cout << "b" << endl;

    Polygon* poly = global_factory->createPolygon(outer, holes);
   cout << "b" << endl;

    return poly;
}

Shape poly_to_shape(const Geometry* path) {
    /*
        Creates a shape object from a clipper Path
        object by looping through points
    */

    Shape s;
    
    // write coordinates in path to vector of Coordinates
    std::unique_ptr <CoordinateSequence> points = path->getCoordinates();
    vector<Coordinate> coords;
    points->toVector(coords);
    
    for (Coordinate coord : coords) {
        coordinate nc = {(float) coord.x, (float) coord.y};
        s.border.push_back(nc);
    }

    return s;
}

Multi_Shape* multipoly_to_shape(MultiPolygon* paths) {
    /*
        Create a Multi_Shape object from a clipper Paths
        (multi path) object through nested iteration
    */

    Multi_Shape* ms;
    
    return ms;
}
// boost_polygon ring_to_boost_poly(LinearRing shape) {

//     /*
//         Converts a shape object into a boost polygon object
//         by looping over each point and manually adding it to a 
//         boost polygon using assign_points and vectors
//     */

//     boost_polygon poly;

//     // create vector of boost points
//     std::vector<boost_point> points;

//     for (coordinate c : shape.border) 
//         points.push_back(boost_point(c[0],c[1])),

//     assign_points(poly, points);
//     correct(poly);

//     return poly;
// }

// LinearRing boost_poly_to_ring(boost_polygon poly) {

//     /*
//         Convert from a boost polygon into a Shape object.
//         Loop over each point in the polygon, add it to the
//         shape's border.
//     */

//     coordinate_set b;
//     vector<boost_point> const& points = poly.outer();

//     for (std::vector<boost_point>::size_type i = 0; i < points.size(); ++i)
//         b.push_back({get<0>(points[i]), get<1>(points[i])});

//     LinearRing shape(b);
//     return shape;
// }

// Shape boost_poly_to_shape(boost_multi_polygon poly) {

//     /*
//         overload the poly_to_shape function to work with multipolygons
//         loop over each polygon, use same process to add to shape's border
//         !!! WARNING: Should update shape data structure to work with
//             multipolygon set. Not quite sure how to do this yet...
//     */

//     coordinate_set b;

//     for (boost_polygon p : poly) {
//         vector<boost_point> const& points = p.outer();

//         for (std::vector<boost_point>::size_type i = 0; i < points.size(); ++i) {
//             b.push_back({(float) get<0>(points[i]), (float) get<1>(points[i])});
//         }
//     }

//     Shape shape(b);
//     return shape;
// }
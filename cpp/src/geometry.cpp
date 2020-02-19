/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:               Sun, Jan 19
 
 Definition of useful functions for
 computational geometry. Basic 
 calculation, area, bordering - no
 algorithmic specific methods.
========================================*/

#include "../include/geometry.hpp"
#include "../include/shape.hpp"   // class definitions
#include "../include/gui.hpp"     // for the draw function

// define geometric constants
#define PI M_PI
const int c = pow(10, 7);

using namespace GeoGerry;
using std::vector;
using std::array;
using std::string;

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

    coordinate coords = { border[0][0], border[0][1] };  // initialize with first values
    
    // loop and add x and y to respective sums
    for ( int i = 1; i < border.size(); i++ ) {
        coords[0] += border[i][0];
        coords[1] += border[i][1];
    }

    coordinate center = {{(coords[0] / border.size()), (coords[1] / border.size())}};
    return center; // return averages
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

    float t = 0;
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

        Need to document htis fucntion later as I'm not
        quite sure how it works - pulled this one from python
    */

    int intersections = 0;
    segments segs = lr.get_segments();

    for (segment s : segs) {
        if ((s[0] < coord[0] && s[2] < coord[0]) ||
            (s[0] > coord[0] && s[2] > coord[0]) ||
            (s[1] < coord[1] && s[3] < coord[1]))
            continue;

        if (s[1] > coord[1] && s[3] > coord[1]) {
            intersections += 1;
            continue;
        }
        else {
            vector<double> eq = calculate_line(s);
            double y_c = eq[0] * coord[0] + eq[1];
            if (y_c > coord[1]) intersections += 1;
        }
    }

    return (intersections % 2 == 1); // odd intersection
}

bool get_inside(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    /*
        returns whether or not s0 is inside of 
        s1 using the intersection point method
    */

    return point_in_ring(s0.border[0], s1);
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


p_index_set get_bordering_precincts(Precinct_Group shape, int p_index) {
    return {1};
}

unit_interval compactness(Shape shape) {

    /*
        An implementation of the Schwartzberg compactness score.
        Returns the ratio of the perimeter of a shape to the
        circumference of a circle with the same area as that shape.
    */

    double circle_radius = sqrt(shape.get_area() / PI);
    double circumference = 2 * circle_radius * PI;

    return (circumference / shape.get_perimeter());
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

    for (Precinct p : precinct_group.precincts)
        subj.push_back(shape_to_path(p));
    // Paths solutions
    ClipperLib::Paths solutions;
    ClipperLib::Clipper c;
	
    c.AddPaths(subj, ClipperLib::ptSubject, false);
    c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

    return paths_to_shape(solutions);
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

ClipperLib::Path shape_to_path(Shape shape) {
    /*
        Creates a clipper Path object from a
        given Shape object by looping through points
    */

    ClipperLib::Path p(shape.border.size());

    for (coordinate point : shape.border ) {
        p << ClipperLib::IntPoint(point[0] * c, point[1] * c);
    }

    return p;
}

Shape path_to_shape(ClipperLib::Path path) {
    /*
        Creates a shape object from a clipper Path
        object by looping through points
    */

    Shape s;

    for (ClipperLib::IntPoint point : path ) {
        coordinate p = {(float)((float)point.X / (float)c), (float)((float)point.Y / (float) c)};
        s.border.push_back(p);
    }

    return s;
}

Multi_Shape paths_to_shape(ClipperLib::Paths paths) {
    /*
        Create a Multi_Shape object from a clipper Paths
        (multi path) object through nested iteration
    */

    //! ERROR: THIS DOES NOT WORK RIGHT NOW, 
    // WHY IS CLIPPER RETURNING SO MANY POLYS?

    Multi_Shape ms;
    
    for (ClipperLib::Path p : paths) {
        coordinate_set border;

        for (ClipperLib::IntPoint point : p) {
            coordinate coord = {(float)((float)point.X / (float)c), (float)((float)point.Y / (float) c)};
            if (!(coord[0] == 0 || coord[1] == 0)) border.push_back(coord);
        }

        Shape s(border);
        ms.border.push_back(s);
    }

    return ms;
}


// Multi_Shape poly_tree_to_shape(PolyTree tree) {
//     /*
//         Loops through top-level children of a
//         PolyTree to access outer-level polygons. Returns
//         a multi_shape object containing these outer polys.
//     */
   
//     Multi_Shape ms;
    
//     for (PolyNode* polynode : tree.Childs) {
//         if (polynode->IsHole()) x++;
//         Shape s = clipper_int_to_shape(polynode->Contour);
//         ms.border.push_back(s);
//     }

//     return ms;
// }
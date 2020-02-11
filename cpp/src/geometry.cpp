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


#define PI M_PI

coordinate Shape::center() {
    // returns the average {x,y} of a shape
    coordinate coords = { border[0][0], border[0][1] };  // initialize with first values
    
    // loop and add x and y to respective sums
    for ( int i = 1; i < border.size(); i++ ) {
        coords[0] += border[i][0];
        coords[1] += border[i][1];
    }

    // divide by number of elements to average
    coords[0] /= border.size();
    coords[1] /= border.size();

    return coords; // return averages
}

float get_distance(segment s) {
    // Distance formula on a segment array
    return sqrt(pow((s[2] - s[0]), 2) + pow((s[3] - s[1]), 2));
}

float get_distance(coordinate c0, coordinate c1) {
    // Distance formula on two separate points
    return sqrt(pow((c1[0] - c0[0]), 2) + pow((c1[1] - c0[1]), 2));
}

float Shape::area() {
    
    /*
        returns the area of a shape, using latitude * long area
    */

    float area = 0;
    int points = border.size() - 1; // last index of border

    for ( int i = 0; i < border.size(); i++ ) {
        area += (border[points][0] + border[i][0]) * (border[points][1] - border[i][1]);
        points = i;
    }

    return (area / 2);
}

float Shape::perimeter() {
    /*
        returns the perimeter of a shape object
        using latitude and longitude coordinates
    */

    float t = 0;
    for (segment s : get_segments())
        t += get_distance(s);    

    return t;
}

segment coords_to_seg(coordinate c1, coordinate c2) {
    // combines coordinates into a segment array
    return {c1[0], c1[1], c2[0], c2[1]};
}

segments Shape::get_segments() {
    segments segs;
    
    for (int i = 0; i < border.size(); i++) {

        coordinate c1 = border[i];
        coordinate c2;

        if (i == border.size() - 1)
            c2 = border[0];
        else
            c2 = border[i + 1];

        segs.push_back(coords_to_seg(c1, c2));
    }

    return segs;
}

vector<float> calculate_line(segment s) {
    float m = (s[3] - s[1]) / (s[2] - s[0]);
    float b = -m * s[0] + s[1];

    return {m, b};
}

bool segments_overlap(segment s0, segment s1) {
    if (s0[0] > s0[2])
        return (((s1[0] < s0[0]) && (s1[0] > s0[2])) || ((s1[2] < s0[0]) && (s1[2] > s0[2])) );
    else
        return (((s1[0] < s0[2]) && (s1[0] > s0[0])) || ((s1[2] < s0[2]) && (s1[2] > s0[0])) );
}

bool are_bordering(Shape s0, Shape s1) {
    // returns whether or not two shapes touch each other

    for (segment seg0 : s0.get_segments()) {
        for (segment seg1 : s1.get_segments()) {
            if (calculate_line(seg0) == calculate_line(seg1) && segments_overlap(seg0, seg1)) {
                return true;
            }
        }
    }

    return false;
}

p_index_set get_inner_boundary_precincts(Precinct_Group shape) {
   
    /*
        returns an array of indices that correspond
        to precincts on the inner edge of a Precinct Group
    */

    p_index_set boundary_precincts;
    Shape exterior_border = generate_exterior_border(shape);
    
    int i = 0;
    
    for (Precinct p : shape.precincts) {
        if (are_bordering(p, exterior_border)) {
            boundary_precincts.push_back(i);
        }
        i++;
    }

    return boundary_precincts;
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

    float circle_radius = sqrt(shape.area() / PI);
    float circumference = 2 * circle_radius * PI;

    return 1/(shape.perimeter() / circumference);
}

bool is_colinear(segment s0, segment s1) {
    return (calculate_line(s0) == calculate_line(s1));
}

Shape generate_exterior_border(Precinct_Group precinct_group) {

    /*
        Get the exterior border of a shape with interior components.
        Equivalent to 'dissolve' in mapshaper - remove bordering edges
    */

    Shape ext_border; 

    boost_multi_polygon border;

    for (Precinct p : precinct_group.precincts) {
        boost_polygon p_poly = shape_to_poly(p);
        boost_multi_polygon tmp_poly;
        union_(border, p_poly, tmp_poly);
        border = tmp_poly;
    }

    ext_border = boost_poly_to_shape(border);
    return ext_border;
}

p_index State::get_addable_precinct(p_index_set available_precincts, p_index current_precinct) {
    p_index ret;
    return ret;
}

boost_polygon shape_to_poly(Shape shape) {
       
    /*
        Converts a shape object into a boost polygon object
        by looping over each point and manually adding it to a 
        boost polygon using assign_points and vectors
    */

    boost_polygon poly;

    // create vector of boost points
    vector<boost_point> points;
    for (coordinate c : shape.border) 
        points.push_back(boost_point(c[0],c[1])),

    assign_points(poly, shape.border);
    correct(poly);

    return poly;
}

Shape boost_poly_to_shape(boost_polygon poly) {
       
    /*
        Convert from a boost polygon into a Shape object.
        Loop over each point in the polygon, add it to the
        shape's border.
    */

    coordinate_set b;
    vector<boost_point> const& points = poly.outer();
    
    for (std::vector<boost_point>::size_type i = 0; i < points.size(); ++i) {
        b.push_back({(float) get<0>(points[i]), (float) get<1>(points[i])});
    }

    Shape shape(b);
    return shape;
}

Shape boost_poly_to_shape(boost_multi_polygon poly) {
   
    /*
        overload the poly_to_shape function to work with multipolygons
        loop over each polygon, use same process to add to shape's border

        !!! WARNING: Should update shape data structure to work with
            multipolygon set. Not quite sure how to do this yet...
    */

    coordinate_set b;

    for (boost_polygon p : poly) {
        vector<boost_point> const& points = p.outer();

        for (std::vector<boost_point>::size_type i = 0; i < points.size(); ++i) {
            b.push_back({(float) get<0>(points[i]), (float) get<1>(points[i])});
        }
    }

    Shape shape(b);
    return shape;
}

void print_shape(Shape shape) {
    cout << dsv(shape_to_poly(shape)) << endl;
}

void print_shape(boost_polygon poly) {
    cout << dsv(poly) << endl;
}
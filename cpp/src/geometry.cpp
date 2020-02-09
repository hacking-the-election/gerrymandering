/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:               Sun, Jan 19
 
 Definition of methods for shapes, 
 precincts, districts and states. Basic
 calculation, area, and drawing - no
 algorithmic specific methods.
========================================*/

#include "../include/shape.hpp"   // class definitions
#include "../include/gui.hpp"     // for the draw function
#include <math.h>                 // for rounding functions
#include <cmath>

#define PI M_PI

vector<float> calculate_line(coordinate c0, coordinate c1) {
    float m = (c1[1] - c0[1]) / (c1[0] - c0[0]);
    float b = -m * c0[0] + c0[1];

    return {m, b};
}

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

    for (int i = 0; i < border.size(); i++) {
        // get pair of coordinates on
        // which to apply distance formula
        coordinate c0 = border[i];
        coordinate c1;

        if (i == border.size() - 1) c1 = border[0];
        else c1 = border[i + 1];

        float d = sqrt(pow((c1[0] - c0[0]), 2) + pow((c1[1] - c0[1]), 2));
        t += d;
    }

    return t;
}

bounding_box normalize_coordinates(Shape* shape) {

    /*
        returns a normalized bounding box, and modifies 
        shape object's coordinates to move it to Quadrant I
    */

    // set dummy extremes
    float top = shape->border[0][1], 
        bottom = shape->border[0][1], 
        left = shape->border[0][0], 
        right = shape->border[0][0];

    // loop through and find actual corner using ternary assignment
    for (vector<float> coord : shape->border) {
        top = (coord[1] > top)? coord[1] : top;
        bottom = (coord[1] < bottom)? coord[1] : bottom;
        left = (coord[0] < left)? coord[0] : left;
        right = (coord[0] > right)? coord[0] : right;
    }

    // add to each coordinate to move it to quadrant 1
    for (float i = 0; i < shape->border.size(); i++) {
        shape->border[i][0] += (0 - left);
        shape->border[i][1] += (0 - bottom);
    }

    // normalize the bounding box too
    top += (0 - bottom);
    right += (0 - left);
    bottom = 0;
    left = 0;

    return {top, bottom, left, right}; // return bounding box
}

coordinate_set resize_coordinates(bounding_box box, coordinate_set shape, int screenX, int screenY) {
    // scales an array of coordinates to fit 
    // on a screen of dimensions {screenX, screenY}
    
    float ratioTop = ceil((float) box[0]) / (float) (screenX);   // the rounded ratio of top:top
    float ratioRight = ceil((float) box[3]) / (float) (screenY); // the rounded ratio of side:side
    
    // find out which is larger and assign it's reciporical to the scale factor
    float scaleFactor = floor(1 / ((ratioTop > ratioRight) ? ratioTop : ratioRight)); 

    // dilate each coordinate in the shape
    for ( int i = 0; i < shape.size(); i++ ) {
        shape[i][0] *= scaleFactor;
        shape[i][1] *= scaleFactor;        
    }

    // return scaled coordinates
    return shape;
}

p_index_set get_boundary_precincts(Precinct_Group shape) {
   
    /*
        returns an array of indices that correspond
        to precincts on the outer edge of a Precinct Group
    */

    p_index_set boundary_precincts;

    for (Precinct p0 : shape.precincts) {
        for (int i = 0; i < p0.border.size(); i++) {
            coordinate c0 = p0.border[i];
            coordinate c1;

            if (i == p0.border.size())
                c1 = p0.border[0];
            else
                c1 = p0.border[i + 1];

            vector<float> line = calculate_line(c0, c1);
            bool found_line = true;
            int index = 0;

            while ((!found_line) && (index < shape.precincts.size())) {
                Precinct p1 = shape.precincts.[index];
                for (int i = 0; i < p1.border.size(); i++) {
                    coordinate d0 = p1.border[i];
                    coordinate d1;

                    if (i == p1.border.size())
                        d1 = p1.border[0];
                    else
                        d1 = p1.border[i + 1];

                    vector<float> line_d = calculate_line(d0, d1);

                    // if (line == line_d && line_in_range(line, line_d) ) 
                        // and one point from d0 falls within
                }
                index++;
            }
        
        }
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

coordinate_set generate_exterior_border(Precinct_Group precinct_group) {
    p_index_set boundary_precincts = get_boundary_precincts(precinct_group);
    
    for (p_index i : boundary_precincts) {

        Precinct precinct = precinct_group.precincts[i];
        coordinate_set border = precinct.border;

        for (int i = 0; i < border.size(); i++) {
            
            coordinate c0 = border[i];
            coordinate c1;

            if (i == border.size())
                c1 = border[0];
            else 
                c1 = border[i + 1];
            
            // calculate_line(c0, c1);
        }
    }
}
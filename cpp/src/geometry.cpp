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

double Shape::area() {
    
    /*
        returns the area of a shape, using latitude * long area
    */

    double area = 0;
    int points = border.size() - 1; // last index of border

    for ( int i = 0; i < border.size(); i++ ) {
        area += (border[points][0] + border[i][0]) * (border[points][1] - border[i][1]);
        points = i;
    }

    return (area / 2);
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

index_set get_boundary_precincts(Precinct_Group shape) {
    return {1,3};
}

index_set get_bordering_precincts(Precinct_Group shape, int index) {

}
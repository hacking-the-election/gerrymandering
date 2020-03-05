#include "../include/canvas.hpp"

void GeoDraw::Canvas::add_shape(GeoGerry::LinearRing s) {
    /*
        @desc: Add a LinearRing object to the screen
        @params: `LinearRing` s: LinearRing object to add
        @return: void
    */

    outlines.push_back(s);
    return;
}


void GeoDraw::Canvas::add_shape(GeoGerry::Shape s) {
    /*
        @desc: Add a shape object to the screen
        @params: `Shape` s: Shape object to add
        @return: void
    */

    outlines.push_back(s.hull);
    for (GeoGerry::LinearRing l : s.holes)
        holes.push_back(l);

    return;
}


void GeoDraw::Canvas::add_shape(GeoGerry::Multi_Shape s) {
    /*
        @desc: Add a shape object to the screen
        @params: `Shape` s: Shape object to add
        @return: void
    */

    for (GeoGerry::Shape shape : s.border) {
        outlines.push_back(shape.hull);
        for (GeoGerry::LinearRing l : shape.holes) {
            holes.push_back(l);
        }
    }

    return;
}


void GeoDraw::Canvas::add_shape(GeoGerry::Precinct_Group s) {
    /*
        @desc: Add a shape object to the screen
        @params: `Shape` s: Shape object to add
        @return: void
    */

    for (GeoGerry::Precinct shape : s.precincts) {
        outlines.push_back(shape.hull);
        for (GeoGerry::LinearRing l : shape.holes) {
            holes.push_back(l);
        }
    }
}

        
void GeoDraw::Canvas::resize_window(int dx, int dy) {
    x = dx;
    y = dy;
}


GeoGerry::bounding_box GeoDraw::Canvas::get_bounding_box() {
    /*
        @desc:
            returns a bounding box of the internal list of hulls
            (because holes cannot be outside shapes)
        
        @params: none;
        @return: `bounding_box` the superior bounding box of the shape
    */

    // set dummy extremes
    int top = outlines[0].border[0][1], 
        bottom = outlines[0].border[0][1], 
        left = outlines[0].border[0][0], 
        right = outlines[0].border[0][0];

    // loop through and find actual corner using ternary assignment
    for (GeoGerry::LinearRing ring : outlines) {
        for (GeoGerry::coordinate coord : ring.border) {
            if (coord[1] > top) top = coord[1];
            if (coord[1] < bottom) bottom = coord[1];
            if (coord[0] < left) left = coord[0];
            if (coord[0] > right) right = coord[0];
        }
    }

    return {top, bottom, left, right}; // return bounding box
}


void GeoDraw::Canvas::translate(long int x, long int y) {
    /*
        @desc:
            Translates all linear rings contained in the
            canvas object by x and y
        
        @params: 
            `long int` x: x coordinate to translate
            `long int` y: y coordinate to translate
        
        @return: void
    */

    for (int i = 0; i < outlines.size(); i++) {
        for (int j = 0; j < outlines[i].border.size(); j++) {
            outlines[i].border[j][0] += x;
            outlines[i].border[j][1] += y;
        }
    }

    for (int i = 0; i < holes.size(); i++) {
        for (int j = 0; j < holes[i].border.size(); j++) {
            holes[i].border[j][0] += x;
            holes[i].border[j][1] += y;
        }
    }
}


void GeoDraw::Canvas::scale(double scale_factor) {
    /*
        @desc:
            Scales all linear rings contained in the canvas
            object by scale_factor (including holes)
        
        @params: `double` scale_factor: factor to scale coordinates by
        
        @return: void
    */

    for (int i = 0; i < outlines.size(); i++) {
        for (int j = 0; j < outlines[i].border.size(); j++) {
            outlines[i].border[j][0] += x;
            outlines[i].border[j][1] += y;
        }
    }

    for (int i = 0; i < holes.size(); i++) {
        for (int j = 0; j < holes[i].border.size(); j++) {
            holes[i].border[j][0] += x;
            holes[i].border[j][1] += y;
        }
    }
}

void GeoDraw::Canvas::rasterize_shapes() {
    /*
        @desc:
            scales an array of coordinates to fit on a screen
            of dimensions {x, y}, and determines pixel values
            and placements

        @params: none
        @return: void
    */

    double ratioTop = ceil((double) box[0]) / (double) (x);   // the rounded ratio of top:top
    double ratioRight = ceil((double) box[3]) / (double) (y); // the rounded ratio of side:side
    
    // find out which is larger and assign its reciporical to the scale factor
    double scaleFactor = floor(1 / ((ratioTop > ratioRight) ? ratioTop : ratioRight)); 

    // dilate each coordinate in the shape
    for ( int i = 0; i < shape.size(); i++ ) {
        shape[i][0] *= scaleFactor;
        shape[i][1] *= scaleFactor;        
    }

    // return scaled coordinates
    return shape;
}


void connect_shapes() {
    return;
}


void GeoDraw::Canvas::draw() {
    /*
        @desc: Prints the shapes in the canvas to the screen
        @params: none
        @return: void
    */

    // size and position coordinates in the right wuat
    GeoGerry::bounding_box box = get_bounding_box();
    translate(-box[1], -box[2]);
}
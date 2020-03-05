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

void GeoDraw::Canvas::draw() {
    
}
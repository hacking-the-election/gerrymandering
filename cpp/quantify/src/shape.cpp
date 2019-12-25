#include "../include/shape.hpp"

Shape expand_border(Shape shape) {
    vector<vector<int> > expanded;
    Shape expanded_shape = Shape(expanded);
    return expanded_shape;
}

double* center(Shape shape) {
    
    double coords[2] = { (float) shape.border[0][0], (float) shape.border[0][1] };
    
    for ( int i = 1; i < shape.border.size(); i++ ) {
        coords[0] += shape.border[i][0];
        coords[1] += shape.border[i][1];
    }

    coords[0] /= shape.border.size();
    coords[1] /= shape.border.size();

    return coords;
}

double area(Shape shape) {
    double area = 0;
    int points = shape.border.size() - 1;

    for ( int i = 0; i < shape.border.size(); i++ ) {
        area += (shape.border[points][0] + shape.border[i][0]) * (shape.border[points][1] - shape.border[i][1]);
        points = i;
    }

    return (area / 2);
}

double Precinct::get_ratio() {
    return dem / (dem + rep);
    // return rep / (dem + rep);
}
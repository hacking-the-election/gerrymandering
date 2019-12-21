#include "../include/shape.hpp"

vector<vector<int> > expand_border(vector<vector<int> > shape) {
    vector<vector<int> > expanded;
    return expanded;
}


double* center(vector<vector<int> > shape) {
    double coords[2] = { (float) shape[0][0], (float) shape[0][1] };
    
    for ( int i = 1; i < shape.size(); i++ ) {
        coords[0] += shape[i][0];
        coords[1] += shape[i][1];
    }

    coords[0] /= shape.size();
    coords[1] /= shape.size();

    return coords;
}

double area(vector<vector<int> > shape) {
    double area = 0;
    int points = shape.size() - 1;

    for ( int i = 0; i < shape.size(); i++ ) {
        area += (shape[points][0] + shape[i][0]) * (shape[points][1] - shape[i][1]);
        points = i;
    }

    return (area / 2);
}

double Precinct::get_ratio() {
    return dem / (dem + rep);
}
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

Precinct::Precinct(vector<vector<int> > shape, int demV, int repV){
    precinct_border = shape;

    dem = demV;
    rep = repV;
}


double Precinct::get_ratio() {
    return dem / (dem + rep);
}

District::District(vector<vector<int> > shape) {
    district_border = shape;
    district_border_expanded = expand_border(district_border);
}

State::State(vector<District> dists, vector<Precinct> pres, vector<vector<int> > shape) {
    state_districts = dists;
    state_border = shape;
    state_precincts = pres;
}

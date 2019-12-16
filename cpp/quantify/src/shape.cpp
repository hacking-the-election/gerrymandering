#include "../include/shape.hpp"

Precinct::Precinct(vector<int> coordinates, int demV, int repV){
    precinct_border = coordinates;

    dem = demV;
    rep = repV;
    
    dratio = dem / (dem + rep);
}

State::State(vector<District> dists, vector<Precinct> pres, vector<int> boundary) {
    state_districts = dists;
    state_border = boundary;
    state_precincts = pres;
}
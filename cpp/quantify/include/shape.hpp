#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include "../../lib/json/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

// define global variables for algorithms
//=======================================
const float EXPANSION_WIDTH = 10;
const float FAIRNESS = 0;
const float COMPACTNESS = 0;
//=======================================


// declare methods for calculating a shape
double area(vector<vector<int> > shape);
double* center(vector<vector<int> > shape);

vector<vector<int> > expand_border(vector<vector<int> > shape);


class Shape {
    public: Shape(vector<vector<int> > shape);
    vector<vector<int> > border;
};


class Precinct : public Shape {
    
    Precinct(vector<vector<int> > shape, int demV, int repV) : Shape(shape) {
        dem = demV;
        rep = repV;
    }

    int dem;
    int rep;

    public: 
        double get_ratio();
};

class District : Shape {
    District(vector<vector<int> > shape) : Shape(shape) {};

    int id;

    double quantify();
    double percent_of_precinct_in_district(Precinct precint);
};

class State : Shape {
    State(vector<District> districts, vector<Precinct> precincts, vector<vector<int> > shape) : Shape(shape) {
        state_districts = districts;
        state_precincts = precincts;
    };

    vector<District> state_districts;
    vector<Precinct> state_precincts;

    void serialize_obj();
    void deserialize_obj();

    int id;
    string name;
};
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "../../lib/rapidjson/include/rapidjson/document.h"
#include "../../lib/rapidjson/include/rapidjson/writer.h"
#include "../../lib/rapidjson/include/rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

// define global variables for algorithms
//=======================================
const int EXPANSION_WIDTH = 10;
//=======================================


// declare methods for calculating a shape
double area(vector<vector<int> > shape);
double* center(vector<vector<int> > shape);

vector<vector<int> > expand_border(vector<vector<int> > shape);


// class Shape {
//     Shape(vector<vector<int> > shape);
//     vector<vector<int> > border;
// };


class Precinct {// : Shape {
    Precinct(int demV, int repV);

    int dem;
    int rep;
    vector<vector<int> > precinct_border;

    public: 
        double get_ratio();
};

class District {
    District(vector<vector<int> > shape);

    vector<vector<int> > district_border;
    vector<vector<int> > district_border_expanded;
    int id;

    double quantify();
    double percent_of_precinct_in_district(Precinct precint);
};

class State {
    State(vector<District> dists, vector<Precinct> pres, vector<vector<int> > shape);

    vector<vector<int> > state_border;
    vector<District> state_districts;
    vector<Precinct> state_precincts;

    void serialize_obj();
    void deserialize_obj();

    int id;
    string name;
};
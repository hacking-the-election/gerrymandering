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
const float EXPANSION_WIDTH = 10;
const float FAIRNESS = 0;
const float COMPACTNESS = 0;

// function declarations
double area(Shape shape);
double* center(Shape shape);
Shape expand_border(Shape shape);
string readf(string path);


// class definitions for shapes
class Shape {
    public: 
        Shape(vector<vector<int> > shape);
        vector<vector<int> > border;
};

class Precinct : public Shape {
    
    public: 
        Precinct(vector<vector<int> > shape, int demV, int repV) : Shape(shape) {
            dem = demV;
            rep = repV;
        }

        double get_ratio();
    
    private:
        int dem;
        int rep;
};

class District : Shape {
    District(vector<vector<int> > shape) : Shape(shape) {};

    int id;

    double quantify();
    double percent_of_precinct_in_district(Precinct precint);
};

class State : Shape {
    
    public: State(vector<District> districts, vector<Precinct> precincts, vector<vector<int> > shape) : Shape(shape) {
        state_districts = districts;
        state_precincts = precincts;
    };
    
    private:
        vector<District> state_districts;
        vector<Precinct> state_precincts;
        string name;

    public:
        static State generate_from_file(string geoJSON, string voter_data);
        void serialize_obj();
        void deserialize_obj();
};
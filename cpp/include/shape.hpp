#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include "../lib/json/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

// define global variables for algorithms
const float EXPANSION_WIDTH = 10;
const float FAIRNESS = 0;
const float COMPACTNESS = 0;

const string no_name = "\e[31m[no_name]\e[0m";

// class definitions for shapes
class Shape {
    public: 
        // overload constructor for adding id
        Shape(vector<vector<float> > shape);
        Shape(vector<vector<float> > shape, string id);

        vector<vector<float> > border;
        string shape_id;

        void draw();
};

double area(Shape shape);
vector<double> center(Shape shape);
Shape expand_border(Shape shape);

class Precinct : public Shape {
    
    public: 
        Precinct(vector<vector<float> > shape, int demV, int repV) : Shape(shape) {
            dem = demV;
            rep = repV;
        }

        double get_ratio();
        vector<int> voter_data();
    
    private:
        int dem;
        int rep;
};

class District : public Shape {
    public: District(vector<vector<float> > shape) : Shape(shape) {};

    int id;

    double quantify();
    double percent_of_precinct_in_district(Precinct precint);
};

class State : Shape {
    
    public: State(vector<District> districts, vector<Precinct> precincts, vector<vector<float> > shape) : Shape(shape) {
        state_districts = districts;
        state_precincts = precincts;
    };
    
    private:
        vector<District> state_districts;
        vector<Precinct> state_precincts;
        string name = no_name;

    public:
        static State generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON);
        void write_txt();
        void serialize_obj(string write_path);
        void read_serialized_obj(string read_path);
        string to_json();
};
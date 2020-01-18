#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
// #include "../lib/json/single_include/nlohmann/json.hpp"
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;
// using json = nlohmann::json;
using namespace rapidjson;

// define global variables for algorithms
const float EXPANSION_WIDTH = 10;
const float FAIRNESS = 0;
const float COMPACTNESS = 0;

const string no_name = "no_name";

// class definitions for shapes
class Shape {
    public: 
        // overload constructor for adding id
        Shape(){};
        Shape(vector<vector<float> > shape);
        Shape(vector<vector<float> > shape, string id);

        vector<vector<float> > border;
        string shape_id;

        void draw();

        friend class boost::serialization::access;

        template<class Archive> void serialize(Archive & ar, const unsigned int version) {
            ar & shape_id;
            ar & border;
        }
};

double area(Shape shape);
vector<double> center(Shape shape);
Shape expand_border(Shape shape);

class Precinct : public Shape {
    
    public:
        Precinct(){};

        Precinct(vector<vector<float> > shape, int demV, int repV) : Shape(shape) {
            dem = demV;
            rep = repV;
        }

        Precinct(vector<vector<float> > shape, int demV, int repV, string id) : Shape(shape, id) {
            dem = demV;
            rep = repV;
        }
        double get_ratio();
        vector<int> voter_data();
    
        friend class boost::serialization::access;

        template<class Archive> void serialize(Archive & ar, const unsigned int version) {
            ar & shape_id;
            ar & border;
            ar & dem;            
            ar & rep;            
        }
        
    private:
        int dem;
        int rep;
};

class District : public Shape {
    friend class boost::serialization::access;

    template<class Archive> void serialize(Archive & ar, const unsigned int version) {
        ar & id;
        ar & border;
    }

    public: District(){};
    public: District(vector<vector<float> > shape) : Shape(shape) {};

    int id;

    double quantify();
    double percent_of_precinct_in_district(Precinct precint);
};

class State : public Shape {
    friend class boost::serialization::access;

    template<class Archive> void serialize(Archive & ar, const unsigned int version) {
        ar & state_districts;
        ar & state_precincts;
        ar & name;
        ar & border;
    }

    public: State(){};
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

        void serialize(string write_path) {
            std::ofstream ofs(write_path);
            
            {
                boost::archive::text_oarchive oa(ofs);
                oa << this;
            }
        }

        static State deserialize(string read_path) {
            State state;

            {
                std::ifstream ifs(read_path);
                boost::archive::text_iarchive ia(ifs);
                ia >> state;
            }

            return state;
        }

        string to_json();
};
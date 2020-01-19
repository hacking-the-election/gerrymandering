/*=======================================
 shape.hpp:                     k-vernooy
 last modified:               Sun, Jan 19
 
 Class definitions and method declarations
 for shapes, precincts, states, and 
 districts. 
========================================*/

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>

#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;
using namespace rapidjson;

/*
    structure of class definitions:
    - Base shape class - contains border + id
        - Derived precinct class - adds voter data
        - Derived district class
        - Derived state class - has array of precincts + districts
*/

class Shape {

    /* 
        Contains a public vector of coordinates 
        and methods for calculation
    */

    public: 

        Shape(){}; // default constructor

        Shape(vector<vector<float> > shape) {
            // constructor with assignment
            border = shape;
        }

        Shape(vector<vector<float> > shape, string id) {
            // overload constructor for adding id
            border = shape;
            shape_id = id;
        }

        vector<vector<float> > border; // array of coordinates
        string shape_id;

        // gui methods
        void draw(); // prints to an SDL window

        // calculation methods
        double area();
        vector<double> center();

        // for boost serialization
        friend class boost::serialization::access;

        template<class Archive> void serialize(Archive & ar, const unsigned int version) {
            // push shape id and border to archive stream
            ar & shape_id;
            ar & border;
        }
};


class Precinct : public Shape {

    // Derived shape class for defining a precinct

    public:

        Precinct(){}; // default constructor

        Precinct(vector<vector<float> > shape, int demV, int repV) : Shape(shape) {
            // assigning vote data
            dem = demV;
            rep = repV;
        }

        Precinct(vector<vector<float> > shape, int demV, int repV, string id) : Shape(shape, id) {
            // overloaded constructor for adding shape id
            dem = demV;
            rep = repV;
        }

        double get_ratio();        // returns dem/total ratio
        vector<int> voter_data();  // get data in {dem, rep} format
    
        // for boost serialization
        friend class boost::serialization::access;

        template<class Archive> void serialize(Archive & ar, const unsigned int version) {
            // push shape, border and vote data
            ar & shape_id;
            ar & border;
            ar & dem;            
            ar & rep;
        }
        
    private:
        int dem; // democratic vote total
        int rep; // republican vote total
};

class District : public Shape {

    // Derived class for defining a district
    // with expanded methods for algorithms

    public:

        District(){}; // default constructor

        District(vector<vector<float> > shape)
            : Shape(shape) {}; // call the superclass constructor

        // for boost serialization
        friend class boost::serialization::access;

        template<class Archive> void serialize(Archive & ar, const unsigned int version) {
            // push id and border into the archive stream
            ar & id;
            ar & border;
        }

    private:
        int id; // the district id number
};

class State : public Shape {

    /*
        Derived shape class for defining a state.
        Includes arrays of precincts, and districts.

        Contains methods for generating from binary files
        or from raw data with proper district + precinct data

        Contains serialization methods for binary and json.
    */

    public:

        State(){}; // default constructor

        State(vector<District> districts, vector<Precinct> precincts, vector<vector<float> > shape) : Shape(shape) {
            // simple assignment constructor
            state_districts = districts;
            state_precincts = precincts;
        };

        // generate a file from proper raw input
        static State generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON);

        // serialize to json format
        string to_json();

        // serialize to binary
        void write_binary(string path) {
            ofstream ofs(path); // open output stream
            boost::archive::binary_oarchive oa(ofs); // open archive stream
            oa << *this; // put this pointer into stream
            ofs.close(); // close stream
        }

        static State read_binary(string path) {
            State state = State(); // blank state object

            ifstream ifs(path); // open input stream
            boost::archive::binary_iarchive ia(ifs); // open archive stream
            ia >> state; // read into state object

            return state; // return state object
        }

        // for boost serialization
        friend class boost::serialization::access;

        template<class Archive> void serialize(Archive & ar, const unsigned int version) {
            // write districts, precincts, name, and border
            ar & state_districts;
            ar & state_precincts;
            ar & name;
            ar & border;
        }

        // name of state
        string name = "no_name";

        private:
            // arrays of shapes in state
            vector<District> state_districts;
            vector<Precinct> state_precincts;
};
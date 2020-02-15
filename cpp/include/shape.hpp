/*=======================================
 shape.hpp:                     k-vernooy
 last modified:               Mon, Feb 10
 
 Class definitions and method declarations
 for shapes, precincts, states, and 
 districts. 
========================================*/
#pragma once

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

// simplify the coordinate modification system
typedef vector<float> coordinate;
typedef vector<coordinate> coordinate_set;

// an array of 4 max/mins:
typedef vector<float> bounding_box;

// for values between 0-1:
typedef float unit_interval;

// a set of two coordinates:
typedef vector<float> segment;
typedef vector<segment> segments;

/*
    typedef indexes for precinct algorithm 
    implementations - rather than using objects, 
    just use indexes of objects in the array
*/

typedef int p_index;
typedef vector<p_index> p_index_set;
typedef vector<int> seg_index; //  {p_index, segment_index};

class Shape {

    /* 
        Contains a public vector of coordinates 
        and methods for calculation
    */

    public: 

        Shape(){}; // default constructor

        Shape(coordinate_set shape) {
            // constructor with assignment
            border = shape;
        }

        Shape(coordinate_set shape, string id) {
            // overload constructor for adding id
            border = shape;
            shape_id = id;
        }

        coordinate_set border; // array of coordinates
        string shape_id;

        // gui methods
        void draw(); // prints to an SDL window

        // calculation methods
        float area();
        float perimeter();
        coordinate center();
        segments get_segments();

        //! beware of the following line, could cause problems for serialization
        friend bool operator== (Shape& p1, Shape& p2) {
            return (p1.border == p2.border);
        }

        //! beware of the following line, could cause problems for serialization
        friend bool operator!= (Shape& p1, Shape& p2) {
            return (p1.border != p2.border);
        }

        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

        int pop = 0; // total population
};

class Multi_Shape {
    /*
        A class containing a vector of shapes
    */

    public: 

        Multi_Shape(){}; // default constructor

        Multi_Shape(vector<Shape> s) {
            // constructor with assignment
            shapes = s;
        }

    vector<Shape> shapes;
};

class Precinct : public Shape {

    // Derived shape class for defining a precinct

    public:

        Precinct(){}; // default constructor

        Precinct(coordinate_set shape, int demV, int repV) : Shape(shape) {
            // assigning vote data
            dem = demV;
            rep = repV;
        }

        Precinct(coordinate_set shape, int demV, int repV, string id) : Shape(shape, id) {
            // overloaded constructor for adding shape id
            dem = demV;
            rep = repV;
        }

        double get_ratio();        // returns dem/total ratio
        vector<int> voter_data();  // get data in {dem, rep} format
    
        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

        int dem; // democratic vote total
        int rep; // republican vote total
};

class Precinct_Group : public Shape {

    /* 
        Derived class for defining a district
        with expanded methods for algorithms
    */

    public:
        Multi_Shape border;
        vector<Precinct> precincts;
        void add_precinct(Precinct pre) {precincts.push_back(pre);};

        Precinct_Group(){}; // default constructor
        Precinct_Group(coordinate_set shape)
            : Shape(shape) {}; // call the superclass constructor
        
        // serialize and read to and from binary
        void write_binary(string path);
        static Precinct_Group read_binary(string path);

        int get_population();
        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

    private:
        int id; // the district id number
};

// for cleaner naming of types when writing community algorithm
typedef Precinct_Group Community;
typedef vector<Precinct_Group> Communities;

/*
    Derived shape class for defining a state.
    Includes arrays of precincts, and districts.

    Contains methods for generating from binary files
    or from raw data with proper district + precinct data

    Contains serialization methods for binary and json.
*/

class State : public Precinct_Group {

    public:

        State(){}; // default constructor

        State(vector<Precinct_Group> districts, vector<Precinct> precincts, coordinate_set shape) : Precinct_Group(shape) {
            // simple assignment constructor
            state_districts = districts;
            state_precincts = precincts;
        };

        // generate a file from proper raw input
        static State generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON);

        // serialize and read to and from binary, json
        void write_binary(string path);
        static State read_binary(string path);
        string to_json();

        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

        // for the community generation algorithm
        void generate_communities(int num_communities, float compactness_tolerance, float partisanship_tolerance, float population_tolerance);
        void give_precinct(p_index precinct, p_index community, string t_type);

        // initial random configuration of communities
        void generate_initial_communities(int num_communities);

        // for the iterative methods
        void refine_compactness(float compactness_tolerance);
        p_index get_next_community(float compactness_tolerance, int process);
        void refine_partisan(float partisanship_tolerance);
        void refine_population(float population_tolerance);

        // return precinct that can be added to the current precinct that won't create islands in the state
        p_index get_addable_precinct(p_index_set available_precincts, p_index current_precinct);
        // write out communities at a certain point in time
        void save_communities(string write_path);

        // name of state
        string name = "no_name";

        // arrays of shapes in state
        vector<Precinct_Group> state_districts;
        vector<Precinct> state_precincts;
        Communities state_communities;

        void draw();
};

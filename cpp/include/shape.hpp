/*=======================================
 shape.hpp:                     k-vernooy
 last modified:               Mon, Feb 17
 
 Class definitions and method declarations
 for shapes, precincts, states, and 
 districts. Useful geometry classes and
 typedefs (coordinate, LinearRing) defined
 here as well.
========================================*/

#pragma once // avoid multiple includes

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include <array> 
#include <exception>

// for the rapidjson parser
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

// for boost binary serialization
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;
using namespace rapidjson;

// =================================================================

/*
    structure of class definitions:
    - Base shape class - contains border + id
        - Derived precinct class - adds voter data
        - Derived district class
        - Derived state class - has array of precincts + districts
*/

namespace GeoGerry {

class LinearRing;      // a group of lines
class Shape;           // Exterior and optional interior LinearRings (for holes)
class Multi_Shape;     // A group of Shapes
class Precinct;        // Voter block class
class Precinct_Group;  // A group of precincts
class State;           // Contains arrays of the above, as well as methods for various algorithms
class Exceptions;       // for any error to be thrown

// simplify the coordinate modification system
typedef array<double, 2> coordinate;        // a list in form {x1, y1}
typedef vector<coordinate> coordinate_set;  // list of coordiantes: {{x1, y1}, {x2, y2}}

// an array of 4 max/mins:
typedef array<double, 4> bounding_box;

// for values between 0-1:
typedef double unit_interval;

// a set of two coordinates:
typedef array<double, 4> segment;
typedef vector<segment> segments;

/*
    typedef indexes for precinct algorithm 
    implementations - rather than using objects, 
    just use indexes of objects in the array
*/

typedef int p_index;
typedef vector<p_index> p_index_set;
typedef vector<int> seg_index; //  {p_index, segment_index};

class Exceptions {
    public:
        struct RingNotClosed : public exception {
            const char* what() const throw() {
                return "Points of LinearRing do not make closed shape.";
            }
        };
};

class LinearRing {
    /*
        Contains a mandatory `border` property that contains
        a simple coordiante_set - a sequence of lines.
        
        Basically just a wrapper for the coordinate_set typedef
        with extended method functionality
    */

    public: 

        LinearRing() {};
        LinearRing(coordinate_set b) {
            border = b;
        }

        virtual float area();            // area of shape using shoelace theorem
        virtual float perimeter();       // sum distance of segments
        virtual coordinate center();     // average of all points in shape
        virtual segments get_segments(); // return a segment list with shape's segments

        coordinate_set border;

        // add operator overloading for object equality
        friend bool operator== (LinearRing& l1, LinearRing& l2) {
            return (l1.border == l2.border);
        }

        // add operator overloading for object inequality
        friend bool operator!= (LinearRing& l1, LinearRing& l2) {
            return (l1.border != l2.border);
        }


        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);
};

class Shape {
    /* 
        Contains a public border of a LinearRing object
        and a public vector of linearRings for holes.

        Shape objects may contain hole(s), but do not have
        multiple exterior borders
    */

    public: 

        Shape(){}; // default constructor

        Shape(LinearRing shape) {
            // constructor with assignment
            border = shape;
        }

        // overload constructor for adding id
        Shape(LinearRing shape, string id) {
            border = shape;
            shape_id = id;
        }

        LinearRing border;               // array of coordinates
        vector<LinearRing> holes;        // array of holes in shape
        string shape_id;                 // the shape's ID, if applicable

        virtual void draw();             // prints to an SDL window

        virtual float area();            // return (area of shape - area of holes)
        virtual float perimeter();  
        virtual coordinate center();
        virtual segments get_segments(); // return a segment list with shape's segments

        // add operator overloading for object equality
        friend bool operator== (Shape& p1, Shape& p2) {
            return (p1.border == p2.border);
        }

        // add operator overloading for object inequality
        friend bool operator!= (Shape& p1, Shape& p2) {
            return (p1.border != p2.border);
        }

        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

        int pop = 0; // total population
        bool is_part_of_multi_polygon = false; // for parsing rules
};

class Precinct : public Shape {

    // Derived shape class for defining a precinct

    public:

        Precinct(){}; // default constructor

        Precinct(coordinate_set shapes, int demV, int repV) : Shape(shapes) {
            // assigning vote data
            dem = demV;
            rep = repV;
        }

        Precinct(coordinate_set shapes, int demV, int repV, string id) : Shape(shapes, id) {
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

class Multi_Shape : public Shape {
    /*
        A class containing a vector of shapes
    */

    public: 

        Multi_Shape(){}; // default constructor

        Multi_Shape(vector<Shape> s) {
            // constructor with assignment
            border = s;
        }
        
        Multi_Shape(vector<Shape> s, string t_id) {
            // constructor with assignment
            border = s;
            shape_id = t_id;
        }

        Multi_Shape(vector<Precinct> s) {
            // constructor with assignment
            for (Precinct p : s) {
                border.push_back(Shape(p.border));
            }
        }
        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

        vector<Shape> border;

        // gui methods
        virtual void draw(); // prints to an SDL window
};

class Precinct_Group : public Multi_Shape {

    /* 
        Derived class for defining a district
        with expanded methods for algorithms
    */

    public:
        vector<Precinct> precincts;
        void add_precinct(Precinct pre) {precincts.push_back(pre);};

        Precinct_Group(){}; // default constructor
        Precinct_Group(vector<Shape> shapes)
            : Multi_Shape(shapes) {}; // call the superclass constructor
        
        Precinct_Group(vector<Precinct> shapes)
            : Multi_Shape(shapes) {}; // call the superclass constructor

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


class State : public Precinct_Group {
    /*
        Derived shape class for defining a state.
        Includes arrays of precincts, and districts.

        Contains methods for generating from binary files
        or from raw data with proper district + precinct data
        and serialization methods for binary and json.
        
        This is where the algorithmic methods are defined,
        for communities and for quantification
    */

    public:

        State(){}; // default constructor

        State(vector<Multi_Shape> districts, vector<Precinct> state_precincts, vector<Shape> shapes) : Precinct_Group(shapes) {
            // simple assignment constructor
            state_districts = districts;
            precincts = state_precincts;
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
        virtual void draw();
        // name of state
        string name = "no_name";

        // arrays of shapes in state
        vector<Multi_Shape> state_districts;
        Communities state_communities;
};
}
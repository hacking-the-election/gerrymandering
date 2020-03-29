/*=======================================
 shape.hpp:                     k-vernooy
 last modified:               Fri, Feb 28
 
 Class definitions and method declarations
 for shapes, precincts, states, and 
 districts. Useful geometry classes and
 typedefs (coordinate, LinearRing) defined
 here as well.
========================================*/

#pragma once // avoid multiple includes

#include <string>
#include <vector>

// for boost binary serialization
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>

/*
    structure of class definitions:
    - Base shape class - contains border + id
        - Derived precinct class - adds voter data
        - Derived district class
        - Derived state class - has array of precincts + districts
*/

namespace GeoGerry {

class LinearRing;      // a group of lines
class Polygon;         // Exterior and optional interior LinearRings (for holes)
class Multi_Polygon;   // A group of Polygons
class Precinct;        // Voter block class
class Precinct_Group;  // A group of precincts
class State;           // Contains arrays of the above, as well as methods for various algorithms
class Exceptions;      // for any error to be thrown
class Community;       // list of precinct id's
class Graph;           // precinct indices as vertices and edges

// simplify the coordinate modification system
typedef std::array<long int, 2> coordinate;              // a list in form {x1, y1}
typedef std::vector<coordinate> coordinate_set;     // list of coordiantes: {{x1, y1}, {x2, y2}}
typedef std::array<long int, 4> bounding_box;            // an array of 4 max/mins:

// for values between 0-1:
typedef double unit_interval;

// a set of two coordinates:
typedef std::array<long int, 4> segment;
typedef std::vector<segment> segments;

// for defining indices of arrays rather than referring to objects:
typedef int p_index;                       // defines an index in an array   
typedef std::vector<p_index> p_index_set;  // vector of indices in an array
typedef std::array<int, 2> seg_index;      // {p_index, segment_index};

enum processes { PARTISANSHIP, COMPACTNESS, POPULATION };

class Exceptions {
    /*
        For throwing custom errors when bad inputs are
        passed to some of these functions/constructors
    */

    public:
        struct RingNotClosed : public std::exception {
            const char* what() const throw() {
                return "Points of LinearRing do not make closed shape.";
            }
        };
        
        struct CreatesIsland : public std::exception {
            const char* what() const throw() {
                return "This precinct exchange would create an island";
            }
        };

        struct PrecinctNotInGroup : public std::exception {
            const char* what() const throw() {
                return "There is no precinct in this group to remove matching the parameter.";
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

        virtual double get_area();            // area of shape using shoelace theorem
        virtual double get_perimeter();       // sum distance of segments
        virtual coordinate get_center();      // average of all points in shape
        virtual segments get_segments();      // return a segment list with shape's segments

        virtual std::string to_json();

        coordinate_set border;

        // add operator overloading for object equality
        friend bool operator== (LinearRing l1, LinearRing l2);
        friend bool operator!= (LinearRing l1, LinearRing l2);

        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);
};


class Polygon {
    /* 
        Contains a public border of a LinearRing object
        and a public vector of linearRings for holes.

        Polygon objects may contain hole(s), but do not have
        multiple exterior borders
    */

    public: 

        Polygon(){}; // default constructor

        Polygon(LinearRing shape) {
            // no holes in shape
            hull = shape;
        }

        // overload constructor for adding id
        Polygon(LinearRing shape, std::string id) {
            hull = shape;
            shape_id = id;
        }

        Polygon(LinearRing ext, std::vector<LinearRing> interior) {
            hull = ext;
            holes = interior;
        }

        Polygon(LinearRing ext, std::vector<LinearRing> interior, std::string id) {
            hull = ext;
            holes = interior;
            shape_id = id;
        }


        LinearRing hull;                 // array of coordinates - ext border
        std::vector<LinearRing> holes;        // array of holes in shape
        std::string shape_id;                 // the shape's ID, if applicable
        virtual std::string to_json();

        // geometric methods, overwritten in Polygon class

        virtual double get_area();            // return (area of shape - area of holes)
        virtual double get_perimeter();       // total perimeter of holes + hull
        virtual coordinate get_center();      // average centers of holes + hull
        virtual segments get_segments();      // return a segment list with shape's segments
        virtual double get_compactness();

        // add operator overloading for object equality
        friend bool operator== (Polygon p1, Polygon p2);
        friend bool operator!= (Polygon p1, Polygon p2);

        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

        int pop = 0; // total population
        int is_part_of_multi_polygon = -1; // for parsing rules
};

class Precinct : public Polygon {

    // Derived shape class for defining a precinct

    public:

        Precinct(){} // default constructor

        Precinct(LinearRing ext, int demV, int repV) : Polygon(ext) {
            // assigning vote data
            dem = demV;
            rep = repV;
        }

        Precinct(LinearRing ext, std::vector<LinearRing> interior, int demV, int repV) : Polygon(ext, interior) {
            // assigning vote data
            dem = demV;
            rep = repV;
        }


        Precinct(LinearRing ext, int demV, int repV, std::string id) : Polygon(ext, id) {
            // overloaded constructor for adding shape id
            dem = demV;
            rep = repV;
        }


        Precinct(LinearRing ext, int demV, int repV, int popu, std::string id) : Polygon(ext, id) {
            // overloaded constructor for adding shape id
            dem = demV;
            rep = repV;
            pop = popu;
        }

        Precinct(LinearRing ext, std::vector<LinearRing> interior, int demV, int repV, std::string id) : Polygon(ext, interior, id) {
            // overloaded constructor for adding shape id
            dem = demV;
            rep = repV;
        }

        virtual double get_ratio();                 // returns dem/total ratio
        std::vector<int> get_voter_data();  // get data in {dem, rep} format

        // add operator overloading for object equality
        friend bool operator== (Precinct p1, Precinct p2);
        friend bool operator!= (Precinct p1, Precinct p2);

        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

        int dem; // democratic vote total
        int rep; // republican vote total
};


class Multi_Polygon : public Polygon {

    // A class containing a vector of shapes

    public: 

        Multi_Polygon(){}; // default constructor
        Multi_Polygon(std::vector<Polygon> s) {
            // constructor with assignment
            border = s;
        }
        
        Multi_Polygon(std::vector<Polygon> s, std::string t_id) {
            // constructor with assignment
            border = s;
            shape_id = t_id;
        }

        Multi_Polygon(std::vector<Precinct> s) {
            // constructor with assignment
            for (Precinct p : s) {
                // copy precinct data to shape object
                Polygon s = Polygon(p.hull, p.holes, p.shape_id);
                border.push_back(s);
            }
        }

        double get_perimeter();               // total perimeter of border array
        coordinate get_center();              // total perimeter of border array
        double get_compactness();             // average compactenss of each shape
        double get_area();                    // total area of the border shape array
        virtual segments get_segments();      // return a segment list with shape's segments
        virtual std::string to_json();       

        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

        std::vector<Polygon> border;

        // add operator overloading for object equality
        friend bool operator== (Multi_Polygon& s1, Multi_Polygon& s2);
        friend bool operator!= (Multi_Polygon& s1, Multi_Polygon& s2);
};


class Graph {
    public:
        std::vector<p_index> vertices;
        std::vector<std::array<p_index, 2> > edges;
};


class Precinct_Group : public Multi_Polygon {

    /* 
        Derived class for defining a district
        with expanded methods for algorithms
    */

    public:
        std::vector<Precinct> precincts;  // array of precinct objects

        virtual void remove_precinct(Precinct pre);
        virtual void remove_precinct_n(Precinct pre);
        virtual void remove_precinct(p_index pre);
        virtual void remove_precinct_n(p_index pre);

        virtual void add_precinct(Precinct pre);
        virtual void add_precinct_n(Precinct pre);
        

        Precinct_Group(){};
        Precinct_Group(std::vector<Polygon> shapes)
            : Multi_Polygon(shapes) {}; // call the superclass constructor
        
        Precinct_Group(std::vector<Precinct> shapes) : Multi_Polygon(shapes) {
            precincts = shapes;
        };

        double get_ratio();
        std::string to_json();
        int get_population();

        // for boost serialization
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);

    private:
        int id; // the district id number
};


// for cleaner naming of types when writing community algorithm
// typedef Precinct_Group Community;
typedef std::vector<Community> Communities;

class Community : public Precinct_Group {
    /*
        Contains a list of precincts, as well as information about
        linking and where is it on an island list.
    */

    public:
        int size;
        Community(){}

        std::string save_frame();
        static Communities load_frame(std::string read_path, State precinct_list);
};

class State : public Precinct_Group {
    /*
        Derived shape class for defining a state.
        Includes arrays of precincts, and districts.

        Contains methods for generating from binary files
        or from raw data with proper district + precinct data
        and serialization methods for binary and json.
    */

    public:

        State(){}; // default constructor

        State(std::vector<Multi_Polygon> t_districts, std::vector<Precinct> state_precincts, std::vector<Polygon> shapes) : Precinct_Group(shapes) {
            // simple assignment constructor
            districts = t_districts;
            precincts = state_precincts;
        };

        // generate a file from proper raw input
        static State generate_from_file(std::string precinct_geoJSON, std::string voter_data, std::string district_geoJSON, std::vector<std::vector<std::string> >);
        static State generate_from_file(std::string precinct_geoJSON, std::string district_geoJSON, std::vector<std::vector<std::string> > opts);

        Graph network;
        std::string name = "no_name";
        std::vector<Multi_Polygon> districts;

        // serialize and read to and from binary, json
        void write_binary(std::string path);
        static State read_binary(std::string path);
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive & ar, const unsigned int version);
};
}
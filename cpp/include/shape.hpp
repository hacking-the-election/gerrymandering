/*=======================================
 shape.hpp:                     k-vernooy
 last modified:             Thurs, Apr 30
 
 Class definitions and method declarations
 for shapes, precincts, states, and 
 districts. Useful geometry classes and
 typedefs (coordinate, LinearRing) defined
 here as well.
========================================*/

#pragma once

#include <string>
#include <vector>
#include <map>

// for boost binary serialization
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>


namespace Geometry {

    // Geometry classes. These all define groups of points
    // (can be converted to segments) that make up different
    // types of geometries.

    class LinearRing;      // a group of lines
    class Polygon;         // Exterior and optional interior LinearRings (for holes)
    class Multi_Polygon;   // A group of Polygons


    // Political division structures for
    // parsing and representing variuos levels
    // and sizes of US political voting blocks.

    class Precinct;        // Voter block class
    class Precinct_Group;  // A group of precincts
    class State;           // Contains arrays of the above, as well as methods for various algorithms

    // Graph classes representing graphs as defined by
    // discrete mathematics. Useful for creating networks
    // and defining the communities algorithm

    class Node;            // A vertex on a graph
    class Graph;           // precinct indices as vertices and edges

    // for any error to be thrown
    class Exceptions;


    typedef std::array<long int, 2> coordinate;         // list in the form {x1, y1};
    typedef std::vector<coordinate> coordinate_set;     // list of coordinates: {{x1, y1} ... {xn, yn}};
    typedef std::array<long int, 4> bounding_box;       // an array of 4 max/mins: {top, bottom, left, right};
    typedef std::array<long int, 4> segment;            // a set of two coordinates:
    typedef std::vector<segment> segments;              // list of multiple segments
    typedef double unit_interval;                       // for values between [0, 1]


    enum POLITICAL_PARTIES {
        // define all policial parties for 
        // voter_data map in precinct class

        DEMOCRAT,
        REPUBLICAN,
        GREEN,
        INDEPENDENT,
        LIBERTARIAN,
        OTHER // refers to all non-specified parties
    };


    class Exceptions {
        /*
            For throwing custom errors when bad inputs are
            passed to some of these functions/constructors
        */

    public:
        struct CreatesIsland : public std::exception {
            const char* what() const throw() {
                return "Precinct exchange creates an island";
            }
        };

        struct PrecinctNotInGroup : public std::exception {
            const char* what() const throw() {
                return "No precinct in this precinct group matches the provided argument";
            }
        };
    };


    class LinearRing {
        /*
            Contains a mandatory `border` property that contains
            a simple coordiante_set - a sequence of lines.
            
            Basically a wrapper for the coordinate_set typedef
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


        LinearRing hull;                      // array of coordinates - ext border
        std::vector<LinearRing> holes;        // array of holes in shape
        std::string shape_id;                 // the shape's ID, if applicable
        
        virtual std::string to_json();

        // geometric methods, overwritten in Polygon class

        virtual double get_area();            // return (area of shape - area of holes)
        virtual double get_perimeter();       // total perimeter of holes + hull
        virtual coordinate get_center();      // average centers of holes + hull
        virtual segments get_segments();      // return a segment list with shape's segments
        virtual double get_compactness();
        virtual bounding_box get_bounding_box();

        // add operator overloading for object equality
        friend bool operator== (Polygon p1, Polygon p2);
        friend bool operator!= (Polygon p1, Polygon p2);
        
        int pop = 0; // total population
        int is_part_of_multi_polygon = -1; // for parsing rules
    };


    class Precinct : public Polygon {
        /*
            Derived shape class for defining a precinct.
            Contains border data, holes, and constituency data

            Data contained:
                - Total population, if specified
                - Voting Age Population, if specified
                - Election Data, likely from 2008 presidential election:
                    - Democrat
                    - Republican
                    - Libertarian
                    - Green
                    - Independent
                    - "Other" - sum of all unspecified parties
        */

        public:

            Precinct(){} // default constructor

            Precinct(LinearRing ext, int dem, int rep)
                : dem(dem), rep(rep), Polygon(ext) {}

            Precinct(LinearRing ext, std::vector<LinearRing> interior, int dem, int rep)
                : dem(dem), rep(rep), Polygon(ext, interior) {}


            Precinct(LinearRing ext, int dem, int rep, std::string id)
                : dem(dem), rep(rep), Polygon(ext, id) {}


            Precinct(LinearRing ext, int dem, int rep, int popu, std::string id)
                : dem(dem), rep(rep), Polygon(ext, id) {
                    this->pop = popu;
            }

            Precinct(LinearRing ext, std::vector<LinearRing> interior, int dem, int rep, std::string id)
                : dem(dem), rep(rep), Polygon(ext, interior, id) {}


            std::vector<int> get_voter_data();  // get data in {dem, rep} format

            // add operator overloading for object equality
            friend bool operator== (const Precinct& p1, const Precinct& p2);
            friend bool operator!= (const Precinct& p1, const Precinct& p2);

            int dem; // democratic vote total
            int rep; // republican vote total
    };


    class Multi_Polygon : public Polygon {
        /*
            
        */
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

        std::vector<Polygon> border;

        // add operator overloading for object equality
        friend bool operator== (Multi_Polygon& s1, Multi_Polygon& s2);
        friend bool operator!= (Multi_Polygon& s1, Multi_Polygon& s2);
    };


    typedef std::array<int, 2> Edge;


    class Node {
        /*
            A vertex on the `Graph` class, containing
            precinct information and edge information
        */
        
        public:

            int id;
            int community;
            Precinct* precinct;

            Node() {};
            Node(Precinct* precinct) : precinct(precinct) {};
            
            friend bool operator< (const Node& l1, const Node& l2);
            friend bool operator== (const Node& l1, const Node& l2);

            std::vector<Edge > edges;
            std::vector<int> collapsed;
            
            // for boost serialization
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive & ar, const unsigned int version);
    };


    class Graph {
        public:
            std::map<int, Node> vertices;
            std::vector<Edge> edges;

            // drivers for component algorithm
            int get_num_components();
            std::vector<Graph> get_components();

            // recursors for getting different data
            void dfs_recursor(int v, std::vector<bool>& visited);
            void dfs_recursor(int v, std::vector<bool>& visited, std::vector<int>* nodes);

            void add_edge(Edge);
            void remove_edge(Edge);
            void remove_edges_to(int id);
            
            // for boost serialization
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive & ar, const unsigned int version);
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
            virtual void remove_precinct(int pre);
            virtual void remove_precinct_n(int pre);

            virtual void add_precinct(Precinct pre);
            virtual void add_precinct_n(Precinct pre);
            

            Precinct_Group(){};
            Precinct_Group(std::vector<Polygon> shapes)
                : Multi_Polygon(shapes) {}; // call the superclass constructor
            
            Precinct_Group(std::vector<Precinct> shapes) : Multi_Polygon(shapes) {
                precincts = shapes;
            };

            double get_area();
            int get_population();
            std::string to_json();
            coordinate get_center();
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

        State(std::vector<Multi_Polygon> districts, std::vector<Precinct> t_precincts, std::vector<Polygon> shapes)
            : Precinct_Group(shapes), districts(districts) {
            // simple assignment constructor for
            // nonstatic superclass member
            precincts = t_precincts;
        }

        // generate a file from proper raw input with and without additional voter data files
        static State generate_from_file(std::string, std::string, std::string, std::vector<std::vector<std::string> >);
        static State generate_from_file(std::string, std::string, std::vector<std::vector<std::string> >);

        Graph network; // represents the precinct network of the state
        std::vector<Multi_Polygon> districts; // the actual districts of the state

        // serialize and read to and from binary, json
        void to_binary(std::string path);
        static State from_binary(std::string path);
    };
}
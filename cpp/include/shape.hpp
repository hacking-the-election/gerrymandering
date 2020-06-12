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
#include <array>
#include <unordered_map>


/**
 * \cond
 */
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include "../lib/ordered-map/include/tsl/ordered_map.h"

#include <boost/serialization/split_free.hpp>
#include <boost/serialization/utility.hpp>

/**
 * \endcond
 */

/*
 * All code commenting is done through Doxygen for auto-generation
 * of HTML documentation. See more information about this in the root
 * README of this repository.
*/

namespace hte {
namespace Geometry {

    // Geometry classes. These all define groups of points
    // (can be converted to segments) that make up different
    // types of geometries.

    class LinearRing;      //!< a group of lines
    class Polygon;         //!< Exterior and optional interior LinearRings (for holes)
    class Multi_Polygon;   //!< A group of Polygons


    // Political division structures for
    // parsing and representing variuos levels
    // and sizes of US political voting blocks.

    class Precinct;        //!< Voter block class
    class Precinct_Group;  //!< A group of precincts
    class State;           //!< Contains arrays of the above, as well as methods for various algorithms

    // Graph classes representing graphs as defined by
    // discrete mathematics. Useful for creating networks
    // and defining the communities algorithm

    class Node;            //!< A vertex on a graph
    class Graph;           //!< precinct indices as vertices and edges

    // for any error to be thrown
    class Exceptions;
    
    typedef std::array<long, 2> coordinate;          //!< list in the form {x1, y1};
    typedef std::vector<coordinate> coordinate_set;  //!< list of coordinates: {{x1, y1} ... {xn, yn}};
    typedef std::array<long, 4> bounding_box;        //!< an array of 4 max/mins: {top, bottom, left, right};
    typedef std::array<long, 4> segment;             //!< a set of two coordinates:
    typedef std::vector<segment> segments;           //!< list of multiple segments
    typedef double unit_interval;                    //!< for values between [0, 1]

    // for parsing and storing data

    /**
     * \brief An enum storing different political parties.
     */
    enum class POLITICAL_PARTY {
        DEMOCRAT,
        REPUBLICAN,
        GREEN,
        INDEPENDENT,
        LIBERTARIAN,
        REFORM,
        OTHER,
        TOTAL,
        ABSOLUTE_QUANTIFICATION
    };

    /**
     * \brief Different types of parsing ID's in geodata and election data
     */
    enum class ID_TYPE {
        GEOID,
        ELECTIONID,
        POPUID
    };


    /**
     * Convert a string that represents a geojson polygon into an Multi_Polygon object.
     * @param str The string to be converted into a vector of Polygons
     * @param texas_coordinates Whether or not to use texas-scaled coordinates 
     * @return The converted polygon object
     * 
     * @see Multi_Polygon
    */
    Multi_Polygon multi_string_to_vector(std::string str, bool texas_coordinates);
    
    /**
     * Convert a string that represents a geojson polygon into an object.
     * @param str The string to be converted into a Polygon
     * @param texas_coordinates Whether or not to use texas-scaled coordinates 
     * @return The converted polygon object
     * 
     * @see Polygon
    */
    Polygon string_to_vector(std::string str, bool texas_coordinates);


    /**
     * Custom exceptions for geometric errors and
     * recursion breakouts.
    */
    class Exceptions {
        public:

            struct PrecinctNotInGroup : public std::exception {
                const char* what() const throw() {
                    return "No precinct in this precinct group matches the provided argument";
                }
            };

            struct LinearRingOpen : public std::exception {
                const char* what() const throw() {
                    return "Points LinearRing do not form closed ring";
                }
            };
    };


    /**
     * \brief A closed set of points
     * 
     * A class containing a closed coordinate_set, analagous to
     * the `LinearRing` property in most GeoJson. 
     * A wrapper for the coordinate_set typedef
     * with extended method functionality for geometric properties
     * such as area, perimeter, and centroid calculation
     * 
     * \throw Must be passed a closed coordinate set in the 
     * constructor or will throw Exceptions::LinearRingOpen
     */

    class LinearRing {
        public:

            LinearRing() : centroid({{NULL, NULL}}) {}
            LinearRing(coordinate_set b) : centroid({{NULL, NULL}}) {
                if (b[0] != b[b.size() - 1]) {
                    throw Exceptions::LinearRingOpen();
                }

                border = b;
            }

            coordinate_set border;  //!< A closed set of coordinates
            coordinate centroid;    //!< The centroid (default NULL,NULL) of the ring

            /**
             * \brief Get the signed area of the linear ring
             * Use the shoelace theorem to return the signed area
             * of the `border` property. Sign is based on coordinate order.
             * 
             * @return Signed area of `border`
            */
            virtual double get_area();
            virtual std::string to_json();
            virtual double get_perimeter();          // sum distance of segments
            virtual coordinate get_centroid();       // average of all points in shape
            virtual segments get_segments();         // return a segment list with shape's segments
            virtual bounding_box get_bounding_box();

            // add operator overloading for object equality
            friend bool operator== (const LinearRing& l1, const LinearRing& l2);
            friend bool operator!= (const LinearRing& l1, const LinearRing& l2);
    };


    /**
     * \brief A shape with a hull and holes
     * 
     * Contains a public border of a LinearRing object
     * and a public vector of LinearRings for holes.
     * Polygon objects may contain holes, but do not have
     * multiple exterior borders
     */
    class Polygon {
        public:

            Polygon(){}
            Polygon(LinearRing hull)
                : hull(hull) {}
            Polygon(LinearRing hull, std::string shape_id)
                : hull(hull), shape_id(shape_id) {}

            Polygon(LinearRing hull, std::vector<LinearRing> holes) 
                : hull(hull), holes(holes) {}

            Polygon(LinearRing hull, std::vector<LinearRing> holes, std::string shape_id) 
                : hull(hull), holes(holes), shape_id(shape_id) {}


            LinearRing hull;                          //!< Ring of exterior hull coordinates
            std::vector<LinearRing> holes;            //!< List of exterior holes in shape
            std::string shape_id;                     //!< The shape's GEOID, if applicable

            virtual std::string to_json();            // get the coordinate data of the polygon as GeoJson
            virtual double get_area();                // return (area of shape - area of holes)
            virtual double get_perimeter();           // total perimeter of holes + hull
            virtual coordinate get_centroid();        // average centers of holes + hull
            virtual segments get_segments();          // return a segment list with shape's segments
            virtual bounding_box get_bounding_box();  // get the bounding box of the hull

            // add operator overloading for object equality
            friend bool operator== (const Polygon& p1, const Polygon& p2);
            friend bool operator!= (const Polygon& p1, const Polygon& p2);
            
            int pop = 0;                        //!< Total population of the polygon
            int is_part_of_multi_polygon = -1;  //!< Internal data for parsing rules
    };


    /**
     * \brief Derived shape class for defining a precinct.
     * 
     * Contains border data, holes, and constituency data
     * Has total population, election data (in the format of)
     * the POLITICAL_PARTY enum, likely from the 2008 presidential election
     */
    class Precinct : public Polygon {
        public:

            Precinct(){} // default constructor
            Precinct(LinearRing ext) : Polygon(ext) {}
            Precinct(LinearRing ext, std::string id) : Polygon(ext, id) {}
            Precinct(LinearRing ext, std::vector<LinearRing> interior)
                : Polygon(ext, interior) {}
            Precinct(LinearRing ext, std::vector<LinearRing> interior, std::string id)
                : Polygon(ext, interior, id) {}
            Precinct(LinearRing ext, int popu, std::string id) : Polygon(ext, id) {
                this->pop = popu;
            }

            // add operator overloading for object equality
            friend bool operator== (const Precinct& p1, const Precinct& p2);
            friend bool operator!= (const Precinct& p1, const Precinct& p2);

            std::map<POLITICAL_PARTY, int> voter_data;  //!< Voter data in the form `{POLITICAL_PARTY, count}`
    };


    /**
     * \brief A polygon object with multiple polygons.
     * 
     * Used for geographical objects similar to the entire discontiguous
     * US and Alaska within a single object, useful for graphics.
     * Each polygon has a mandatory `hull` property, and may contain holes.
     */
    class Multi_Polygon : public Polygon {
        public:

            Multi_Polygon(){} // default constructor
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
            double get_area();                    // total area of the border shape array

            coordinate get_centroid();              // total perimeter of border array
            virtual segments get_segments();      // return a segment list with shape's segments
            virtual std::string to_json();       
            bounding_box get_bounding_box();

            std::vector<Polygon> border;

            // add operator overloading for object equality
            friend bool operator== (const Multi_Polygon& s1, const Multi_Polygon& s2);
            friend bool operator!= (const Multi_Polygon& s1, const Multi_Polygon& s2);
    };


    /**
     * A class for defining a group of precincts.
     * Used for all geometric calculations in the
     * community generation algorithm
     */
    class Precinct_Group : public Multi_Polygon {
        public:

            Precinct_Group(){}
            Precinct_Group(std::vector<Polygon> shapes)
                : Multi_Polygon(shapes) {}
            
            Precinct_Group(std::vector<Precinct> precincts)
                : Multi_Polygon(precincts), precincts(precincts) {}


            // array of precinct objects
            std::vector<Precinct> precincts;

            virtual void remove_precinct(Precinct);
            virtual void add_precinct(Precinct);
            Precinct get_precinct_from_id(std::string);
            bounding_box get_bounding_box();
            double get_area();
            coordinate get_centroid();

            int get_population();
            std::string to_json();

            static Precinct_Group from_graph(Graph& g);
    };


    typedef std::array<int, 2> Edge;

    /**
     * A vertex on the `Graph` class, containing
     * precinct information and edge information
    */
    class Node {
        public:

            int id;                             //!< A unique integer identifier for the node
            int community;                      //!< Which community the node belongs to
            hte::Geometry::Precinct* precinct;  //!< A precinct pointer for geometry functions

            Node(){}
            Node(hte::Geometry::Precinct* precinct) : precinct(precinct) {}
            ~Node() {
                delete precinct;
            }

            std::vector<Edge> edges;       //!< A list of unique edges for the node, in the form `{this->id, connected_node.id}`
            std::vector<int> collapsed;    //!< A list of collapsed nodes for the karger-stein algorithm

            friend bool operator< (const Node& l1, const Node& l2);
            friend bool operator== (const Node& l1, const Node& l2);
    };

    /**
     * \brief Remove all edges to specific node
     * 
     * For every node connected to `id`, remove the edge
     * connecting to `id`.
     * 
     * @param g Graph to remove edges from
     * @param id Node to removes edges to
     * @return Updated graph with removed edges
     */
    Graph remove_edges_to(Graph g, int id);


    /**
     * \brief A graph as defined by discrete mathematics
     * 
     * Contains a list of Node objects, with Edges connecting
     * a certain subset of Nodes. This represents an undirected
     * and unweighted graph. It is used for Community
     * generation algorithms
     */
    class Graph {

        public:
            tsl::ordered_map<int, Node> vertices;  //!< All nodes on the graph
            std::vector<Edge> edges;               //!< List of unique edges on the graph

            /**
             * \brief Get the induced subgraph from a list of node ids
             * @param nodes the integer list of nodes to get the subgraph of
             * @return The subgraph object
             */
            Graph get_induced_subgraph(std::vector<int> nodes);

            /**
             * Determine the number of [components](https://en.wikipedia.org/wiki/Component_(graph_theory)) in the graph
             * @return number of components
             */
            int get_num_components();

            /**
             * get a list of subgraph components
             * @return subgraph components
             */
            std::vector<Graph> get_components();
            bool is_connected();

            void add_node(Node node);
            void remove_node(int id);
            void add_edge(Edge);
            void remove_edge(Edge);
            void remove_edges_to(int id);

            protected:
            // recursors for getting different data
            void dfs_recursor(int v, std::unordered_map<int, bool>& visited);
            void dfs_recursor(int v, std::unordered_map<int, bool>& visited, std::vector<int>* nodes);
            void dfs_recursor(int v, int& visited, std::unordered_map<int, bool>& visited_b);
    };


    /**
     * \brief Shape class for defining a state.
     *        Includes arrays of precincts, and districts.
     * 
     * Contains methods for generating from binary files
     * or from raw data with proper district + precinct data
     * and serialization methods for binary and json.
    */
    class State : public Precinct_Group {
        public:

            State(){}
            State(std::vector<Multi_Polygon> districts, std::vector<Precinct> t_precincts, std::vector<Polygon> shapes)
                : Precinct_Group(shapes), districts(districts) {
                // simple assignment constructor for
                // nonstatic superclass member
                precincts = t_precincts;
            }

            // generate a file from proper raw input with and without additional voter data files
            static State generate_from_file(std::string, std::string, std::string, std::map<POLITICAL_PARTY, std::string>, std::map<ID_TYPE, std::string>);
            static State generate_from_file(std::string, std::string, std::map<POLITICAL_PARTY, std::string>, std::map<ID_TYPE, std::string>);

            Graph network; // represents the precinct network of the state
            std::vector<Multi_Polygon> districts; // the actual districts of the state

            // serialize and read to and from binary, json
            void to_binary(std::string path);
            static State from_binary(std::string path);
    };
}
}

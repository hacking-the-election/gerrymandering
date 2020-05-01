/*=======================================
 graph.hpp:                     k-vernooy
 last modified:             Thurs, Apr 30
 
 Class definitions for graph theory
 structures, including Nodes and Graphs.
========================================*/


#pragma once

#include "../include/shape.hpp"
#include "../lib/ordered-map/include/tsl/ordered_map.h"


namespace Geometry {
    /*
        Expand geometry namespace to include
        graph and node class definitions

        @note:
            could probably make a difference namespace
            here that would include all graph utils,
            a separate graph library
    */

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

        std::vector<Edge> edges;
        std::vector<int> collapsed;
        
        friend bool operator< (const Node& l1, const Node& l2);
        friend bool operator== (const Node& l1, const Node& l2);
    };


    class Graph {

    public:
        tsl::ordered_map<int, Node> vertices;
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
    };
}
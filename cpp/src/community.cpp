/*===============================================
 community.cpp:                        k-vernooy
 last modified:                     Fri, Feb 28
 Definition of the community-generation algorithm for
 quantifying gerrymandering and redistricting. This
 algorithm is the main result of the project
 hacking-the-election, and detailed techincal
 documents can be found in the /docs root folder of
 this repository.
 Visit https://hacking-the-election.github.io for
 more information on this project.
 
 Useful links for explanations, visualizations,
 results, and other information:
 Our data sources for actually running this
 algorithm can be seen at /data/ or on GitHub:
 https://github.com/hacking-the-election/data
 This algorithm has been implemented in C++ and in
 Python. If you're interested in contributing to the
 open source political movement, please contact us
 at: hacking.the.election@gmail.com
===============================================*/


#include <math.h>    // for rounding functions
#include <numeric>   // include std::iota
#include <iostream>  // std::cout and std::endl
#include <random>
#include <chrono>


#include "../include/shape.hpp"       // class definitions
#include "../include/community.hpp"   // class definitions
#include "../include/util.hpp"        // array modification functions
#include "../include/geometry.hpp"    // geometry modification, border functions
#include "../include/canvas.hpp"      // geometry modification, border functions

using namespace std;
using namespace Geometry;
using namespace Graphics;
using namespace chrono;


#define VERBOSE 1
#define DEBUG 0


vector<int> refs;
// vector<array<int, 3> > c_colors;
int fill_size;


class EdgeWrapper {
    public:
        Node node_to_remove;
        Node node_to_collapse;

        int ntr_id;
        int ntc_id;

        int attrs;
        
        friend bool operator< (const EdgeWrapper& l1, const EdgeWrapper& l2);
        EdgeWrapper(Node n, Node c, int id, int cid, int attrs) : node_to_remove(n), node_to_collapse(c), ntr_id(id), ntc_id(cid), attrs(attrs) {}

};


bool operator< (const EdgeWrapper& l1, const EdgeWrapper& l2) {
    return (l1.attrs < l2.attrs);
}


Precinct_Group Community::get_shape(Graph& graph) {
    /*
        @desc: derives the shape of a community's node list from a graph
        @params: `Graph&` graph: precinct information graph
        @return: `Precinct_Group` Community shape
    */

    vector<Precinct> precincts;
    for (int id : node_ids) {
        precincts.push_back(*graph.vertices[id].precinct);
    }

    return Precinct_Group(precincts);
}


Communities karger_stein(Graph g, int n_communities) {
    /*
        @desc: Partitions a graph according to the Karger-Stein algorithm

        @params:
            `Graph` g: the graph to partition
            `int` n_communities: the number of partitions

        @return: `Communities` list of id's corresponding to the partition
    */


    while (g.vertices.size() > n_communities) {
        cout << g.vertices.size() << endl;
        vector<EdgeWrapper> attr_lengths;

        cout << "a ";
        auto start = high_resolution_clock::now(); 

        for (int i = 0; i < g.vertices.size() && i < 100; i++) {
            int node_to_remove_id = (g.vertices.begin() + rand_num(0, g.vertices.size() - 1)).key();
            Node node_to_remove_val = g.vertices[node_to_remove_id];
            int node_to_collapse_id = node_to_remove_val.edges[rand_num(0, node_to_remove_val.edges.size() - 1)][1];
            Node node_to_collapse_val = g.vertices[node_to_collapse_id];


            // node_to_remove(n), node_to_collapse(c), ntr_id(id), ntc_id(cid), attrs(attrs)

            EdgeWrapper er(
                node_to_remove_val,
                node_to_collapse_val,
                node_to_remove_id,
                node_to_collapse_id,
                node_to_remove_val.collapsed.size() + node_to_collapse_val.collapsed.size()
            );
            
            attr_lengths.push_back(er);
        }


        auto stop = high_resolution_clock::now(); 
        auto duration = duration_cast<microseconds>(stop - start); 
        cout << duration.count() << endl;
        cout << "b ";

        start = high_resolution_clock::now(); 

        EdgeWrapper min = *min_element(attr_lengths.begin(), attr_lengths.end());
        for (Edge edge : min.node_to_remove.edges) {
            if (edge[1] != min.ntc_id) {
                g.add_edge({edge[1], min.ntc_id});
            }
        }


        g.vertices[min.ntc_id].collapsed.push_back(min.ntr_id);

        for (int c : g.vertices[min.ntr_id].collapsed) {
            if (find(g.vertices[min.ntc_id].collapsed.begin(), g.vertices[min.ntc_id].collapsed.end(), c) == g.vertices[min.ntc_id].collapsed.end()) {
                g.vertices[min.ntc_id].collapsed.push_back(c);
                // cout << "adding to c" << endl;
            }
        }


        g.remove_node(min.ntr_id);
        
        stop = high_resolution_clock::now(); 
        duration = duration_cast<microseconds>(stop - start); 
        cout << duration.count() << endl;
    }

    Communities communities(n_communities);

    for (int i = 0; i < g.vertices.size(); i++) {
        communities[i].node_ids = (g.vertices.begin() + i).value().collapsed;
        communities[i].node_ids.push_back((g.vertices.begin() + i).key());
    }

    cout << "done" << endl;
    return communities;
}


Communities get_initial_configuration(Graph graph, int n_communities) {
    /*
        @desc: determines a random list of community objects

        @params:
            `Geometry::Graph` graph: connection data
            `int` n_communities: number of communities to generate

        @return: `Communities` init config
    */

    Communities communities = karger_stein(graph, n_communities);

    // Canvas canvas(500, 500);
    // canvas.add_shape(communities, graph);
    // canvas.draw();
    // for (int i = 0; i < communities.size(); i++) {
    //     writef(communities[i].get_shape(graph).to_json(), "x" + to_string(i) + ".json");
    // }

    return communities;
}
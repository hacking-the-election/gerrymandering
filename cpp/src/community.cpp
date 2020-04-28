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

#include "../lib/Miniball.hpp"

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
#define ITERATION_LIMIT 100


class EdgeWrapper {
    public:
        int ntr_id;
        int ntc_id;
        int attrs;
        
        friend bool operator< (const EdgeWrapper& l1, const EdgeWrapper& l2);
        inline EdgeWrapper(int id, int cid, int attrs) : ntr_id(id), ntc_id(cid), attrs(attrs) {}
};


bool operator< (const EdgeWrapper& l1, const EdgeWrapper& l2) {
    return (l1.attrs < l2.attrs);
}


double get_compactness(Community& community) {
    coordinate_set points;
    
    for (Precinct p : community.shape.precincts) {
        for (Geometry::coordinate coord : p.hull.border) {
            points.push_back(coord);
        }
    }

    MB mb(2, points.begin(), points.end());
    // mb.center();
} 


double average(Communities& communities, double (*measure)(Community&)) {
    double sum = 0;
    for (Community& c : communities) sum += measure(c);
    return (sum / (double) communities.size());
}


void minimize(Communities& communities, Graph& graph, double (*measure)(Community&)) {
    
    int iterations_since_best = 0;
    Communities best = communities;

    while (iterations_since_best < ITERATION_LIMIT) {
        // choose worst community to modify
        int min_community = 0;
        double min_measure = measure(communities[0]);

        for (int i = 1; i < communities.size(); i++) {
            if (measure(communities[i]) < min_measure) {
                min_measure = measure(communities[i]);
                min_community = 0;
            }
        }

        if (average(communities, measure) > average(best, measure)) {
            best = communities;
            iterations_since_best = 0;
        }
    }

    cout << "did not improve after " << ITERATION_LIMIT << " iterations, returning..." << endl;
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

        EdgeWrapper min(-1, -1, 1000000);
        // cout << g.vertices.size() << endl;

        for (int i = 0; i < g.vertices.size(); i++) {
            int node_to_remove_id = (g.vertices.begin() + rand_num(0, g.vertices.size() - 1)).key();
            int node_to_collapse_id = g.vertices[node_to_remove_id].edges[rand_num(0, g.vertices[node_to_remove_id].edges.size() - 1)][1];
            int t = g.vertices[node_to_remove_id].collapsed.size() + g.vertices[node_to_collapse_id].collapsed.size();

            if (t < min.attrs) {
                min = EdgeWrapper(
                    node_to_remove_id,
                    node_to_collapse_id,
                    t
                );
            }
            
            if (i == 100) break;
        }


        for (Edge edge : g.vertices[min.ntr_id].edges) {
            if (edge[1] != min.ntc_id) {
                g.add_edge({edge[1], min.ntc_id});
            }
        }


        g.vertices[min.ntc_id].collapsed.push_back(min.ntr_id);

        for (int c : g.vertices[min.ntr_id].collapsed) {
            if (find(g.vertices[min.ntc_id].collapsed.begin(), g.vertices[min.ntc_id].collapsed.end(), c) == g.vertices[min.ntc_id].collapsed.end()) {
                g.vertices[min.ntc_id].collapsed.push_back(c);
            }
        }

        g.remove_node(min.ntr_id);
    }

    Communities communities(n_communities);

    for (int i = 0; i < g.vertices.size(); i++) {
        communities[i].node_ids = (g.vertices.begin() + i).value().collapsed;
        communities[i].node_ids.push_back((g.vertices.begin() + i).key());
    }

    // cout << "done" << endl;

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
    for (int i = 0; i < communities.size(); i++) {
        writef(communities[i].get_shape(graph).to_json(), "x" + to_string(i) + ".json");
    }

    return communities;
}
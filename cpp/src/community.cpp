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
using namespace Gerrymandering::Geometry;
using namespace Gerrymandering::Graphics;
using namespace chrono;

#define VERBOSE 1
#define DEBUG 0
#define ITERATION_LIMIT 100


class EdgeWrapper {
    // a class that defines an edge and an attr, for sorting by 

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


void Community::update_shape(Graph& graph) {
    for (int x : this->node_ids) {
        this->shape.add_precinct(*graph.vertices[x].precinct);
    }
}


double get_compactness(Community& community) {
    /*
        @desc: finds the reock compactness of a `Community`
        @params: `Community&` community: community object to find compactness of
        @return: `double` reock compactness
        @ref: https://fisherzachary.github.io/public/r-output.html
    */

    coordinate_set lp;
    vector<vector<double> > p;

    for (Precinct pre : community.shape.precincts) {
        lp.insert(lp.end(), pre.hull.border.begin(), pre.hull.border.end());
    }

    p.reserve(lp.size());

    for (int i = 0; i < lp.size(); i++)
        p.emplace_back(lp[i].begin(), lp[i].end());

    lp.clear();

    MB mb (2, p.begin(), p.end());
    int t = (double)community.shape.get_area() / (mb.squared_radius() * PI);
    return t;
} 


double average(Communities& communities, double (*measure)(Community&)) {
    /*
        @desc: find average `measure` of the `communities`

        @params:
            `Communities&` communities: list of communities measure
            `double (*measure)(Community&)`: pointer to function of type double with param `Community&`
        
        @return: `double` average measure
    */

    double sum = 0;
    for (Community& c : communities) sum += measure(c);
    return (sum / (double) communities.size());
}


void maximize(Communities& communities, Graph& graph, double (*measure)(Community&)) {
    int n_communities = communities.size();
    cout << "optimizing" << endl;

    int iterations_since_best = 0;
    Communities best = communities;

    while (true) {
        // choose worst community to modify
        // vector<double> measures;
        // measures.reserve(n_communities);
        int smallest_index = 0;
        double smallest_measure = measure(communities[0]);
        
        for (int i = 1; i < communities.size(); i++) {
            double x = measure(communities[i]);
            if (x < smallest_measure) {
                smallest_measure = x;
                smallest_index = i;
            }
        }

        cout << "the worst community is " << smallest_index << endl;

        vector<int> takeable;
        for (int i = 0; i < graph.vertices.size(); i++) {
            if (find(communities[smallest_index].node_ids.begin(), communities[smallest_index].node_ids.end(), i) == communities[smallest_index].node_ids.end()) {
                bool has_neighbor = false;
                for (Edge e : graph.vertices[i].edges) {
                    if (find(communities[smallest_index].node_ids.begin(), communities[smallest_index].node_ids.end(), e[1]) != communities[smallest_index].node_ids.end()) {
                        has_neighbor = true;
                        break;
                    }
                }

                if (has_neighbor) {
                    takeable.push_back(i);
                }
            }
        }

        coordinate center = communities[smallest_index].shape.get_center();
        double radius = sqrt(communities[smallest_index].shape.get_area() / PI);

        for (int i = 0; i < takeable.size(); i++) {
            if (point_in_circle(center, radius, graph.vertices[takeable[i]].precinct->get_center())) {
                cout << "adding takeable precinct " << i << endl;
                // communities[graph.vertices[takeable[i]].community].remove_node(graph.vertices[takeable[i]]);
            }
            else {
                cout << "not adding takeable precinct " << i << endl;
            }
        }


        cout << takeable.size() << endl;

        if (average(communities, measure) > average(best, measure)) {
            cout << "found a better solution" << endl;
            best = communities;
            iterations_since_best = 0;
        }
        else {
            cout << "not currently a better solution" << endl;
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
    
    const int MAX_SEARCH = 100;

    while (g.vertices.size() > n_communities) {
        // still more verts than communities needed

        // create edgewrapper as first beatable min
        EdgeWrapper min(-1, -1, 10000000);

        for (int i = 0; i < g.vertices.size(); i++) {
            // choose random node and edge of that node
            int node_to_remove_id = (g.vertices.begin() + rand_num(0, g.vertices.size() - 1)).key();
            int node_to_collapse_id = g.vertices[node_to_remove_id].edges[rand_num(0, g.vertices[node_to_remove_id].edges.size() - 1)][1];
            int t = g.vertices[node_to_remove_id].collapsed.size() + g.vertices[node_to_collapse_id].collapsed.size();

            // if combining the `collapsed` vector of the nodes
            // on this edge is smaller than min, add new min
            if (t < min.attrs) {
                min = EdgeWrapper(
                    node_to_remove_id,
                    node_to_collapse_id,
                    t
                );
            }
            
            // create max cap for searching for min
            if (i == MAX_SEARCH) break;
        }


        // add the edges from node to remove to node to collapse
        for (Edge edge : g.vertices[min.ntr_id].edges) {
            if (edge[1] != min.ntc_id) {
                g.add_edge({edge[1], min.ntc_id});
            }
        }

        // update collapsed of node_to_collapse
        g.vertices[min.ntc_id].collapsed.push_back(min.ntr_id);

        for (int c : g.vertices[min.ntr_id].collapsed) {
            if (find(g.vertices[min.ntc_id].collapsed.begin(), g.vertices[min.ntc_id].collapsed.end(), c) == g.vertices[min.ntc_id].collapsed.end()) {
                g.vertices[min.ntc_id].collapsed.push_back(c);
            }
        }

        // remove node_to_remove
        g.remove_node(min.ntr_id);
    }

    // create vector of size `n_communities`
    Communities communities(n_communities);
    
    for (int i = 0; i < g.vertices.size(); i++) {
        // update communities with precincts according to `collapsed` vectors
        communities[i].node_ids = (g.vertices.begin() + i).value().collapsed;
        communities[i].node_ids.push_back((g.vertices.begin() + i).key());
    }

    return communities;
}


Communities Gerrymandering::Geometry::get_initial_configuration(Graph graph, int n_communities) {
    /*
        @desc: determines a random list of community objects

        @params:
            `Geometry::Graph` graph: connection data
            `int` n_communities: number of communities to generate

        @return: `Communities` init config
    */

    srand(time(NULL));
    Communities cs = karger_stein(graph, n_communities);
    for (int i = 0; i < cs.size(); i++) {
        cs[i].update_shape(graph);
    }

    // Community state;
    // state.node_ids.resize(graph.vertices.size());
    // iota(state.node_ids.begin(), state.node_ids.end(), 0);
    // state.update_shape(graph);
    // double total = 0;
    
    // for (int i = 0; i < 500; i++) {
    //     // for (int x = 0; x < cs.size(); x++) {
    //     auto start = high_resolution_clock::now();
    //     get_compactness(state);
    //     auto stop = high_resolution_clock::now();
    //     total += duration_cast<microseconds> (stop - start).count();

    //     // }
    // }

    // total /= 500.0;
    // cout << total << endl;

    maximize(cs, graph, get_compactness);

    return cs;
}
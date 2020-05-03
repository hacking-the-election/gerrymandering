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


int Community::get_population() {
    int sum = 0;
    for (Precinct p : shape.precincts) {
        sum += p.pop;
    }
    return sum;
}


void Community::add_node(Node& node) {
    node_ids.push_back(node.id);
    this->shape.add_precinct(*node.precinct);
}


void Community::remove_node(Node& node) {
    node_ids.erase(remove(node_ids.begin(), node_ids.end(), node.id), node_ids.end());
    shape.remove_precinct(*node.precinct);
}


int get_population(Communities& c) {
    int sum = 0;
    for (Community cs : c) {
        sum += cs.get_population();
    }
    return sum;
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


void exchange_precinct(Graph& g, Communities& cs, int node_to_take, int community_to_take) {
    // moves a node from its community into `community_to_take`
    cs[community_to_take].add_node(g.vertices[node_to_take]);
    cs[g.vertices[node_to_take].community].remove_node(g.vertices[node_to_take]);
    g.vertices[node_to_take].community = community_to_take;
}


vector<int> get_takeable_precincts(Graph& g, Communities& c, int in) {

    // this can be sped up with `community` attr
    
    vector<int> takeable;
    for (int i = 0; i < g.vertices.size(); i++) {
        if (find(c[in].node_ids.begin(), c[in].node_ids.end(), i) == c[in].node_ids.end()) {
            bool has_neighbor = false;
            for (Edge e : g.vertices[i].edges) {
                if (find(c[in].node_ids.begin(), c[in].node_ids.end(), e[1]) != c[in].node_ids.end()) {
                    has_neighbor = true;
                    break;
                }
            }

            if (has_neighbor) {
                takeable.push_back(i);
            }
        }
    }

    return takeable;
}


vector<array<int, 2> > get_giveable_precincts(Graph& g, Communities& c, int in) {
    /*
        @desc: Find precincts bordering another community
    */

    vector<array<int, 2> > giveable;
    for (int i = 0; i < c[in].node_ids.size(); i++) {
        bool has_neighbor_outside = false;
        int community_neighbor = 0;

        for (Edge edge : g.vertices[c[in].node_ids[i]].edges) {
            if (g.vertices[edge[1]].community != in) {
            // if (find(c[in].node_ids.begin(), c[in].node_ids.end(), edge[1]) == c[in].node_ids.end()){
                has_neighbor_outside = true;
                community_neighbor = g.vertices[edge[1]].community;
                break;
            }
            // }
        }

        if (has_neighbor_outside) giveable.push_back({c[in].node_ids[i], community_neighbor});
    }

    return giveable;
}


// double average(Communities& communities, double (*measure)(Community&)) {
//     /*
//         @desc: find average `measure` of the `communities`

//         @params:
//             `Communities&` communities: list of communities measure
//             `double (*measure)(Community&)`: pointer to function of type double with param `Community&`
        
//         @return: `double` average measure
//     */

//     double sum = 0;
//     for (Community& c : communities) sum += measure(c);
//     return (sum / (double) communities.size());
// }


// void maximize(Communities& communities, Graph& graph, double (*measure)(Community&)) {
//     int n_communities = communities.size();
//     cout << "optimizing" << endl;

//     int iterations_since_best = 0;
//     Communities best = communities;

//     while (true) {
//         // choose worst community to modify
//         // vector<double> measures;
//         // measures.reserve(n_communities);
//         int smallest_index = 0;
//         double smallest_measure = measure(communities[0]);
        
//         for (int i = 1; i < communities.size(); i++) {
//             double x = measure(communities[i]);
//             if (x < smallest_measure) {
//                 smallest_measure = x;
//                 smallest_index = i;
//             }
//         }

//         cout << "the worst community is " << smallest_index << endl;

//         vector<int> takeable;
//         for (int i = 0; i < graph.vertices.size(); i++) {
//             if (find(communities[smallest_index].node_ids.begin(), communities[smallest_index].node_ids.end(), i) == communities[smallest_index].node_ids.end()) {
//                 bool has_neighbor = false;
//                 for (Edge e : graph.vertices[i].edges) {
//                     if (find(communities[smallest_index].node_ids.begin(), communities[smallest_index].node_ids.end(), e[1]) != communities[smallest_index].node_ids.end()) {
//                         has_neighbor = true;
//                         break;
//                     }
//                 }

//                 if (has_neighbor) {
//                     takeable.push_back(i);
//                 }
//             }
//         }

//         coordinate center = communities[smallest_index].shape.get_center();
//         double radius = sqrt(communities[smallest_index].shape.get_area() / PI);

//         for (int i = 0; i < takeable.size(); i++) {
//             if (point_in_circle(center, radius, graph.vertices[takeable[i]].precinct->get_center())) {
//                 cout << "adding takeable precinct " << i << endl;
//                 // communities[graph.vertices[takeable[i]].community].remove_node(graph.vertices[takeable[i]]);
//             }
//             else {
//                 cout << "not adding takeable precinct " << i << endl;
//             }
//         }


//         cout << takeable.size() << endl;

//         if (average(communities, measure) > average(best, measure)) {
//             cout << "found a better solution" << endl;
//             best = communities;
//             iterations_since_best = 0;
//         }
//         else {
//             cout << "not currently a better solution" << endl;
//         }
//     }

//     cout << "did not improve after " << ITERATION_LIMIT << " iterations, returning..." << endl;
// }


void optimize_population(Graph& g, Communities& communities, double range) {
    bool done = false;

    // find optimal populations
    int opt = get_population(communities) / communities.size();
    cout << opt << endl;
    array<int, 2> bounds = {opt - (int)((double) opt * (double)range), opt + (int)((double) opt * (double)range)};
    cout << bounds[0] << ", " << bounds[1] << endl;

    while (true) {
        // determine populations
        vector<int> pops;
        for (Community c : communities)
            pops.push_back(c.get_population());

        int sum = 0;
        for (int p : pops) sum += p;

        done = true;
        int worst_index = 0;
        int index = 0;
        double worst_margin = 0.0;
        bool lower = false;

        for (int p : pops) {
            if (p < bounds[0]) {
                done = false;
                if (abs(p - bounds[0]) > worst_margin) {
                    worst_margin = abs(p - bounds[0]);
                    worst_index = index;
                    lower = true;
                }
            }
            else if (p > bounds[1]) {
                done = false;
                if (bounds[1] - p > worst_margin) {
                    worst_margin = bounds[1] - p;
                    worst_index = index;
                    lower = false;
                }
            }

            index++;
        }

        if (done) break;

        if (lower) {
            vector<int> take = get_takeable_precincts(g, communities, worst_index);

            int x = 0;
            while (communities[worst_index].get_population() < bounds[0] && x < take.size()) {
                cout << "taking precinct " << take[x] << " for community " << worst_index << endl;
                exchange_precinct(g, communities, take[x], worst_index);
                x++;
            }
        }
        else {
            vector<array<int, 2> > give = get_giveable_precincts(g, communities, worst_index);
            int x = 0;
            while (communities[worst_index].get_population() > bounds[1] && x < give.size()) {
                cout << "GIVING PRECINCT" << endl;
                exchange_precinct(g, communities, give[x][0], give[x][1]);
            }

            cout << "remove preicncts" << endl;
        }
    }
}


Communities karger_stein(Graph& g1, int n_communities) {
    /*
        @desc: Partitions a graph according to the Karger-Stein algorithm

        @params:
            `Graph` g: the graph to partition
            `int` n_communities: the number of partitions

        @return: `Communities` list of id's corresponding to the partition
    */
    
    Graph g = g1;
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

        for (int x : (g.vertices.begin() + i).value().collapsed)
            g1.vertices[x].community = i;
        g1.vertices[(g.vertices.begin() + i).key()].community = i;
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
    cout << "getting init config..." << endl;
    Communities cs = karger_stein(graph, n_communities);
    for (int i = 0; i < cs.size(); i++) {
        cs[i].update_shape(graph);
    }

    cout << "got init config." << endl;
    // maximize(cs, graph, get_compactness);
    optimize_population(graph, cs, 0.01);

    return cs;
}

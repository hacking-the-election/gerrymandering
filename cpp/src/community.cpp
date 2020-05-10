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
using namespace hte::Geometry;
using namespace hte::Graphics;
using namespace chrono;

#define VERBOSE 1
#define DEBUG 0
#define ITERATION_LIMIT 40


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
    this->shape = Precinct_Group();    
    for (int i = 0; i < this->vertices.size(); i++) {
        this->shape.add_precinct(*(vertices.begin() + i).value().precinct);
    }
}


int Community::get_population() {
    int sum = 0;
    for (Precinct p : shape.precincts) {
        sum += p.pop;
    }
    return sum;
}


void Community::remove_node(int id) {
    this->remove_edges_to(id);
    this->shape.remove_precinct(*vertices[id].precinct);
    this->vertices.erase(id);
}


void Community::add_node(Node& node) {
    this->vertices.insert({node.id, node});
    this->shape.add_precinct(*node.precinct);
}


void hte::Geometry::save(Communities cs, std::string out) {
    
    string file = "[";
    for (Community c : cs) {
        file += "[";
        for (Precinct p : c.shape.precincts)
            file += "'" + p.shape_id + "', ";
        file.pop_back(); file.pop_back();
        file += "]";
    }

    file += "]";
    writef(file, out);
}


Communities hte::Geometry::load(std::string path, Graph& g) {
    
    string file = readf(path);
    file = file.substr(1, file.size() - 3);
    vector<string> strs = split(file, "[");
    Communities communities(strs.size() - 1);
    
    for (int x = 0; x < strs.size(); x++) {
        if (strs[x] != "" && strs[x] != " ") {
            vector<string> s = split(strs[x], ",");
            for (int i = 0; i < s.size(); i++) {
                if (s[i] != " " && s[i] != "") {
                    string mod = s[i];
                    mod = mod.substr(mod.find("'") + 1, mod.size() - mod.find("'") - 1);
                    mod = mod.substr(0, mod.find("'"));

                    for (int n = 0; n < g.vertices.size(); n++) {
                        if ((g.vertices.begin() + n).value().precinct->shape_id == mod) {
                            communities[x - 1].add_node((g.vertices.begin() + n).value());
                            g.vertices[(g.vertices.begin() + n).key()].community = x - 1;
                        }
                    }
                }
            }
            // communities[x - 1].update_shape(g);
        }
    }

    return communities;
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

    // cout << mb.squared_radius() << ", " << (double)community.shape.get_area() << endl;
    double t = (double)community.shape.get_area() / (mb.squared_radius() * PI);
    return t;
} 


void exchange_precinct(Graph& g, Communities& cs, int node_to_take, int community_to_take) {
    // moves a node from its community into `community_to_take`
    cs[community_to_take].add_node(g.vertices[node_to_take]);
    cs[g.vertices[node_to_take].community].remove_node(node_to_take);
    g.vertices[node_to_take].community = community_to_take;
}


vector<int> get_takeable_precincts(Graph& g, Communities& c, int in) {
    vector<int> takeable;

    for (int i = 0; i < g.vertices.size(); i++) {
        if (g.vertices[i].community != in) {
            bool has_neighbor = false;
            for (Edge e : g.vertices[i].edges) {
                if (g.vertices[e[1]].community == in) {
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
        @params:
            `Graph&` g: graph for reference
            `Communities&` c: communities to check
            `int` in: index of community to check

        @return: `vector<array<int, 2>` giveable precincts, and the community to give to
    */

    vector<array<int, 2> > giveable;
    for (int i = 0; i < c[in].vertices.size(); i++) {
        // bool has_neighbor_outside = false;
        // int community_neighbor = 0;
        Graph copy = c[in];

        for (Edge edge : g.vertices[(c[in].vertices.begin() + i).key()].edges) {
            // if the node borders a precinct not in the community
            if (g.vertices[edge[1]].community != in) {
                // this arbitrarily decides to give the precinct
                // to the first edge's community that goes outside
                // communities[c]
                if (remove_edges_to(c[in], (c[in].vertices.begin() + i).key()).get_num_components() > 2) {
                    giveable.push_back({(c[in].vertices.begin() + i).key(), g.vertices[edge[1]].community});
                    break;
                }
            }
        }
    }

    return giveable;
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
    // cout << "sum " << sum << endl;
    return (sum / (double) communities.size());
}


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



void optimize_compactness(Communities& communities, Graph& graph, double (*measure)(Community&)) {

    int n_communities = communities.size();
    int iterations_since_best = 0;
    Communities best = communities;

    while (iterations_since_best < ITERATION_LIMIT) {
        int smallest_index = 0;//rand_num(0, communities.size() - 1);//0;
        double smallest_measure = measure(communities[0]);

        for (int i = 1; i < communities.size(); i++) {
            double x = measure(communities[i]);
            if (x < smallest_measure) {
                smallest_measure = x;
                smallest_index = i;
            }
        }

        coordinate center = communities[smallest_index].shape.get_center();
        double radius = sqrt(communities[smallest_index].shape.get_area() / PI);
        int num_exchanged = 0;

        vector<int> takeable = get_takeable_precincts(graph, communities, smallest_index);

        for (int t : takeable) {
            if (point_in_circle(center, radius, graph.vertices[t].precinct->get_center())) {
                cout << "taking precinct " << t << " for community " << smallest_index << endl;
                exchange_precinct(graph, communities, t, smallest_index);
                cout << communities[smallest_index].vertices.size() << endl;
                cout << communities[smallest_index].shape.precincts.size() << endl;
                num_exchanged++;
            }
        }

        vector<array<int, 2> > giveable = get_giveable_precincts(graph, communities, smallest_index);
        for (array<int, 2> g : giveable) {
            if (!point_in_circle(center, radius, graph.vertices[g[0]].precinct->get_center())) {
                cout << "giving precinct " << g[0] << " to community " << g[1] << endl;
                exchange_precinct(graph, communities, g[0], g[1]);
                cout << communities[smallest_index].vertices.size() << endl;
                cout << communities[smallest_index].shape.precincts.size() << endl;
                num_exchanged++;
            }
        }

        cout << takeable.size() << ", " << giveable.size() << endl;

        if (average(communities, measure) > average(best, measure)) {
            best = communities;
            iterations_since_best = 0;
        }
        else {
            iterations_since_best++;
        }

        if (num_exchanged == 0) {
            cout << "nothing happened this iteration" << endl;
            break;
        }

        // cout << "drawing" << endl;
        // Canvas canvas(900, 900);
        // cout << "drawing" << endl;
        // canvas.add_outlines(to_outline(communities));
        // cout << "drawing" << endl;
        // canvas.draw_to_window();
        // cout << "drawing" << endl;
        cout << average(communities, measure) << endl;
        // for (int i = 0; i < communities.size(); i++) {
        //     communities[i].update_shape(graph);
        // }
    }

    communities = best;
    
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(communities));
    canvas.draw_to_window();

    cout << "did not improve after " << ITERATION_LIMIT << " iterations, returning..." << endl;
    cout << average(communities, measure) << endl;
}


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
        bool lower = true;

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
            cout << communities[worst_index].get_population() << ", " << bounds[1] << endl;
            cout << give.size() << endl;
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
        (g.vertices.begin() + i).value().collapsed.push_back((g.vertices.begin() + i).key());
        communities[i].vertices = g1.get_induced_subgraph((g.vertices.begin() + i).value().collapsed).vertices;

        for (int x : (g.vertices.begin() + i).value().collapsed) {
            g1.vertices[x].community = i;
        }
    }

    return communities;
}


Communities hte::Geometry::get_initial_configuration(Graph& graph, int n_communities) {
    /*
        @desc: determines a random list of community objects

        @params:
            `Geometry::Graph` graph: connection data
            `int` n_communities: number of communities to generate

        @return: `Communities` init config
    */

    srand(time(NULL));
    // Communities cs;

    Communities cs = load("test.txt", graph);
    // Communities cs = karger_stein(graph, n_communities);
    
    for (int i = 0; i < cs.size(); i++) {
        cs[i].update_shape(graph);
    }

    // cout << "got init config." << endl;
    optimize_compactness(cs, graph, get_compactness);
    // optimize_population(graph, cs, 0.01);

    return cs;
}

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
        this->shape.add_precinct(*graph.vertices[(vertices.begin() + i).key()].precinct);
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
    for (Edge edge : node.edges) {
        if (vertices.find(edge[1]) != vertices.end()) {
            this->add_edge(edge);
        }
    }

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
    
    if (cs[g.vertices[node_to_take].community].vertices.size() == 1) {
        return;
    }
    else if (remove_edges_to(cs[g.vertices[node_to_take].community], node_to_take).get_num_components() > 2) {
        return;
    }
    
    // cout << "removex node" << endl;
    cs[g.vertices[node_to_take].community].remove_node(node_to_take);
    cs[community_to_take].add_node(g.vertices[node_to_take]);
    g.vertices[node_to_take].community = community_to_take;
}


vector<int> get_takeable_precincts(Graph& g, Communities& c, int in) {
    vector<int> takeable;

    for (int i = 0; i < g.vertices.size(); i++) {
        if (g.vertices[i].community != in) {
            for (Edge e : g.vertices[i].edges) {
                if (g.vertices[e[1]].community == in) {
                    takeable.push_back(i);
                    break;
                }
            }
        }
    }

    // sort takeable by respective xcoord
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
                giveable.push_back({(c[in].vertices.begin() + i).key(), g.vertices[edge[1]].community});
                break;
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

//         coordinate center = communities[smallest_index].shape.get_centroid();
//         double radius = sqrt(communities[smallest_index].shape.get_area() / PI);

//         for (int i = 0; i < takeable.size(); i++) {
//             if (point_in_circle(center, radius, graph.vertices[takeable[i]].precinct->get_centroid())) {
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
        int smallest_index = 0;
        double smallest_measure = measure(communities[0]);

        for (int i = 1; i < communities.size(); i++) {
            double x = measure(communities[i]);
            if (x < smallest_measure) {
                smallest_measure = x;
                smallest_index = i;
            }
        }


        coordinate center = communities[smallest_index].shape.get_centroid();
        double radius = sqrt(communities[smallest_index].shape.get_area() / PI);
        int num_exchanged = 0;
        vector<array<int, 2> > giveable = get_giveable_precincts(graph, communities, smallest_index);

        for (array<int, 2> g : giveable) {
            if (!point_in_circle(center, radius, graph.vertices[g[0]].precinct->get_centroid())) {
                exchange_precinct(graph, communities, g[0], g[1]);
                num_exchanged++;

                // Canvas canvas(900, 900);
                // canvas.add_outlines(to_outline(communities));
                // canvas.save_img_to_anim(ImageFmt::BMP, "output");
            }
        }

        vector<int> takeable = get_takeable_precincts(graph, communities, smallest_index);

        for (int t : takeable) {
            if (point_in_circle(center, radius, graph.vertices[t].precinct->get_centroid())) {
                exchange_precinct(graph, communities, t, smallest_index);
                num_exchanged++;


                // Canvas canvas(900, 900);
                // canvas.add_outlines(to_outline(communities));
                // canvas.save_img_to_anim(ImageFmt::BMP, "output");
            }
        }

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

        cout << average(communities, measure) << endl;
    }

    communities = best;

    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(communities));
    canvas.draw_to_window();

    cout << "did not improve after " << ITERATION_LIMIT << " iterations, returning..." << endl;
    cout << average(communities, measure) << endl;
}


void optimize_population(Communities& communities, Graph& g, double range) {

    // find optimal populations
    range /= 2.0;
    int opt = get_population(communities) / communities.size();
    int bounds[2] = {(int)(opt - (range * (double)opt)), (int)(opt + (range * (double)opt))};

    vector<int> pops = {communities[0].get_population()};
    int worst_index = 0;
    int worst_difference = abs(opt - pops[0]);
    cout << " the optimal population is " << opt << endl; 


    for (int i = 1; i < communities.size(); i++) {
        pops.push_back(communities[i].get_population());
        if (abs(opt - pops[i]) > worst_difference) {
            worst_difference = abs(opt - pops[i]);
            worst_index = i;
        }
    }

    while (true) {
        // determine populations
        // cout << "current worst community is " << pops[worst_index] << endl;
        int x = 0;

        if (pops[worst_index] < opt) {
            cout << "taking precincts to get to " << opt << endl;
            vector<int> take = get_takeable_precincts(g, communities, worst_index);
            while (communities[worst_index].get_population() < opt) {
                if (x == take.size()) {
                    x = 0;
                    take = get_takeable_precincts(g, communities, worst_index);
                }

                exchange_precinct(g, communities, take[x], worst_index);
                x++;
            }

            Canvas canvas(900, 900);
            canvas.add_outlines(to_outline(communities));
            canvas.save_img_to_anim(ImageFmt::BMP, "output");

        }
        else {
            cout << "giving precincts to get to " << opt << endl;
            vector<array<int, 2> > give = get_giveable_precincts(g, communities, worst_index);
            while (communities[worst_index].get_population() > opt) {
                if (x == give.size()) {
                    x = 0;
                    give = get_giveable_precincts(g, communities, worst_index);
                }
                exchange_precinct(g, communities, give[x][0], give[x][1]);
                x++;
            }

            Canvas canvas(900, 900);
            canvas.add_outlines(to_outline(communities));
            canvas.save_img_to_anim(ImageFmt::BMP, "output");
        }

        cout << "updated worst with pop of " << communities[worst_index].get_population() << endl;

        pops.clear();
        pops = {communities[0].get_population()};
        worst_index = 0;
        worst_difference = abs(opt - pops[0]);


        for (int i = 1; i < communities.size(); i++) {
            pops.push_back(communities[i].get_population());
            if (abs(opt - pops[i]) > worst_difference) {
                worst_difference = abs(opt - pops[i]);
                worst_index = i;
            }
        }


        for (int i = 0; i < communities.size(); i++) {
            communities[i].update_shape(g);
        }

        for (int pop : pops) cout << pop << ", ";
        cout << endl;

        if (worst_difference < (int)(range * (double)opt)) {
            cout << "got it bois!" << endl;
            Canvas canvas(900, 900);
            canvas.add_outlines(to_outline(communities));
            canvas.draw_to_window();

            break;
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
    Communities cs = load("config.txt", graph);
    // Communities cs = karger_stein(graph, n_communities);
    
    for (int i = 0; i < cs.size(); i++) {
        cs[i].update_shape(graph);
    }

    optimize_population(cs, graph, 0.01);
    // optimize_compactness(cs, graph);

    return cs;
}

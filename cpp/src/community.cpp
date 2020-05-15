/*===============================================
 community.cpp:                        k-vernooy
 last modified:                     Fri, Feb 28
 
 Definition of the community-generation algorithm
 quantifying gerrymandering and redistricting
 Detailed documentation can be found in the /docs
 root folder of this repository.
 
 Visit https://hacking-the-election.github.io for
 more information on this project.
 
 Our data sources for actually executing this
 algorithm can be seen at /data or on GitHub:
 https://github.com/hacking-the-election/data
 This algorithm has been implemented in C++ and in
 Python. If you're interested in contributing to the
 project, please contact us at
 hacking.the.election@gmail.com
===============================================*/


#include <math.h>    // for rounding functions
#include <numeric>   // include std::iota
#include <iostream>  // std::cout and std::endl
#include <random>
#include <chrono>
#include <iomanip>

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


double get_partisanship_stdev(Community& community) {
    double average = 0;
    map<POLITICAL_PARTY, vector<double> > total_data;

    for (auto& pair : community.vertices) {
        int total = 0;
        for (auto& p2 : pair.second.precinct->voter_data) {
            if (p2.first != POLITICAL_PARTY::TOTAL && p2.second >= 0) total += p2.second;
        }

        for (auto& p2 : pair.second.precinct->voter_data) {
            if (p2.first != POLITICAL_PARTY::TOTAL && p2.second >= 0) {
                if (total != 0) {
                    total_data[p2.first].push_back(((double)p2.second / (double)total));
                }
            }
        }
    }

    for (auto& pair : total_data) {
        average += get_stdev(pair.second);
    }
    
    return (average / total_data.size());
}


double get_population_stdev(Community& community) {
    vector<double> pops;
    for (auto& pair : community.vertices) {
        pops.push_back(pair.second.precinct->pop);
    }

    return get_stdev(pops);
}


bool exchange_precinct(Graph& g, Communities& cs, int node_to_take, int community_to_take) {
    // moves a node from its community into `community_to_take`
    
    if (cs[g.vertices[node_to_take].community].vertices.size() == 1) {
        return false;
    }
    else if (remove_edges_to(cs[g.vertices[node_to_take].community], node_to_take).get_num_components() > 2) {
        return false;
    }
    
    // cout << "moving " << node_to_take << " from " << g.vertices[node_to_take].community << " to community " << community_to_take << endl;
    // cout << (cs[g.vertices[node_to_take].community].vertices.find(node_to_take) != cs[g.vertices[node_to_take].community].vertices.end()) << endl;
    // if (cs[g.vertices[node_to_take].community].vertices.find(node_to_take) == cs[g.vertices[node_to_take].community].vertices.end()) {
    //     cout << node_to_take << " not in " << g.vertices[node_to_take].community << endl;

    //     for (int i = 0; i < cs.size(); i++) {
    //         for (auto& n : cs[i].vertices) {
    //             if (n.second.id == node_to_take) {
    //                 cout << "actually in " << i << endl;
    //                 Canvas canvas(900, 900);
    //                 canvas.add_outlines(to_outline(cs));
    //                 Outline o(n.second.precinct->hull);
    //                 o.style().fill(RGB_Color(-1,-1,-1)).thickness(4).outline(RGB_Color(0,0,0));
    //                 canvas.add_outline(o);
    //                 canvas.add_outlines(to_outline(cs[i]));
    //                 canvas.add_outlines(to_outline(cs[g.vertices[node_to_take].community]));
    //                 canvas.draw_to_window();
    //             }
    //         }
    //     }
    // }

    // cout << node_to_take << " community: " << cs[community_to_take].vertices[node_to_take].community << endl;
    cs[g.vertices[node_to_take].community].remove_node(node_to_take);
    cs[community_to_take].add_node(g.vertices[node_to_take]);
    g.vertices[node_to_take].community = community_to_take;
    // cs[community_to_take].vertices[node_to_take].community = community_to_take;
    return true;
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
        for (Edge edge : g.vertices[(c[in].vertices.begin() + i).key()].edges) {
            // if the node borders a precinct not in the community
            if (g.vertices[edge[1]].community != in) {
                giveable.push_back({
                    (c[in].vertices.begin() + i).key(), g.vertices[edge[1]].community
                });
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


double worst(Communities& communities, double (*measure)(Community&)) {
    double worst = measure(communities[0]);
    int w_ind = 0;

    for (int i = 1; i < communities.size(); i++) {
        double meas = measure(communities[i]);
        if (meas < worst) {
            w_ind = i;
            worst = meas;
        }
    }

    return worst;
}


void optimize_compactness(Communities& communities, Graph& graph, double (*measure)(Community&)) {

    int community_to_modify = 0;
    int sub_modifications = 20;
    double area = 0;
    for (Community c : communities) area += c.shape.get_area();
    area /= communities.size();

    for (int i = 0; i < 40; i++) {
        coordinate center = communities[community_to_modify].shape.get_centroid();
        double radius = sqrt(communities[community_to_modify].shape.get_area() / PI);

        for (int i = 0; i < sub_modifications; i++) {
            vector<array<int, 2> > giveable = get_giveable_precincts(graph, communities, community_to_modify);
            for (array<int, 2> g : giveable) {
                if (!point_in_circle(center, radius, graph.vertices[g[0]].precinct->get_centroid())) {
                    exchange_precinct(graph, communities, g[0], g[1]);
                }
            }

            vector<int> takeable = get_takeable_precincts(graph, communities, community_to_modify);
            for (int t : takeable) {
                if (point_in_circle(center, radius, graph.vertices[t].precinct->get_centroid())) {
                    exchange_precinct(graph, communities, t, community_to_modify);
                }
            }
        }

        community_to_modify++;
        if (community_to_modify == communities.size()) community_to_modify = 0;
    }

    cout << "drawing" << endl;
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(communities));
    canvas.add_outlines(to_outline(communities[community_to_modify]));
    canvas.draw_to_window();
    // int n_communities = communities.size();
    // int iterations_since_best = 0;
    // Communities best = communities;
    // double best_measure = average(communities, measure);

    // while (iterations_since_best < ITERATION_LIMIT) {
    //     int smallest_index = 0;//rand_num(0, communities.size() - 1);//0;
    //     double smallest_measure = measure(communities[0]);

    //     for (int i = 1; i < communities.size(); i++) {
    //         double x = measure(communities[i]);
    //         if (x < smallest_measure) {
    //             smallest_measure = x;
    //             smallest_index = i;
    //         }
    //     }


    //     coordinate center = communities[smallest_index].shape.get_centroid();
    //     double radius = sqrt(communities[smallest_index].shape.get_area() / 3.141592653);

    //     int num_exchanged = 0;
    //     vector<array<int, 2> > giveable = get_giveable_precincts(graph, communities, smallest_index);

    //     for (array<int, 2> g : giveable) {
    //         if (!point_in_circle(center, radius, graph.vertices[g[0]].precinct->get_centroid())) {
    //             if (exchange_precinct(graph, communities, g[0], g[1])) {
    //                 num_exchanged++;
    //             }
    //         }
    //     }

    //     vector<int> takeable = get_takeable_precincts(graph, communities, smallest_index);

    //     for (int t : takeable) {
    //         if (point_in_circle(center, radius, graph.vertices[t].precinct->get_centroid())) {
    //             if (exchange_precinct(graph, communities, t, smallest_index)) {
    //                 num_exchanged++;
    //             }
    //         }
    //     }

    //     double cur_measure = average(communities, measure);
    //     cout << cur_measure << endl;

    //     if (cur_measure > best_measure) {
    //         best = communities;
    //         best_measure = cur_measure;
    //         iterations_since_best = 0;
    //     }
    //     else {
    //         iterations_since_best++;
    //     }

    //     if (num_exchanged == 0) {
    //         break;
    //     }
    // }

    // communities = best;
    // cout << average(communities, measure) << endl;
}


void optimize_population(Communities& communities, Graph& g, double range) {

    // find optimal populations
    range /= 2.0;
    int ideal_pop = get_population(communities) / communities.size();

    vector<int> pops = {communities[0].get_population()};
    int worst_index = 0;
    int worst_difference = abs(ideal_pop - pops[0]);

    for (int i = 1; i < communities.size(); i++) {
        pops.push_back(communities[i].get_population());
        if (abs(ideal_pop - pops[i]) > worst_difference) {
            worst_difference = abs(ideal_pop - pops[i]);
            worst_index = i;
        }
    }

    while (true) {
        // determine populations
        int x = 0;

        if (pops[worst_index] < ideal_pop) {
            vector<int> take = get_takeable_precincts(g, communities, worst_index);
            std::shuffle(take.begin(), take.end(), std::random_device());

            while (communities[worst_index].get_population() < (ideal_pop - ((double)ideal_pop * range))) {
                if (x == take.size()) {
                    x = 0;
                    take = get_takeable_precincts(g, communities, worst_index);
                    std::shuffle(take.begin(), take.end(), std::random_device());
                }

                cout << "taking precinct " << take[x] << " for community " << worst_index << endl;
                exchange_precinct(g, communities, take[x], worst_index);
                x++;
            }
        }
        else {
            vector<array<int, 2> > give = get_giveable_precincts(g, communities, worst_index);
            std::shuffle(give.begin(), give.end(), std::random_device());
            while (communities[worst_index].get_population() > (ideal_pop + ((double)ideal_pop * range))) {
                if (x == give.size()) {
                    x = 0;
                    give = get_giveable_precincts(g, communities, worst_index);
                    std::shuffle(give.begin(), give.end(), std::random_device());
                }

                cout << "giving precinct " << give[x][0] << " to community " << give[x][1] << endl;
                exchange_precinct(g, communities, give[x][0], give[x][1]);
                x++;
            }
        }

        pops.clear();
        pops = {communities[0].get_population()};
        worst_index = 0;
        worst_difference = abs(ideal_pop - pops[0]);


        for (int i = 1; i < communities.size(); i++) {
            pops.push_back(communities[i].get_population());
            if (abs(ideal_pop - pops[i]) > worst_difference) {
                worst_difference = abs(ideal_pop - pops[i]);
                worst_index = i;
            }
        }


        // cout << worst_difference << ", " << range * (double)ideal_pop << endl;
        // Canvas canvas(900, 900);
        // canvas.add_outlines(to_outline(communities));
        // canvas.draw_to_window();

        if (worst_difference < (int)(range * (double)ideal_pop)) {
            // Canvas canvas(900, 900);
            // canvas.add_outlines(to_outline(communities));
            // canvas.draw_to_window();
            break;
        }
    }
}

void maximize(Communities& communities, Graph& graph, double (*measure)(Community&), bool minimize = false);

void maximize(Communities& communities, Graph& graph, double (*measure)(Community&), bool minimize) {
    int n_communities = communities.size();
    int iterations_since_best = 0;
    Communities best = communities;

    while (iterations_since_best < ITERATION_LIMIT) {
        // choose worst community to modify

        int smallest_index = 0;
        double smallest_measure = measure(communities[0]);
        
        for (int i = 1; i < communities.size(); i++) {
            double x = measure(communities[i]);
            if (x < smallest_measure) {
                smallest_measure = x;
                smallest_index = i;
            }
        }

        vector<array<int, 2> > giveable = get_giveable_precincts(graph, communities, smallest_index);
        
        for (array<int, 2> g : giveable) {
            double before = average(communities, measure);
            Communities before_c = communities;
            Graph before_g = graph;

            if (exchange_precinct(graph, communities, g[0], g[1])) {
                // num_exchanged++;
                if (minimize) {
                    if (average(communities, measure) > before) {
                        // undo the exchange
                        communities = before_c;
                        graph = before_g;
                    }
                }
                else {
                    if (average(communities, measure) < before) {
                        // undo the exchange
                        communities = before_c;
                        graph = before_g;
                    }
                }
            }
        }

        vector<int> takeable = get_takeable_precincts(graph, communities, smallest_index);
        for (int t : takeable) {
            double before = average(communities, measure);
            Communities before_c = communities;
            Graph before_g = graph;

            if (exchange_precinct(graph, communities, t, smallest_index)) {
                if (minimize) {
                    if (average(communities, measure) > before) {
                        // undo the exchange
                        communities = before_c;
                        graph = before_g;
                    }
                }
                else {
                    if (average(communities, measure) < before) {
                        // undo the exchange
                        communities = before_c;
                        graph = before_g;
                    }
                }
            }
        }

        if (minimize) {
            if (average(communities, measure) < average(best, measure)) {
                best = communities;
                // cout << average(best, measure) << endl;
                iterations_since_best = 0;
            }
            else {
                iterations_since_best++;
            }
        }
        else {
            if (average(communities, measure) > average(best, measure)) {
                // cout << "found a better solution" << endl;
                best = communities;
                iterations_since_best = 0;
            }
            else {
                iterations_since_best++;
            }
        }
    }

    communities = best;
    // Canvas canvas(900, 900);
    // canvas.add_outlines(to_outline(communities));
    // canvas.draw_to_window();
    
    // cout << "did not improve after " << ITERATION_LIMIT << " iterations, returning..." << endl;
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


Communities hte::Geometry::get_communities(Graph& graph, int n_communities) {
    /*
        @desc: determines a random list of community objects

        @params:
            `Geometry::Graph` graph: connection data
            `int` n_communities: number of communities to generate

        @return: `Communities` init config
    */

    srand(time(NULL));
    // Communities cs = load("config.txt", graph);
    Communities cs = karger_stein(graph, n_communities);
    cout << "got karger stein" << endl;
    for (int i = 0; i < cs.size(); i++) {
        cs[i].update_shape(graph);
    }

    cout << "updated, drawing" << endl;
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(cs));
    canvas.draw_to_window();
    
    cout << "init config: " << average(cs, get_partisanship_stdev) << endl;
    // for (int i = 0; i < 10; i++) {
    //     cout << "optimizing pop" << endl;
    //     optimize_population(cs, graph, 0.01);
    //     cout << "optimizing compactness" << endl;
    optimize_compactness(cs, graph, get_compactness);
    // }

    canvas.clear();
    canvas.add_outlines(to_outline(cs));
    canvas.draw_to_window();
    
    // maximize(cs, graph, get_partisanship_stdev, true);
    // maximize(cs, graph, get_population_stdev, true);
    // optimize_compactness(cs, graph, get_compactness);
    return cs;
}

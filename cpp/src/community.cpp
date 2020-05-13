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


double get_partisanship_stdev(Community& community) {
    double average = 0;

    map<POLITICAL_PARTY, vector<int> > total_data;

    for (auto& pair : community.vertices) {
        for (auto& p2 : pair.second.precinct->voter_data) {
            total_data[p2.first].push_back(p2.second);
        }
    }

    for (auto& pair : total_data) {
        average += get_stdev(pair.second);
    }

    return (average / total_data.size());
}


double get_population_stdev(Community& community) {
    vector<int> pops;
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
    
    // cout << "removex node" << endl;
    cs[g.vertices[node_to_take].community].remove_node(node_to_take);
    cs[community_to_take].add_node(g.vertices[node_to_take]);
    g.vertices[node_to_take].community = community_to_take;

    return true;
}


void sort_by_xcoord(Graph& g, vector<int>& x) {
    vector<Node> nodes(x.size());
    for (int i = 0; i < x.size(); i++) {
        nodes[i] = g.vertices[x[i]];
    }

    sort(nodes.begin(), nodes.end());
    for (int i = 0; i < nodes.size(); i++) {
        x[i] = nodes[i].id;
    }
}


void sort_by_xcoord(Graph& g, vector<array<int, 2> >& x) {
    vector<Node> nodes(x.size());
    for (int i = 0; i < x.size(); i++) {
        nodes[i] = g.vertices[x[i][0]];
        nodes[i].id = x[i][0];
        nodes[i].community = x[i][1];
    }

    sort(nodes.begin(), nodes.end());
    for (int i = 0; i < nodes.size(); i++) {
        x[i][0] = nodes[i].id;
        x[i][1] = nodes[i].community;
    }
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
    sort_by_xcoord(g, takeable);
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

        int community_neighbor = 0;
        long int community_neighbor_pop = 100000000;
        bool has_neighbor_outside = false;

        for (Edge edge : g.vertices[(c[in].vertices.begin() + i).key()].edges) {
            // if the node borders a precinct not in the community
            if (g.vertices[edge[1]].community != in) {
                has_neighbor_outside = true;

                if (c[g.vertices[edge[1]].community].get_population() < community_neighbor_pop) {
                    community_neighbor = g.vertices[edge[1]].community;
                    community_neighbor_pop = c[g.vertices[edge[1]].community].get_population();
                }
            }
        }

        if (has_neighbor_outside) {
            giveable.push_back({
                (c[in].vertices.begin() + i).key(), community_neighbor
            });
        }
    }

    sort_by_xcoord(g, giveable);
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

    int n_communities = communities.size();
    int iterations_since_best = 0;
    Communities best = communities;
    double best_measure = worst(communities, measure);

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
        double radius = sqrt(communities[smallest_index].shape.get_area() / 3.141592653);

        int num_exchanged = 0;
        vector<array<int, 2> > giveable = get_giveable_precincts(graph, communities, smallest_index);

        for (array<int, 2> g : giveable) {
            if (!point_in_circle(center, radius, graph.vertices[g[0]].precinct->get_centroid())) {
                // cout << "giving precinct " << graph.vertices[g[0]].precinct->shape_id << " to community " << g[1] << endl;
                if (exchange_precinct(graph, communities, g[0], g[1])) {
                    num_exchanged++;

                    // Canvas canvas(500, 500);
                    // canvas.add_outlines(to_outline(communities));
                    // canvas.add_outlines(to_outline(communities[smallest_index]));
                    // canvas.save_img_to_anim(ImageFmt::BMP, "output");
                }
            }
        }

        vector<int> takeable = get_takeable_precincts(graph, communities, smallest_index);

        for (int t : takeable) {
            if (point_in_circle(center, radius, graph.vertices[t].precinct->get_centroid())) {
                // cout << "taking precinct " << t << " for community " << smallest_index << endl;
                if (exchange_precinct(graph, communities, t, smallest_index)) {
                    num_exchanged++;

                    // Canvas canvas(500, 500);
                    // canvas.add_outlines(to_outline(communities));
                    // canvas.add_outlines(to_outline(communities[smallest_index]));
                    // canvas.save_img_to_anim(ImageFmt::BMP, "output");
                }
            }
        }

        double cur_measure = worst(communities, measure);
        if (cur_measure > best_measure) {
            best = communities;
            best_measure = cur_measure;
            iterations_since_best = 0;
        }
        else {
            iterations_since_best++;
        }

        if (num_exchanged == 0) {
            cout << "nothing happened this iteration" << endl;
            break;
        }

        cout << cur_measure << endl;
    }

    communities = best;

    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(communities));
    canvas.draw_to_window();

    cout << "did not improve after " << ITERATION_LIMIT << " iterations, returning..." << endl;
    cout << worst(communities, measure) << endl;
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
            cout << "before\t" << communities[worst_index].get_population() << endl;
            vector<int> take = get_takeable_precincts(g, communities, worst_index);
            while (communities[worst_index].get_population() < ideal_pop) {
                if (x == take.size()) {
                    x = 0;
                    take = get_takeable_precincts(g, communities, worst_index);
                }

                if (exchange_precinct(g, communities, take[x], worst_index)) {
                    Canvas canvas(900, 900);
                    canvas.add_outlines(to_outline(communities));
                    canvas.save_img_to_anim(ImageFmt::BMP, "output");
                }

                x++;
            }

        }
        else {
            cout << "before\t" << communities[worst_index].get_population() << endl;
            vector<array<int, 2> > give = get_giveable_precincts(g, communities, worst_index);
            while (communities[worst_index].get_population() > ideal_pop) {
                if (x == give.size()) {
                    x = 0;
                    give = get_giveable_precincts(g, communities, worst_index);
                }

                if (exchange_precinct(g, communities, give[x][0], give[x][1])) {
                    Canvas canvas(900, 900);
                    canvas.add_outlines(to_outline(communities));
                    canvas.save_img_to_anim(ImageFmt::BMP, "output");
                }
                
                x++;
            }

        }

        cout << "after\t" << communities[worst_index].get_population() << endl;

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


        if (worst_difference < (int)(range * (double)ideal_pop)) {
            Canvas canvas(900, 900);
            canvas.add_outlines(to_outline(communities));
            canvas.draw_to_window();
            break;
        }
    }
}


void maximize(Communities& communities, Graph& graph, double (*measure)(Community&)) {
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
            if (exchange_precinct(graph, communities, g[0], g[1])) {
                // num_exchanged++;
                if (average(communities, measure) < before) {
                    // undo the exchange
                    communities = before_c;
                }
                else {
                    cout << "keeping exchange" << endl;
                }
            }
        }


        vector<int> takeable = get_takeable_precincts(graph, communities, smallest_index);
        for (int t : takeable) {
            double before = average(communities, measure);
            Communities before_c = communities;
            if (exchange_precinct(graph, communities, t, smallest_index)) {
                if (average(communities, measure) < before) {
                    // undo the exchange
                    communities = before_c;
                }
                else {
                    cout << "keeping exchange" << endl;
                }
            }
        }

        if (average(communities, measure) > average(best, measure)) {
            cout << "found a better solution" << endl;
            best = communities;
            iterations_since_best = 0;
        }
        else {
            iterations_since_best++;
        }
    }

    communities = best;
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(communities));
    canvas.draw_to_window();
    
    cout << "did not improve after " << ITERATION_LIMIT << " iterations, returning..." << endl;
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
    // Canvas canvas(900, 900);
    // canvas.add_outlines(to_outline(graph));
    // canvas.save_image(ImageFmt::BMP, "vt_graph");

    // Communities cs = karger_stein(graph, n_communities);
    
    for (int i = 0; i < cs.size(); i++) {
        cs[i].update_shape(graph);
    }

    optimize_compactness(cs, graph, get_compactness);
    // optimize_population(cs, graph, 0.01);

    return cs;
}

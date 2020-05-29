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
#define SPEED_OPT 0
// #define WRITE_OUTPUT

#define MAX_TIME 13500
#define ITERATION_LIMIT 20
#define MIN_PERCENT_PRECINCTS 0.03
#define MIN_PERCENT_PRECINCTS_STDEV 0.5

void drawc(Communities&);

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


class NodePtr {
    public:
        int id;
        int id_x;
        int degree;
    friend bool operator< (const NodePtr& l1, const NodePtr& l2);
};


bool operator< (const NodePtr& l1, const NodePtr& l2) {
    return (l1.degree < l2.degree);
}


void Community::update_shape(Graph& graph) {
    this->shape = Precinct_Group();    
    for (int i = 0; i < this->vertices.size(); i++) {
        this->shape.add_precinct(*graph.vertices[(vertices.begin() + i).key()].precinct);
    }
}


void sort_by_degree(Graph& g, Communities& cs, vector<int>& v) {
    vector<NodePtr> nodes;
    for (int x : v) {
        NodePtr n;
        n.id = x;
        n.degree = cs[g.vertices[x].community].vertices[x].edges.size();
        nodes.push_back(n);
    }

    sort(nodes.begin(), nodes.end());
    for (int i = 0; i < nodes.size(); i++) {
        v[i] = nodes[i].id;
    }
}


void sort_by_degree(Graph& g, Communities& cs, vector<vector<int> >& v) {
    vector<NodePtr> nodes;
    for (vector<int> x : v) {
        NodePtr n;
        n.id = x[0];
        n.id_x = x[1];
        n.degree = cs[g.vertices[x[0]].community].vertices[x[0]].edges.size();
        nodes.push_back(n);
    }

    sort(nodes.begin(), nodes.end());
    for (int i = 0; i < nodes.size(); i++) {
        v[i][0] = nodes[i].id;
        v[i][1] = nodes[i].id_x;
    }
}


int get_num_communities_changed(Graph& before, Graph& after) {
    int tot = 0;
    for (int i = 0; i < before.vertices.size(); i++) {
        if ((before.vertices.begin() + i).value().community != (after.vertices.begin() + i).value().community) {
            tot++;
        }
    }
    return tot;
}


int Community::get_population() {
    int sum = 0;
    for (Precinct p : shape.precincts) sum += p.pop;
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
        file += "], ";
    }

    file.pop_back(); file.pop_back();
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

    
    // return (0.4);
    coordinate center = community.shape.get_centroid();
    double farthest = 0;

    for (Precinct& p : community.shape.precincts) {
        if ((center[0] - p.get_centroid()[0]) * (center[0] - p.get_centroid()[0]) + (center[1] - p.get_centroid()[1]) * (center[1] - p.get_centroid()[1]) > farthest) {
            farthest = (center[0] - p.get_centroid()[0]) * (center[0] - p.get_centroid()[0]) + (center[1] - p.get_centroid()[1]) * (center[1] - p.get_centroid()[1]);
        }
    }

    return (community.shape.get_area() / (farthest * PI));
}


double get_partisanship_stdev(Community& community) {
    double average = 0;
    map<POLITICAL_PARTY, vector<int> > total_data;

    for (auto& pair : community.vertices) {
        // for each vertex
        for (auto& p2 : pair.second.precinct->voter_data) {
            // for each party
            if (p2.first != POLITICAL_PARTY::TOTAL && p2.second >= 0) {
                total_data[p2.first].push_back(p2.second);
            }
        }
    }

    for (auto& pair : total_data) {
        average += get_stdev(pair.second);
    }
    
    return (average / total_data.size());
}


bool exchange_precinct(Graph& g, Communities& cs, int node_to_take, int community_to_take) {
    // moves a node from its community into `community_to_take`
    int nttc = g.vertices[node_to_take].community;
    if (cs[nttc].vertices.size() == 1) {
        return false;
    }

    if (!remove_edges_to(cs[nttc], node_to_take).is_connected()) return false;

    cs[nttc].remove_node(node_to_take);
    cs[community_to_take].add_node(g.vertices[node_to_take]);
    g.vertices[node_to_take].community = community_to_take;
    return true;
}


vector<int> get_takeable_precincts(Graph& g, Communities& c, int in) {
    vector<int> takeable = {};

    for (auto& pair : c[in].vertices) {
        for (Edge& e : g.vertices[pair.first].edges) {
            if (g.vertices[e[1]].community != in) {
                if (std::find(takeable.begin(), takeable.end(), e[1]) == takeable.end()) {
                    takeable.push_back(e[1]);
                }
            }
        }
    }

    return takeable;
}


vector<vector<int> > get_giveable_precincts(Graph& g, Communities& c, int in) {
    /*
        @desc: Find precincts bordering another community
        @params:
            `Graph&` g: graph for reference
            `Communities&` c: communities to check
            `int` in: index of community to check

        @return: `vector<array<int, 2>` giveable precincts, and the community to give to
    */

    vector<vector<int> > giveable = {};

    for (auto& pair : c[in].vertices) {
        for (Edge& edge : g.vertices[pair.first].edges) {
            // if the node borders a precinct not in the community
            if (g.vertices[edge[1]].community != in) {
                giveable.push_back({
                    pair.first, g.vertices[edge[1]].community
                });
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
    for (int i = 0; i < communities.size(); i++) sum += measure(communities[i]);
    return (sum / communities.size());
}


array<double, 2> worst(Communities& communities, double (*measure)(Community&)) {
    double worst = measure(communities[0]);
    int w_ind = 0;

    for (int i = 1; i < communities.size(); i++) {
        double meas = measure(communities[i]);
        if (meas < worst) {
            w_ind = i;
            worst = meas;
        }
    }

    return {worst, (double)w_ind};
}


void optimize_compactness(Communities& communities, Graph& graph) {
    int iterations_since_best = 0;
    auto community_to_modify = communities.begin();
    int community_to_modify_ind = 0;

    const int SUB_MODIFICATIONS = 20;

    // set the best solution
    Communities best = communities;
    Graph best_g = graph;
    double best_val = average(best, get_compactness);
    // cout << "\e[92m" << best_val << endl;

    // pre-determine locations and areas
    vector<coordinate> centers;
    vector<double> radius;
    for (Community c : communities) {
        centers.push_back(c.shape.get_centroid());
        radius.push_back(sqrt(c.shape.get_area() / PI));
    }

    if (SPEED_OPT) cout << "starting gives" << endl;
    while (iterations_since_best < ITERATION_LIMIT) {
        if (SPEED_OPT) cout << "giving" << endl;
        int give = 0;
        for (int i = 0; i < SUB_MODIFICATIONS; i++) {
            vector<vector<int> > giveable = get_giveable_precincts(graph, communities, community_to_modify_ind);
            for (vector<int> g : giveable) {
                if (!point_in_circle(centers[community_to_modify_ind], radius[community_to_modify_ind], graph.vertices[g[0]].precinct->get_centroid())) {
                    exchange_precinct(graph, communities, g[0], g[1]);
                    give++;
                }
            }
        }

        if (SPEED_OPT) cout << "gave " << give << " taking" << endl;
        int take = 0;
        for (int i = 0; i < SUB_MODIFICATIONS; i++) {
            vector<int> takeable = get_takeable_precincts(graph, communities, community_to_modify_ind);
            for (int t : takeable) {
                if (point_in_circle(centers[community_to_modify_ind], radius[community_to_modify_ind], graph.vertices[t].precinct->get_centroid())) {
                    exchange_precinct(graph, communities, t, community_to_modify_ind);
                    take++;
                }
            }
        }

        if (SPEED_OPT) cout << "taked " << take << " gooved" << endl;
        community_to_modify++;
        community_to_modify_ind++;
        if (community_to_modify == communities.end()) {
            community_to_modify = communities.begin();
            community_to_modify_ind = 0;
        }
        
        double cur = average(communities, get_compactness);

        if (cur > best_val) {
            best_val = cur;
            best = communities;
            best_g = graph;
            iterations_since_best = 0;
        }
        else {
            iterations_since_best++;
        }
    }

    communities = best;
    graph = best_g;
}


bool optimize_population(Communities& communities, Graph& g, double range) {
    // find optimal populations
    range /= 2.0;
    int ideal_pop = get_population(communities) / communities.size();
    int smallest_diff_possible = (int)(range * (double)ideal_pop);
    int iteration = 0;
    int worst_index = 0;
    int worst_difference = 0;
    int worst_pop = 0;
    int c_to_ignore = -1;

    for (int i = 1; i < communities.size(); i++) {
        int pop = communities[i].get_population();
        int diff = abs(ideal_pop - pop);
        if (diff > worst_difference) {
            worst_difference = diff;
            worst_pop = pop;
            worst_index = i;
        }
    }


    while (true) {
        // determine populations
        int x = 0;
        bool quit_from_no_ex = false;

        if (communities[worst_index].get_population() < ideal_pop) {
            vector<int> take = get_takeable_precincts(g, communities, worst_index);
            sort_by_degree(g, communities, take);
            int exchanged_list = 0;
            while (communities[worst_index].get_population() < ideal_pop - smallest_diff_possible) {
                if (x == take.size()) {
                    x = 0;
                    if (exchanged_list == 0) {
                        quit_from_no_ex = true;
                        break;
                    }
                    exchanged_list = 0;
                    take = get_takeable_precincts(g, communities, worst_index);
                    sort_by_degree(g, communities, take);
                }

                if ((g.vertices[take[x]].community != c_to_ignore) || communities.size() <= 2) {
                    if (exchange_precinct(g, communities, take[x], worst_index)) {
                        exchanged_list++;
                    }
                }
                x++;
            }
        }
        else {
            vector<vector<int> > give = get_giveable_precincts(g, communities, worst_index);
            sort_by_degree(g, communities, give);
            int exchanged_list = 0;

            while (communities[worst_index].get_population() > ideal_pop + smallest_diff_possible) {
                if (x == give.size()) {
                    x = 0;
                    if (exchanged_list == 0) {
                        quit_from_no_ex = true;
                        break;
                    }
                    exchanged_list = 0;
                    give = get_giveable_precincts(g, communities, worst_index);
                    sort_by_degree(g, communities, give);
                }

                if ((give[x][1] != c_to_ignore) || (communities.size() <= 2)) {
                    if (exchange_precinct(g, communities, give[x][0], give[x][1])) {
                        exchanged_list++;
                    }
                }
                x++;
            }
        }

        c_to_ignore = worst_index;
        int real_worst_index = 0;
        if (quit_from_no_ex) {
            int first_index = worst_index;
            do {
                worst_index = rand_num(0, communities.size() - 1);
            } while (worst_index == first_index);

            worst_difference = abs(ideal_pop - communities[0].get_population());
            for (int i = 1; i < communities.size(); i++) {
                int diff = abs(ideal_pop - communities[i].get_population());
                if (diff > worst_difference) {
                    worst_difference = diff;
                    real_worst_index = i;
                }
            }
        }
        else {
            worst_index = 0;
            worst_difference = abs(ideal_pop - communities[0].get_population());
            for (int i = 1; i < communities.size(); i++) {
                int diff = abs(ideal_pop - communities[i].get_population());
                if (diff > worst_difference) {
                    worst_difference = diff;
                    worst_index = i;
                    real_worst_index = i;
                }
            }
        }

        if (iteration == 10000) {
            return false;
        }

        iteration++;
        if (worst_difference < smallest_diff_possible) {
            break;
        }
    }

    return true;
}


void minimize_stdev(Communities& communities, Graph& graph) {

    double before_average = average(communities, get_partisanship_stdev);
    int iteration = 0;
    int precincts_to_die = 0;

    while (true) {
	int num_exchanges = 0;
        double first = before_average;
        int community_to_modify_ind = rand_num(0, communities.size() - 1);

        // choose worst community to modify
        vector<vector<int> > giveable = get_giveable_precincts(graph, communities, community_to_modify_ind);
        for (vector<int> g : giveable) {
            Communities before_c = communities;
            Graph before_g = graph;

            if (exchange_precinct(graph, communities, g[0], g[1])) {
                double after_average = average(communities, get_partisanship_stdev);
                if (after_average > before_average) {
                    // undo the exchange
                    communities = before_c;
                    graph = before_g;
                } 
                else {
                    // keep the exchange, set a new best
                    before_average = after_average;
                    num_exchanges++;
                }
            }
        }

        vector<int> takeable = get_takeable_precincts(graph, communities, community_to_modify_ind);
        for (int t : takeable) {
            Communities before_c = communities;
            Graph before_g = graph;

            if (exchange_precinct(graph, communities, t, community_to_modify_ind)) {
                double after_average = average(communities, get_partisanship_stdev);
                if (after_average > before_average) {
                    // undo the exchange
                    communities = before_c;
                    graph = before_g;
                }
                else {
                    before_average = after_average;
                    num_exchanges++;
                }
            }
        }

        if (iteration == 0) precincts_to_die = MIN_PERCENT_PRECINCTS_STDEV * (double)num_exchanges;
        else if (num_exchanges <= precincts_to_die) break;
        iteration++;
    }
}


Communities hte::Geometry::karger_stein(Graph& g1, int n_communities) {
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


void drawc(Communities& cs) {
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(cs));
    canvas.draw_to_window();
}


void save_data_to_csv(string output_file, vector<string> data) {
    string current = readf(output_file);
    current += "\n" + join(data, "\t");
    writef(output_file, current);
}


string get_progress_bar(double progress) {
    int length = 30;
    string bar = "[";
    int num = ceil(progress * (double) length);
    for (int i = 0; i < num; i++) bar += "\e[32m=\e[0m";
    for (int i = num; i < length; i++) bar += " ";
    return bar + "]";
}


void save_voter_counts(string output, Communities& cs) {
    string file = "";
    for (Community c : cs) {
        map<POLITICAL_PARTY, int> total_voter_data;
        for (Precinct p : c.shape.precincts) {
            for (auto& pair : p.voter_data) {
                if (pair.second != -1)
                total_voter_data[pair.first] += pair.second;
            }
        }

        file += "[";
        for (auto& pair : total_voter_data) {
            if (pair.first == POLITICAL_PARTY::DEMOCRAT) file += "\"DEM\":";
            else if (pair.first == POLITICAL_PARTY::REPUBLICAN) file += "\"REP\":";
            else if (pair.first == POLITICAL_PARTY::GREEN) file += "\"GRE\":";
            else if (pair.first == POLITICAL_PARTY::INDEPENDENT) file += "\"IND\":";
            else if (pair.first == POLITICAL_PARTY::LIBERTARIAN) file += "\"LIB\":";
            else if (pair.first == POLITICAL_PARTY::REFORM) file += "\"REF\":";
            else if (pair.first == POLITICAL_PARTY::OTHER) file += "\"OTH\":";
            else break;
            file += to_string(pair.second) + ",";
        }
        
        file += "\"POP\":";
        file += to_string(c.get_population());
        file += "]\n";
    }

    writef(file, output);
}


void save_iteration_data(string write_path, Communities& cs, int num_exchanges) {
    string file = readf(write_path);
    file += "[";
    // add compactness fo file
    for (Community c : cs) {
        file += to_string(get_compactness(c));
        file += ",";
    }
    file.pop_back();
    file += "]\t[";
    for (Community c : cs) {
        file += to_string(get_partisanship_stdev(c));
        file += ",";
    }
    file.pop_back();
    file += "]\t";
    file += to_string(num_exchanges);
    file += "\n";
    writef(file, write_path);
}


Communities hte::Geometry::get_communities(Graph& graph, Communities cs, double pop_constraint, string output_dir, bool communities_run) {
    /*
        @desc: determines a random list of community objects

        @params:
            `Geometry::Graph` graph: connection data
            `int` n_communities: number of communities to generate

        @return: `Communities` init config
    */


    int TIME_ELAPSED = 0;
    int PRECINCTS_EXCHANGED = -1;
    int iteration = 0;
    for (int i = 0; i < cs.size(); i++) {
        cs[i].update_shape(graph);
    }

    string anim_out = output_dir + "/anim/";
    string voter_out = output_dir + "/stats/";
    string data_out = output_dir + "/stats/";
    if (communities_run) anim_out += "communities/";
    else anim_out += "redistricts/";
    if (communities_run) voter_out += "communities/";
    else voter_out += "redistricts/";    
    if (communities_run) data_out += "communities/";
    else data_out += "redistricts/";
    data_out += "data.tsv";

    bool pop_compliant = false;
    int non_compliant_iterations = 0;

    do {
        // start time for max_time and initialize first graph
        auto start = high_resolution_clock::now();
        Graph before = graph;
        cout << "\e[2K\r" << get_progress_bar(((double)TIME_ELAPSED / (double)MAX_TIME)) << "  " << PRECINCTS_EXCHANGED << " exchanges, on stdev, np: " << non_compliant_iterations;
        cout.flush();

        // perform optimization passes
        minimize_stdev(cs, graph);
        auto stop = high_resolution_clock::now();
        TIME_ELAPSED += duration_cast<seconds>(stop - start).count();
        cout << "\e[2K\r" << get_progress_bar(((double)TIME_ELAPSED / (double)MAX_TIME)) << "  " << PRECINCTS_EXCHANGED << " exchanges, on compactness, np: " << non_compliant_iterations;
        cout.flush();

        start = high_resolution_clock::now();
        optimize_compactness(cs, graph);
        stop = high_resolution_clock::now();
        TIME_ELAPSED += duration_cast<seconds>(stop - start).count();
        cout << "\e[2K\r" << get_progress_bar(((double)TIME_ELAPSED / (double)MAX_TIME)) << "  " << PRECINCTS_EXCHANGED << " exchanges, on population, np: " << non_compliant_iterations;
        cout.flush();
        start = high_resolution_clock::now();
        pop_compliant = optimize_population(cs, graph, pop_constraint);
        if (pop_compliant) non_compliant_iterations = 0;
        else non_compliant_iterations++;

        PRECINCTS_EXCHANGED = get_num_communities_changed(before, graph);
        Canvas canvas(450, 450);
        canvas.add_outlines(to_outline(cs));
        canvas.save_img_to_anim(ImageFmt::BMP, anim_out);
        iteration++;
        save_iteration_data(data_out, cs, PRECINCTS_EXCHANGED);
        stop = high_resolution_clock::now();
        TIME_ELAPSED += duration_cast<seconds>(stop - start).count();
    } while (((TIME_ELAPSED < MAX_TIME) && (PRECINCTS_EXCHANGED > (int)(MIN_PERCENT_PRECINCTS * (double)graph.vertices.size()))) || !pop_compliant);

    // the communities are fully optimized
    cout << "\e[2K\r" << get_progress_bar(1) << endl;
    save_voter_counts(voter_out, cs);
    return cs;
}

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

#include "../include/shape.hpp"       // class definitions
#include "../include/community.hpp"   // class definitions
#include "../include/util.hpp"        // array modification functions
#include "../include/geometry.hpp"    // geometry modification, border functions
#include "../include/canvas.hpp"      // geometry modification, border functions

using namespace std;
using namespace Geometry;
using namespace Graphics;

#define VERBOSE 1
#define DEBUG 0

bool stop_init_config;


Precinct_Group Community::get_shape(Graph& graph) {
    vector<Precinct> precincts;
    for (int id : node_ids) {
        precincts.push_back(*graph.vertices[id].precinct);
    }

    return Precinct_Group(precincts);
}


void Community::add_node(Node& node) {
    node_ids.push_back(node.id);
}


vector<int> Graph::get_neighbors(int node) {
    vector<int> t;

    for (int i = 0; i < vertices[node].edges.size(); i++) {
        t.push_back(vertices[node].edges[i][1]);
    }

    return t;
}


vector<int> _union(vector<int> x, int t) {
    x.push_back(t);
    return x;
}


Graph remove_edges_to(int node, Graph g) {
    Graph g2 = g;

    for (Edge edge : g2.vertices[node].edges) {
        Edge remove = {edge[1], edge[0]};

        g2.vertices[edge[1]].edges.erase(
            std::remove(g2.vertices[edge[1]].edges.begin(),
                g2.vertices[edge[1]].edges.end(),
                remove
            ),
            g2.vertices[edge[1]].edges.end()
        );
    }

    g2.vertices[node].edges.clear();
    return g2;
}

// where:
// selected: an ordered set of nodes that can be divided to n consecutive groups
// stop: becomes true when the solution was found
// G: the initial graph
// G2: what remains of the graph after removing all edges to last selected node
// lastGroupLen: number of nodes selected for last group
// groupSize: maximum allowable size of each group
// discomp(): returns number of discontinuous components of the graph
// removeEdgesTo(): removes all edges connected to a node

void back_track(Graph g, Graph g2, vector<int>& selected, int last_group_len, int group_size) {
    
    if (selected.size() == g.vertices.size()) {
        throw Exceptions::CommunityComplete();
    }

    if (last_group_len == group_size) {
        cout << endl << "starting new community" << endl;
        last_group_len = 0;
    }


    if (g2.get_num_components() > selected.size() + 1) {
        return;
    }
    
    vector<int> available = {};

    if (last_group_len == 0) {
        for (int i = 0; i < g.vertices.size(); i++) {
            if (!(find(selected.begin(), selected.end(), i) != selected.end())) {
                available.push_back(i);
            }
        }
        // available = [node for node in G.nodes() if node not in selected]
    }
    else {
        // Find all nodes bordering current group.
        for (int i = selected.size() - last_group_len; i < selected.size(); i++) { // @warn -1
            // cout << "getting neighbors of " << i << endl;
            for (int neighbor : g.get_neighbors(selected[i])) {
                if (find(available.begin(), available.end(), neighbor) == available.end() && 
                    find(selected.begin(), selected.end(), neighbor) == selected.end()) {
                    available.push_back(neighbor);

                    // if (neighbor == 24) {
                    //     cout << "hmm " << i << endl;
                    // }
                }
            }
        }
    }

    Community ca, cs, cd;
    ca.node_ids = available;
    cs.node_ids = selected;

    Canvas canvas(700, 700);
    canvas.add_shape(ca.get_shape(g), false, Color(255, 0, 0), 2);
    canvas.add_shape(cs.get_shape(g), false, Color(0,0,255), 3);
    canvas.draw();

    
    if (available.size() == 0) return;
    sort(available.begin(), available.end());

    for (int node : available) {
        selected.push_back(node);
        back_track(g, remove_edges_to(node, g2), selected,
                    last_group_len + 1, group_size);

        selected.erase(remove(selected.begin(), selected.end(), node), selected.end());
        cout << "backtracking..." << endl;
    }
}



Communities get_initial_configuration(Graph graph, int n_communities) {
    /*
        @desc: determines a random list of community objects

        @params:
            `Geometry::Graph` graph: connection data
            `int` n_communities: number of communities to generate

        @return: `Communities` init config
    */


    // Graph init = graph;
    // Communities communities(n_communities);

    // for (int i = 0; i < graph.vertices.size(); i++) {
    //     (graph.vertices.begin() + i).value().in_group = false;
    // }


    // int base = floor((double)graph.vertices.size() / (double)n_communities); // the base num
    // int rem = graph.vertices.size() % n_communities; // how many need to be increased by 1
    // vector<int> sizes(n_communities, base);
    // for (int i = sizes.size() - 1; i > sizes.size() - rem - 1; i--) sizes[i]++;
    
    // int index = 0;
    // int comm_ind = 0;

    // Graph tmp_graph = graph;
    // vector<int> list = back_track(graph, tmp_graph, {}, 0, base);

    // cout << sizes[0] << endl;
    // cout << list.size() << endl;
    

    // cout << endl;

    // cout << communities[0].node_ids.size() << endl;
    // cout << communities[1].node_ids.size() << endl;

    // for (int i = 0; i < n_communities; i++)
    //     writef(communities[i].get_shape(init).to_json(), "t" + to_string(i) + ".json");


    // // Canvas canvas(200,200);
    // // canvas.add_shape(communities[0].get_shape(init), true, Color(255,0,0), 1);
    // // canvas.add_shape(communities[1].get_shape(init), true, Color(0,255,0), 1);

    // // cout << "drawing" << endl;
    // // canvas.draw();



    Communities communities(n_communities);

    int group_size;
    int n_precincts = graph.vertices.size();
    
    if (n_precincts % n_communities == 0) group_size = n_precincts / n_communities;
    else group_size = floor((double)n_precincts / (double)n_communities) + 1;


    int base = floor((double)graph.vertices.size() / (double)n_communities); // the base num
    int rem = graph.vertices.size() % n_communities; // how many need to be increased by 1
    vector<int> sizes(n_communities, group_size);

    if (n_communities * group_size > n_precincts) {
        int overflow = (n_communities * group_size) - n_precincts;
        sizes[sizes.size() - 1] -= overflow;
    }


    Graph light_graph = graph;
    vector<int> selected = {};


    try {
        back_track(light_graph, light_graph, selected, 0, group_size);
    }
    catch (Exceptions::CommunityComplete) {}

    
    int index = 0;
    int comm_ind = 0;

    for (int size : sizes) {
        for (int x = index; x < index + size; x++) {
            cout << selected[x] << ", ";
            communities[comm_ind].node_ids.push_back(selected[x]);
        }

        cout << endl;
        index += size;
        comm_ind++;
    }


    for (int i = 0; i < communities.size(); i++) {
        writef(communities[i].get_shape(graph).to_json(), "x" + to_string(i) + ".json");
    }

    return communities;
}
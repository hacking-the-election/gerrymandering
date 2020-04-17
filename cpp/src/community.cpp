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

    if (vertices[node].edges.size() == 0) cout << "BALLS" << endl;

    for (int i = 0; i < vertices[node].edges.size(); i++) {
        t.push_back(vertices[node].edges[i][1]);
    }

    return t;
}


vector<int> _union(vector<int> x, int t) {
    x.push_back(t);
    return x;
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

void back_track(Graph& g, Graph& g2, vector<int>& selected, int last_group_len, int group_size) {

    cout << "on a community of size " << last_group_len << endl;

    if (selected.size() == g.vertices.size()) {
        stop_init_config = true;
        cout << "finished all communities" << endl;
        return;
    }

    stop_init_config = false;
    if (last_group_len == group_size) {
        // start a new group
        cout << endl << "starting a new community" << endl;
        last_group_len = 0;
    }

    // check continuity of remaining part of graph
    if (g2.get_num_components() > selected.size() + 1) {
        cout << "more components than pres" << endl;
        return;
    }

    vector<int> available;

    if (last_group_len == 0) {
        available.resize(g.vertices.size(), 0);
        std::iota(available.begin(), available.end(), 0);
        vector<int> tmp = selected;
        sort(tmp.begin(), tmp.end());
        
        vector<int> diff;
        std::set_difference(available.begin(), available.end(), tmp.begin(), tmp.end(),
            std::inserter(diff, diff.begin()));

        available = diff;
    }
    else {
        // find all nodes connected to current group
        for (int i = selected.size() - last_group_len; i < selected.size(); i++) {
            for (int t : g.get_neighbors(selected[i])) {
                if (!(std::find(available.begin(), available.end(), t) != available.end()))
                   available.push_back(t);
            }
            // available = union(available, neighbors(G, node));
        }

        vector<int> tmp = selected;
        sort(tmp.begin(), tmp.end());
        sort(available.begin(), available.end()); //@warn ahhhhh

        vector<int> diff;
        std::set_difference(available.begin(), available.end(), tmp.begin(), tmp.end(),
            std::inserter(diff, diff.begin()));

        available = diff;
        // available = available-selected;
    }


    cout << available.size() << " available" << endl;
    for (int a : available) cout << a << ", ";
    cout << endl;

    if (available.size() == 0) return;
        
    std::shuffle(available.begin(), available.end(), std::random_device());

    // vector<int> last_selected = selected;
    for (int n : available) {
        
        g2.remove_edges_to(n);
        selected.push_back(n);
        cout << "adding " << n << endl;
        back_track(g, g2, selected, last_group_len + 1, group_size);
        cout << "removing " << n << endl;

        if (last_group_len != 0) {
            selected.pop_back();
            last_group_len--;
        }

        if (stop_init_config) {
            break;
        }
    }
}


// vector<Node> get_eligible_precincts(Graph graph, Community& group) {
//     vector<Node> nodes = {};
//     Graph init = graph;

//     for (int i = 0; i < graph.vertices.size(); i++) {
//         Node node = (graph.vertices.begin() + i).value();
//         if (!node.in_group) {
//             graph.remove_edges_to(node.id);

//             if (graph.get_num_components() > group.node_ids.size() + 2) {
//                 nodes.push_back(init.vertices[node.id]);
//             }

//             graph = init;
//         }
//     }

//     return nodes;
// }


// void create_community(Graph& graph, int group_size, Community& group) {
//     vector<Node> eligible_precincts = get_eligible_precincts(graph, group); // all the precincts in the graph that are not already in group and if added will not create an island
//     // How to calculate the latter condition for an eligible precinct:
//     // Remove all the edges connected to that precinct. Check if the number of components of the
//     // graph is more than the number of precincts in the group + 2.

//     vector<Node> tried_precincts = {};

//     while (true) {
//         // This is the islands problem. There are no precincts you can add at this level of 
//         // recursion that don’t eventually lead to the islands problem in higher levels of
//         // recursion, so you have to change a decision you made in lower levels of recursion.
        
//         if (eligible_precincts == tried_precincts) return;

//         vector<Node> not_tried_precincts = {};

//         set_difference(eligible_precincts.begin(), 
//             eligible_precincts.end(),
//             tried_precincts.begin(),
//             tried_precincts.end(),
//             inserter(not_tried_precincts, not_tried_precincts.begin())
//         );

//         Node selected_precinct = *min_element(not_tried_precincts.begin(), not_tried_precincts.end());
//         // this min() is of the node number (rank by degree.) I'm not sure if we’re doing greatest
//         // degree is 1 or the last one, so this may become max() in the future.
        
//         group.add_node(selected_precinct);
//         graph.vertices[selected_precinct.id].in_group = true;

//         vector<array<int, 2> > removed_edges = graph.remove_edges_to(selected_precinct.id);

//         if (group.node_ids.size() == group_size) {
//             // break out of all levels of recursion
//             throw Exceptions::CommunityComplete();
//         }
        
//         // Adds one precinct every call, but recursion,
//         // so it's actually adding all the precincts every call.
//         create_community(graph, group_size, group); 

//         // The above call didn’t raise a CommunityCompleteException, so it has returned from 
//         // all higher levels of recursion back to this call (see red), meaning we have to change 
//         // our decision at this level.

//         // undo edge removal
//         for (Edge edge : removed_edges)
//             graph.add_edge(edge);

//         graph.vertices[selected_precinct.id].in_group = false;
//         group.node_ids.erase(std::remove(group.node_ids.begin(), group.node_ids.end(), selected_precinct.id), group.node_ids.end());
//         tried_precincts.push_back(selected_precinct);
//     }

//     return;
// }


Communities get_initial_configuration(Graph graph, int n_communities) {
    /*
        @desc: determines a random list of community objects

        @params:
            `Geometry::Graph` graph: connection data
            `int` n_communities: number of communities to generate

        @return: `Communities` init config
    */


    Graph init = graph;
    Communities communities(n_communities);

    for (int i = 0; i < graph.vertices.size(); i++) {
        (graph.vertices.begin() + i).value().in_group = false;
    }


    int base = floor((double)graph.vertices.size() / (double)n_communities); // the base num
    int rem = graph.vertices.size() % n_communities; // how many need to be increased by 1
    vector<int> sizes(n_communities, base);
    for (int i = sizes.size() - 1; i > sizes.size() - rem - 1; i--) sizes[i]++;
    
    int index = 0;
    int comm_ind = 0;


    vector<int> list;
    Graph tmp_graph = graph;

    back_track(graph, tmp_graph, list, 0, base);

    cout << sizes[0] << endl;
    cout << list.size() << endl;
    for (int size : sizes) {
        for (int x = index; x < index + size; x++) {
            cout << list[x] << ", ";
            communities[comm_ind].node_ids.push_back(list[x]);
        }

        cout << endl;
        index += size;
        comm_ind++;
    }

    cout << endl;

    cout << communities[0].node_ids.size() << endl;
    cout << communities[1].node_ids.size() << endl;

    for (int i = 0; i < n_communities; i++)
        writef(communities[i].get_shape(init).to_json(), "t" + to_string(i) + ".json");


    // // Canvas canvas(200,200);
    // // canvas.add_shape(communities[0].get_shape(init), true, Color(255,0,0), 1);
    // // canvas.add_shape(communities[1].get_shape(init), true, Color(0,255,0), 1);

    // // cout << "drawing" << endl;
    // // canvas.draw();

    return communities;
}
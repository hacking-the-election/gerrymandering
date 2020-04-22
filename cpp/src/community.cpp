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


// vector<int> refs;
// vector<array<int, 3> > c_colors;
int fill_size;


class NodePtr {
    public:

        NodePtr(int id, Graph* graph) : id(id), graph(graph) {}
        int id;
        Graph* graph;

        friend bool operator< (const NodePtr& l1, const NodePtr& l2);
};


bool operator< (const NodePtr& l1, const NodePtr& l2) {
    return (l1.graph->vertices[l1.id] < l2.graph->vertices[l2.id]);
}


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

    for (int i = 0; i < g2.edges.size(); i++) {
        if (g2.edges[i][0] == node || g2.edges[i][1] == node) {
            g2.edges.erase(g2.edges.begin() + i);
            i--;
        }
    }

    g2.vertices[node].edges.clear();
    return g2;
}


Graph remove_node(int node, Graph graph) {
    graph.remove_edges_to(node);
    graph.vertices.erase(node);
    return graph;
}


void sort_by_degree(vector<int>& ints, Graph* graph) {
    vector<NodePtr> nodes;

    for (int i = 0; i < ints.size(); i++) {
        nodes.push_back(NodePtr(ints[i], graph));
    }

    sort(nodes.begin(), nodes.end());
    vector<int> sorted;
    for (NodePtr node : nodes) {
        sorted.push_back(node.id);
    }

    ints = sorted;
}


void back_track(Graph g, Graph g2, vector<int>& selected, int last_group_len, int group_size) {

    // where:
    // selected: an ordered set of nodes that can be divided to n consecutive groups
    // stop: becomes true when the solution was found
    // G: the initial graph
    // G2: what remains of the graph after removing all edges to last selected node
    // lastGroupLen: number of nodes selected for last group
    // groupSize: maximum allowable size of each group
    // discomp(): returns number of discontinuous components of the graph
    // removeEdgesTo(): removes all edges connected to a node

    // cout << g.vertices.size() << ", " << fill_size << endl;
    int ncomp = g2.get_num_components();
    if (selected.size() == fill_size && ncomp <= selected.size() + 1) {
        throw Exceptions::CommunityComplete();
    }


    if (last_group_len == group_size) {
        // cout << endl << "starting new community" << endl;
        last_group_len = 0;
    }


    if ( ncomp > selected.size() + 1) {
        // cout << "gots them components" << endl;
        return;
    }

    vector<int> available = {};

    if (last_group_len == 0) {
        for (int i = 0; i < g.vertices.size(); i++) {
            if (!(find(selected.begin(), selected.end(), i) != selected.end())) {
                available.push_back(i);
            }
        }
    }
    else {
        // Find all nodes bordering current group.
        for (int i = selected.size() - last_group_len; i < selected.size(); i++) { // @warn -1
            for (int neighbor : g.get_neighbors(selected[i])) {
                if (find(available.begin(), available.end(), neighbor) == available.end() && 
                    find(selected.begin(), selected.end(), neighbor) == selected.end()) {
                    available.push_back(neighbor);
                }
            }
        }
    }


    if (available.size() == 0) {
        // cout << "no precincts available" << endl;
        return;
    }


    // cout << "a" << endl;

    // Communities communities(refs.size());
    // int index = 0;
    // int comm_ind = 0;

    // for (int size : refs) {
    //     bool _break = false;
    //     for (int x = index; x < index + size; x++) {
    //         if (x == selected.size()) {
    //             _break = true;
    //             break;
    //         }

    //         communities[comm_ind].node_ids.push_back(selected[x]);
    //     }

    //     if (_break) {
    //         break;
    //     }

    //     index += size;
    //     comm_ind++;
    // }


    // // cout << "a" << endl;
    // Community state;
    // state.node_ids.resize(g.vertices.size());
    // iota(state.node_ids.begin(), state.node_ids.end(), 0);
    // writef(state.get_shape(g).to_json(), "state.json");
    
    sort_by_degree(available, &g2);
    // sort(available.begin(), available.end());
    // shuffle(available.begin(), available.end(), random_device());

    // for (int i = 0; i < communities.size(); i++) {
    //     writef(communities[i].get_shape(g).to_json(), "x" + to_string(i) + ".json");
    // }

    // cout << "a" << endl;

    for (int i = 0; i < available.size(); i++) {
        int node = available[i];
        selected.push_back(node);
        cout << "add " << node << endl;

        back_track(g, remove_edges_to(node, g2), selected, last_group_len + 1, group_size);

        cout << "backtracking..." << endl;
        selected.erase(remove(selected.begin(), selected.end(), node), selected.end());
    }

    return;
    // cout << "finished with everything hmm" << endl;
}



Communities get_initial_configuration(Graph graph, int n_communities) {
    /*
        @desc: determines a random list of community objects

        @params:
            `Geometry::Graph` graph: connection data
            `int` n_communities: number of communities to generate

        @return: `Communities` init config
    */

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

    fill_size = 0;
    for (int i = 0; i < sizes.size() - 1; i++) {
        fill_size += sizes[i];
    }

    Graph light_graph = graph;
    vector<int> selected = {};


    try {
        back_track(light_graph, light_graph, selected, 0, group_size);
    }
    catch (Exceptions::CommunityComplete) {
        cout << "finding communities" << endl;
    }


    for (int i = 0; i < n_precincts; i++) {
        if (std::find(selected.begin(), selected.end(), i) == selected.end()) {
            selected.push_back(i);
        }
    }


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
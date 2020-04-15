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


Precinct_Group Community::get_shape() {
    return Precinct_Group();
}


void Community::add_node(Node& node) {
    node_ids.push_back(node.id);
}


void create_community(Geometry::Graph& graph, int group_size, Geometry::Community& group) {
    vector<Node> eligible_precincts = {}; // all the precincts in the graph that are not already in group and if added will not create an island
    // How to calculate the latter condition for an eligible precinct:
    // Remove all the edges connected to that precinct. Check if the number of components of the
    // graph is more than the number of precincts in the group + 2. 2

    vector<Node> tried_precincts = {};

    while (true) {
        // This is the islands problem. There are no precincts you can add at this level of 
        // recursion that don’t eventually lead to the islands problem in higher levels of
        // recursion, so you have to change a decision you made in lower levels of recursion.
        
        if (eligible_precincts == tried_precincts) return;

        vector<Node> not_tried_precincts = {};

        set_difference(eligible_precincts.begin(), 
            eligible_precincts.end(),
            tried_precincts.begin(),
            tried_precincts.end(),
            inserter(not_tried_precincts, not_tried_precincts.begin())
        );

        Node selected_precinct = *min_element(not_tried_precincts.begin(), not_tried_precincts.end());
        // this min() is of the node number (rank by degree.) I'm not sure if we’re doing greatest
        // degree is 1 or the last one, so this may become max() in the future.
        
        group.add_node(selected_precinct);
        vector<array<int, 2> > removed_edges = graph.remove_edges_to(selected_precinct.id);

        if (group.node_ids.size() == group_size) {
            // break out of all levels of recursion
            throw Exceptions::CommunityComplete();
        }
        
        // Adds one precinct every call, but recursion,
        // so it's actually adding all the precincts every call.
        create_community(graph, group_size, group); 

        // The above call didn’t raise a CommunityCompleteException, so it has returned from 
        // all higher levels of recursion back to this call (see red), meaning we have to change 
        // our decision at this level.

        // undo edge removal
        for (Edge edge : removed_edges)
            graph.add_edge(edge);

        group.node_ids.erase(std::remove(group.node_ids.begin(), group.node_ids.end(), selected_precinct.id), group.node_ids.end());
        tried_precincts.push_back(selected_precinct);
    }

    return;
}


Communities get_initial_configuration(Graph graph, int n_communities) {
    Communities communities(n_communities);

    int base = floor((double)graph.vertices.size() / (double)n_communities); // the base num
    int rem = graph.vertices.size() % n_communities; // how many need to be increased by 1

    vector<int> sizes(n_communities, base);
    for (int i = sizes.size() - 1; i > sizes.size() - rem - 1; i--) sizes[i]++;

    for (int i = 0; i < sizes.size() - 1; i++) {
        
        try {
            create_community(graph, sizes[i], communities[i]);
        }
        catch (Exceptions::CommunityComplete) {
            cout << "Finished community of size " << sizes[i] << endl;
        }

        for (int id : communities[i].node_ids) {
            graph.vertices.erase(id);
        }
    }

    for (int i = 0; i < graph.vertices.size(); i++)
        communities[communities.size() - 1].add_node(graph.vertices[i]);

    return communities;
}
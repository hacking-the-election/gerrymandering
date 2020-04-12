/*=======================================
 graph.cpp:                     k-vernooy
 last modified:               Sat, Apr 11
 
 Definitions for graph theory related
 algorithm implementations such as
 searches and component counts.
========================================*/

#include <iostream>
#include "../include/shape.hpp"

using namespace Geometry;
using namespace std;


int Graph::get_num_components() {
    /*
        @desc: get number of components of a graph
        @params: none
        @return: `int` number of components
    */

    vector<bool> visited(vertices.size(), false);
    int x = 0;

    for (int i = 0; i < vertices.size(); i++) {
        if (visited[i] == false) {
            dfs_recursor(i, visited); 
            x++;
        }
    }

    return x;
}


void Graph::dfs_recursor(int v, std::vector<bool>& visited) { 
    /*
        @desc: recur seach for each adjacent node to index v
        
        @params: 
            `int` v: index of node
            `std::vector<bool>&` visited: array of which
                nodes have been visited

        @return: void
    */

    visited[v] = true; 
    
    for (int i = 0; i < this->vertices[v].edges.size(); i++) {
        int t_id = this->vertices[v].edges[i][1];

        if (!visited[t_id]) { 
            dfs_recursor(t_id, visited);
        }
    }
} 

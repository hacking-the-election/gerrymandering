/*=======================================
 graph.cpp:                     k-vernooy
 last modified:               Sat, Apr 11
 
 Definitions for graph theory related
 algorithm implementations such as
 searches and component counts.
========================================*/

#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <chrono>

#include "../include/shape.hpp"
#include "../include/canvas.hpp"
#include "../include/util.hpp"


using namespace hte::Graphics;
using namespace hte::Geometry;
using namespace std;
using namespace chrono;

void Graph::remove_node(int id) {
    remove_edges_to(id);
    vertices.erase(id);
}


void Graph::add_node(Node node) {
    this->vertices.insert({node.id, node});
}


Graph Graph::get_induced_subgraph(vector<int> nodes) {

    Graph subgraph;
    Graph copy = *this;

    for (int x : nodes) {
        for (int i = 0; i < copy.vertices[x].edges.size(); i++) {
            if (find(nodes.begin(), nodes.end(), copy.vertices[x].edges[i][1]) == nodes.end()) {
                copy.vertices[x].edges.erase(copy.vertices[x].edges.begin() + i);
                i--;
            }
        }
        subgraph.vertices[x] = copy.vertices[x];
    }
    
    return subgraph;
}
        

std::vector<Graph> Graph::get_components() {
    /*
        @desc: get the subgraphs that make up the components
        @params: none
        @return: `vector<Geometry::Graph>` components subgraphs
    */

    unordered_map<int, bool> visited;
    vector<Graph> components;

    for (int i = 0; i < vertices.size(); i++) {
        if (!visited[(vertices.begin() + i).key()]) {
            Graph component;
            vector<int> graph = {};
            dfs_recursor((vertices.begin() + i).key(), visited, &graph);

            for (int i = 0; i < graph.size(); i++) {
                component.add_node(this->vertices[graph[i]]);
            }

            components.push_back(component);
        }
    }

    return components;
}


void Graph::dfs_recursor(int v, std::unordered_map<int, bool>& visited, std::vector<int>* nodes) {
    /*
        @desc: recur seach for each adjacent node to index v
        
        @params: 
            `int` v: index of node
            `std::vector<bool>&` visited: array of which
                nodes have been visited

        @return: void
    */
 
    visited[v] = true; 
    nodes->push_back(v);

    for (Edge& e : vertices[v].edges) {
        int t_id = e[1];
        if (!visited[t_id]) { 
            dfs_recursor(t_id, visited, nodes);
        }
    }
}


bool Graph::is_connected() {
    unordered_map<int, bool> v;
    int visited = 0;
    dfs_recursor(vertices.begin().key(), visited, v);
    return (visited == vertices.size());
}


void Graph::dfs_recursor(int v, int& visited, unordered_map<int, bool>& visited_b) {
    visited++;
    visited_b[v] = true;
    for (Edge& e : vertices[v].edges) {
        if (!visited_b[e[1]]) {
            dfs_recursor(e[1], visited, visited_b);
        }
    }
}


int Graph::get_num_components() {
    /*
        @desc: get number of components of a graph
        @params: none
        @return: `int` number of components
    */


    unordered_map<int, bool> visited;
    int x = 0;

    for (int i = 0; i < vertices.size(); i++) {
        if (!visited[(vertices.begin() + i).key()]) {
            dfs_recursor((vertices.begin() + i).key(), visited); 
            x++;
        }
    }

    return x;
}



void Graph::dfs_recursor(int v, std::unordered_map<int, bool>& visited) { 
    /*
        @desc: recur seach for each adjacent node to index v
        
        @params: 
            `int` v: index of node
            `std::vector<bool>&` visited: array of which
                nodes have been visited

        @return: void
    */

    visited[v] = true; 
    for (Edge& edge : vertices[v].edges) {
        if (!visited[edge[1]]) { 
            dfs_recursor(edge[1], visited);
        }
    }
} 


void Graph::add_edge(Edge edge) {
    /*
        @desc: Adds an edge to a graph object
        @params: `Geometry::Edge` edge: edge to be added
        @return: `void`
    */

    Edge hl, lh;

    if (edge[0] < edge[1]) {
        hl[0] = edge[0];
        hl[1] = edge[1];

        lh[0] = edge[1];
        lh[1] = edge[0];
    }
    else {
        hl[0] = edge[1];
        hl[1] = edge[0];

        lh[0] = edge[0];
        lh[1] = edge[1];
    }


    // if (!(std::find(edges.begin(), edges.end(), hl) != edges.end())) {
    //     edges.push_back(hl);
    // }

    if (!(std::find(vertices[hl[0]].edges.begin(),
            vertices[hl[0]].edges.end(), hl)
        != vertices[hl[0]].edges.end())) {

        vertices[hl[0]].edges.push_back(hl);
    }


        if (!(std::find(vertices[hl[1]].edges.begin(),
            vertices[hl[1]].edges.end(), lh) 
        != vertices[hl[1]].edges.end())) {

        vertices[hl[1]].edges.push_back(lh);
    }
}


Graph hte::Geometry::remove_edges_to(Graph g, int id) {
    for (Edge edge : g.vertices[id].edges) {
        Edge remove = {edge[1], edge[0]};

        if (g.vertices.find(edge[1]) != g.vertices.end()) {
            g.vertices[edge[1]].edges.erase(
                std::remove(g.vertices[edge[1]].edges.begin(),
                    g.vertices[edge[1]].edges.end(),
                    remove
                ),
                g.vertices[edge[1]].edges.end()
            );
        }
    }

    // g.vertices[id].edges.clear();
    g.vertices.erase(id);
    return g;
}


void Graph::remove_edges_to(int id) {
    /*
        @desc: removes edges to a node id
        @params: `int` id: id to remove edges of
        @return: `std::vector<Geometry::Edge>` edges that have been removed
    */


    for (Edge edge : vertices[id].edges) {
        Edge remove = {edge[1], edge[0]};
        if (vertices.find(edge[1]) != vertices.end()) {
            vertices[edge[1]].edges.erase(
                std::remove(vertices[edge[1]].edges.begin(),
                    vertices[edge[1]].edges.end(),
                    remove
                ),
                vertices[edge[1]].edges.end()
            );
        }
    }

    vertices[id].edges.clear();
}

// void Graph::remove_edge(Edge edge) {
    
// }

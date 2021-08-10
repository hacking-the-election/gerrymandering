// #include <unordered_map>
// #include <algorithm>
// #include <numeric>

#include "../include/graph.h"

namespace hte {

// void Graph::removeNode(int id) {
//     removeEdgesTo(id);
//     vertices.erase(id);
// }


// void Graph::addNode(Node node) {
//     this->vertices.insert({node.id, node});
// }


// Graph Graph::getInducedSubgraph(vector<int> nodes) {

//     Graph subgraph;
//     Graph copy = *this;

//     for (int x : nodes) {
//         for (int i = 0; i < copy.vertices[x].edges.size(); i++) {
//             if (find(nodes.begin(), nodes.end(), copy.vertices[x].edges[i][1]) == nodes.end()) {
//                 copy.vertices[x].edges.erase(copy.vertices[x].edges.begin() + i);
//                 i--;
//             }
//         }
//         subgraph.vertices[x] = copy.vertices[x];
//     }
    
//     return subgraph;
// }
        

// std::vector<Graph> Graph::getComponents() {
//     /*
//         @desc: get the subgraphs that make up the components
//         @params: none
//         @return: `vector<Graph>` components subgraphs
//     */

//     unordered_map<int, bool> visited;
//     vector<Graph> components;

//     for (int i = 0; i < vertices.size(); i++) {
//         if (!visited[(vertices.begin() + i).key()]) {
//             Graph component;
//             vector<int> graph = {};
//             dfsRecursor((vertices.begin() + i).key(), visited, &graph);

//             for (int i = 0; i < graph.size(); i++) {
//                 component.addNode(this->vertices[graph[i]]);
//             }

//             components.push_back(component);
//         }
//     }

//     return components;
// }


// template<typename K, typename T>
// template<typename Ret>
// void AdjacencyListGraph<K, T>::dfsRecursor(const K v, const std::unordered_map<K, bool>& visited, const std::function<Ret(const K)>& f, bool doBreak)
// {
//     visited[v] = true;
//     f(v);

//     for (const K& neighbor : vertexMap[v])
//     {
//         if (!visited[neighbor])
//         { 
//             dfsRecursor(neighbor, visited, f, doBreak);
//         }
//     }
// }


// bool Graph::isConnected() {
//     unordered_map<int, bool> v;
//     int visited = 0;
//     dfsRecursor(vertices.begin().key(), visited, v);
//     return (visited == vertices.size());
// }


// void Graph::dfsRecursor(int v, int& visited, unordered_map<int, bool>& visitedB) {
//     visited++;
//     visitedB[v] = true;
//     for (Edge& e : vertices[v].edges) {
//         if (!visitedB[e[1]]) {
//             dfsRecursor(e[1], visited, visitedB);
//         }
//     }
// }


// int Graph::getNumComponents() {
//     /*
//         @desc: get number of components of a graph
//         @params: none
//         @return: `int` number of components
//     */

//     unordered_map<int, bool> visited;
//     int x = 0;

//     for (int i = 0; i < vertices.size(); i++) {
//         if (!visited[(vertices.begin() + i).key()]) {
//             dfsRecursor((vertices.begin() + i).key(), visited); 
//             x++;
//         }
//     }

//     return x;
// }


// void Graph::dfsRecursor(int v, std::unordered_map<int, bool>& visited) { 
//     /*
//         @desc: recur seach for each adjacent node to index v
        
//         @params: 
//             `int` v: index of node
//             `std::vector<bool>&` visited: array of which
//                 nodes have been visited

//         @return: void
//     */

//     visited[v] = true; 
//     for (Edge& edge : vertices[v].edges) {
//         if (!visited[edge[1]]) { 
//             dfsRecursor(edge[1], visited);
//         }
//     }
// } 


// void Graph::addEdge(Edge edge) {
//     /*
//         @desc: Adds an edge to a graph object
//         @params: `Edge` edge: edge to be added
//         @return: `void`
//     */

//     Edge hl, lh;

//     if (edge[0] < edge[1]) {
//         hl[0] = edge[0];
//         hl[1] = edge[1];

//         lh[0] = edge[1];
//         lh[1] = edge[0];
//     }
//     else {
//         hl[0] = edge[1];
//         hl[1] = edge[0];

//         lh[0] = edge[0];
//         lh[1] = edge[1];
//     }


//     // if (!(std::find(edges.begin(), edges.end(), hl) != edges.end())) {
//     //     edges.push_back(hl);
//     // }

//     if (!(std::find(vertices[hl[0]].edges.begin(),
//             vertices[hl[0]].edges.end(), hl)
//         != vertices[hl[0]].edges.end())) {

//         vertices[hl[0]].edges.push_back(hl);
//     }


//         if (!(std::find(vertices[hl[1]].edges.begin(),
//             vertices[hl[1]].edges.end(), lh) 
//         != vertices[hl[1]].edges.end())) {

//         vertices[hl[1]].edges.push_back(lh);
//     }
// }


// Graph hte::RemoveEdgesTo(Graph g, int id) {
//     for (Edge edge : g.vertices[id].edges) {
//         Edge remove = {edge[1], edge[0]};

//         if (g.vertices.find(edge[1]) != g.vertices.end()) {
//             g.vertices[edge[1]].edges.erase(
//                 std::remove(g.vertices[edge[1]].edges.begin(),
//                     g.vertices[edge[1]].edges.end(),
//                     remove
//                 ),
//                 g.vertices[edge[1]].edges.end()
//             );
//         }
//     }

//     // g.vertices[id].edges.clear();
//     g.vertices.erase(id);
//     return g;
// }


// void Graph::removeEdgesTo(int id) {
//     /*
//         @desc: removes edges to a node id
//         @params: `int` id: id to remove edges of
//         @return: `std::vector<Edge>` edges that have been removed
//     */


//     for (Edge edge : vertices[id].edges) {
//         Edge remove = {edge[1], edge[0]};
//         if (vertices.find(edge[1]) != vertices.end()) {
//             vertices[edge[1]].edges.erase(
//                 std::remove(vertices[edge[1]].edges.begin(),
//                     vertices[edge[1]].edges.end(),
//                     remove
//                 ),
//                 vertices[edge[1]].edges.end()
//             );
//         }
//     }

//     vertices[id].edges.clear();
// }

// // void Graph::remove_edge(Edge edge) {
    
// }

}
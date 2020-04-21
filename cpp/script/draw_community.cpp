/*=======================================
 draw_community.cpp:            k-vernooy
 last modified:                Sun, Apr 5
 
 Reads a state object and draws its
 community object list to a canvas
========================================*/

#include <iostream>
#include <boost/filesystem.hpp>
#include <iomanip>

#include "../include/shape.hpp"
#include "../include/util.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include <chrono> 

using namespace std::chrono; 

using namespace std;
using namespace Geometry;
using namespace Graphics;

class NodePair {
    public:
        array<int, 2> node_ids;
        double distance;
        NodePair() {};

        bool operator<(const NodePair& l2) const {
            return this->distance < l2.distance;
        }
};



Graph link_islands(Graph graph) {
    
    while (graph.get_num_components() > 1) {

        vector<Graph> components = graph.get_components();
        vector<NodePair> dists;

        for (size_t i = 0; i < components.size(); i++) {
            for (size_t j = i + 1; j < components.size(); j++) {
                for (size_t p = 0; p < components[i].vertices.size(); p++) {
                    for (size_t q = 0; q < components[j].vertices.size(); q++) {
                        double distance = get_distance((components[i].vertices.begin() + p).value().precinct->get_center(), (components[j].vertices.begin() + q).value().precinct->get_center());
                        NodePair np;
                        np.distance = distance;
                        np.node_ids = {(components[i].vertices.begin() + p).key(), (components[j].vertices.begin() + q).key()};
                        dists.push_back(np);
                    }
                }
            }
        }

        array<int, 2> me = min_element(dists.begin(), dists.end())->node_ids;
        graph.add_edge({me[0], me[1]});
        
        // cout << *min_element(distances.begin(), distances.end()) << endl;
        // get closest two islands
        // find closest exterior nodes
    }

    return graph;
}


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    // draw communities
    Canvas canvas(900, 900);

    // read binary file from path
    State state = State::read_binary(argv[1]);
    // canvas.add_graph(state.network);

    cout << "drawing" << endl;

    // auto start = high_resolution_clock::now(); 
    // cout << state.network.get_num_components() << endl;
    // auto stop = high_resolution_clock::now(); 
    // auto duration = duration_cast<microseconds>(stop - start); 
    
    // cout << duration.count() << endl; 
    // link_islands(state.network);
    canvas.add_graph(link_islands(state.network));
    canvas.draw();
    return 0;
}
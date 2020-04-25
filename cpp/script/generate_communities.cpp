/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                Sat, Feb 8
 
 Run community generation algorithm and 
 print coordinates as geojson for a given
 state object
========================================*/

#include <boost/filesystem.hpp>
#include <iostream>


#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/community.hpp"

using namespace boost::filesystem;
using namespace std;
using namespace Geometry;
using namespace Graphics;


class NodePair2 {
    /*
        A class representing a link between two nodes.
        This is used for sorting in the link_islands method.
        @warn: this is not to be used as an edge.
    */

    public:
        array<int, 2> node_ids;
        double distance;
        NodePair2() {};

        bool operator<(const NodePair2& l2) const {
            return this->distance < l2.distance;
        }
};


void link_islands2(Geometry::Graph& graph) {
    /*
        @desc: Links discontinuous components of a graph
        @params: `Geometry::Graph&` graph: graph to connect
        @return: `void`
    */


    cout << "a" << endl;
    if (graph.get_num_components() > 1) {
        // the graph needs to be linked

        cout << "a" << endl;
        map<int, Geometry::coordinate> centers;        
        // determine all centers of precincts
        for (int i = 0; i < graph.vertices.size(); i++) {
            // add center of precinct to the map
            centers.insert({
                (graph.vertices.begin() + i).key(),
                (graph.vertices.begin() + i).value().precinct->get_center()
            });
        }

        cout << "a" << endl;

        while (graph.get_num_components() > 1) {
            // add edges between two precincts on two islands
            // until `graph` is connected
            
            vector<Geometry::Graph> components = graph.get_components();
            vector<NodePair2> dists;

            for (size_t i = 0; i < components.size(); i++) {
                for (size_t j = i + 1; j < components.size(); j++) {
                    // for each distinct combination of islands
                    for (size_t p = 0; p < components[i].vertices.size(); p++) {
                        for (size_t q = 0; q < components[j].vertices.size(); q++) {
                            // for each distinct precinct combination of the
                            // two current islands, determine distance between centers
                            int keyi = (components[i].vertices.begin() + p).key(), keyj = (components[j].vertices.begin() + q).key();
                            double distance = get_distance(centers[keyi], centers[keyj]);
                            
                            // add node pairing to list
                            NodePair2 np;
                            np.distance = distance;
                            np.node_ids = {keyi, keyj};
                            dists.push_back(np);
                        }
                    }
                }
            }

            // find the shortest link between any two precincts on any two islands islands
            array<int, 2> me = std::min_element(dists.begin(), dists.end())->node_ids;
            // add link as edge to the graph
            graph.add_edge({me[0], me[1]});
        }
    } // else the graph is linked already
}


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    // if (argc != 3) {
    //     cerr << "generate_communities: usage: <state.dat> <data_dir>" << endl;
    //     return 1;
    // }

    // read binary file from path
    string read_path = string(argv[1]);
    State state = State::read_binary(read_path);

    int n_communities = stoi(string(argv[2]));
    link_islands2(state.network);
    get_initial_configuration(state.network, n_communities);
    
    return 0;
}

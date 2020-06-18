/*=======================================
 algorithm.h:                   k-vernooy
 last modified:               Thu, Jun 18

 A header for defining graph theory classes
 and other structures needed for all
 community or quantification algorithm
 related code.
========================================*/

#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "data.h"
#include "geometry.h"
#include <unordered_map>
#include "../lib/ordered-map/include/tsl/ordered_map.h"


namespace hte {
    namespace Algorithm {

        class Node;
        class Graph;
        class Community;
        typedef std::array<int, 2> Edge;
        typedef std::vector<Community> Communities;

        /**
         * A vertex on the `Graph` class, containing
         * precinct information and edge information
        */
        class Node {
            public:

                int id;                           //!< A unique integer identifier for the node
                int community;                    //!< Which community the node belongs to
                Data::Precinct* precinct;     //!< A precinct pointer for geometry functions

                Node(){}
                Node(hte::Data::Precinct* precinct) : precinct(precinct) {}

                std::vector<Edge> edges;       //!< A list of unique edges for the node, in the form `{this->id, connected_node.id}`
                std::vector<int> collapsed;    //!< A list of collapsed nodes for the karger-stein algorithm

                friend bool operator< (const Node& l1, const Node& l2);
                friend bool operator== (const Node& l1, const Node& l2);
        };

        
        /**
         * \brief Remove all edges to specific node
         * 
         * For every node connected to `id`, remove the edge
         * connecting to `id`.
         * 
         * @param g Graph to remove edges from
         * @param id Node to removes edges to
         * @return Updated graph with removed edges
         */
        Graph remove_edges_to(Graph g, int id);


        /**
         * \brief A graph as defined by discrete mathematics
         * 
         * Contains a list of Node objects, with Edges connecting
         * a certain subset of Nodes. This represents an undirected
         * and unweighted graph. It is used for Community
         * generation algorithms
         */
        class Graph {
            public:
                tsl::ordered_map<int, Node> vertices;  //!< All nodes on the graph
                std::vector<Edge> edges;               //!< List of unique edges on the graph

                /**
                 * \brief Get the induced subgraph from a list of node ids
                 * @param nodes the integer list of nodes to get the subgraph of
                 * @return The subgraph object
                 */
                Graph get_induced_subgraph(std::vector<int> nodes);

                /**
                 * Determine the number of [components](https://en.wikipedia.org/wiki/Component_(graph_theory)) in the graph
                 * @return number of components
                 */
                int getNumComponents();

                /**
                 * get a list of subgraph components
                 * @return subgraph components
                 */
                std::vector<Graph> getComponents();
                bool isConnected();

                void add_node(Node node);
                void remove_node(int id);
                void addEdge(Edge);
                void remove_edge(Edge);
                void remove_edges_to(int id);

                protected:
                // recursors for getting different data
                void dfs_recursor(int v, std::unordered_map<int, bool>& visited);
                void dfs_recursor(int v, std::unordered_map<int, bool>& visited, std::vector<int>* nodes);
                void dfs_recursor(int v, int& visited, std::unordered_map<int, bool>& visited_b);
        };

        class Community : public Graph {
            /*
                Contains a list of precincts, as well as information about
                linking and where is it on an island list.
            */

            public:
                double quantification;
                double partisanQuantification;
                
                // shape object for geometry methods,
                // must be kept up to date in every operation
                Data::PrecinctGroup shape;

                int  getPopulation();
                void addNode(Node&);
                void removeNode(int);
                void resetShape(Graph&);

                Community(std::vector<int>& nodeIds, Graph& graph);
                Community() {}
        };


        double average(Communities& cs, double (*measure)(Community&));
        double get_partisanship_stdev(Community& c);
        double get_compactness(Community& c);
        double get_precise_compactness(Community& c);
        double get_distance_from_pop(Communities& cs);
        double get_scalarized_metric(Communities& cs);
        int get_num_precincts_changed(Graph& g1, Graph& g2);

        void optimize_compactness(Communities& cs, Graph& g);
        void minimize_stdev(Communities& cs, Graph& g);
        bool optimize_population(Communities& cs, Graph& g, double range);


        void save(Communities, std::string);
        Communities load(std::string, Graph&);
        Communities load(std::string, Graph&, std::string);
        std::vector<std::vector<double> > load_quantification(std::string tsv);


        /**
         * Partitions a graph according to the Karger-Stein algorithm
         * 
         * \param graph The graph to partition
         * \param n_communities The number of partitions to make
         * @return: `Communities` list of id's corresponding to the partition
         */
        Communities karger_stein(Graph& graph, int nCommunities);

        /**
         * Given a random init config, optimize the provided communities
         * under the communities algorithm.
         * 
         * \param graph The connection network of the state
         * \param init_config The communities to modify
         * \param output_dir Where to save data files to
         * \param communities_run Whether or not communities are being run (as apposed to redistricting)
         * 
         * \return Optimized communities
         */
        Communities get_communities(Graph& g, Communities initConfig, double popConstraint, std::string outputDir, bool communitiesRun);
        void gradient_descent_optimization(Graph& g, Communities& cs, double (*measure)(Communities&));
        void simulated_annealing_optimization(Graph& g, Communities& cs, double (*measure)(Community&));
    }
}

#endif

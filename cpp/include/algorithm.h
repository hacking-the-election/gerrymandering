/*=======================================
 algorithm.h:                   k-vernooy
 last modified:               Fri, Jun 19

 A header for defining graph theory classes
 and other structures needed for all
 community or quantification algorithm
 related code.
========================================*/

#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <unordered_map>

#include "data.h"
#include "geometry.h"
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
        Graph RemoveEdgesTo(Graph g, int id);


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
                Graph getInducedSubgraph(std::vector<int> nodes);

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
                void addNode(Node node);
                void removeNode(int id);
                void addEdge(Edge);
                void removeEdge(Edge);
                void removeEdgesTo(int id);

                protected:
                // recursors for getting different data
                void dfsRecursor(int v, std::unordered_map<int, bool>& visited);
                void dfsRecursor(int v, std::unordered_map<int, bool>& visited, std::vector<int>* nodes);
                void dfsRecursor(int v, int& visited, std::unordered_map<int, bool>& visitedB);
        };


        /**
         * Contains a list of precincts, as well as information about
         * linking and where is it on an island list.
        */
        class Community : public Graph {

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


        double Average(Communities& cs, double (*measure)(Community&));
        double GetPartisanshipStdev(Community& c);
        double GetCompactness(Community& c);
        double GetPreciseCompactness(Community& c);
        double GetDistanceFromPop(Communities& cs, double);
        double GetScalarizedMetric(Communities& cs);
        int GetNumPrecinctsChanged(Graph& g1, Graph& g2);

        void SaveCommunitiesToFile(Communities, std::string);
        Communities LoadCommunitiesFromFile(std::string, Graph&);
        Communities LoadCommunitiesWithQuantification(std::string, Graph&, std::string);
        std::vector<std::vector<double> > LoadQuantification(std::string tsv);


        /**
         * Partitions a graph according to the Karger-Stein algorithm
         * \param graph The graph to partition
         * \param n_communities The number of partitions to make
         * @return: `Communities` list of id's corresponding to the pa
         * rtition
         */
        Communities KargerStein(Graph& graph, int nCommunities);
        void GradientDescentOptimization(Graph& g, Communities& cs, double (*measure)(Communities&));
        void SimulatedAnnealingOptimization(Graph& g, Communities& cs, double (*measure)(Community&));

        double CollapseVals(double a, double b)
        double GetPopulationFromMask(Data::PrecinctGroup pg, Geometry::MultiPolygon mp);
        std::map<Data::PoliticalParty, double> GetPartisanshipFromMask(Data::PrecinctGroup pg, Geometry::MultiPolygon mp);
        std::map<Data::PoliticalParty, double> GetQuantification(Graph& graph, Communities& communities, Geometry::MultiPolygon district);
    }
}

#endif

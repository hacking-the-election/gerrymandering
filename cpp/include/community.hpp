/*=======================================
 community.hpp:                     k-vernooy
 last modified:              Tues, May 1
 
 Definition of the community class, and
 declaration of community algorithm 
 methods.
========================================*/

#pragma once
#include "shape.hpp"

namespace hte {
namespace Geometry {

    // for cleaner naming of types when writing community algorithm
    // typedef Precinct_Group Community;
    class Community;
    typedef std::vector<Community> Communities;

    void save(Communities, std::string);
    Communities load(std::string, Graph&);

    class Community : public Graph {
        /*
            Contains a list of precincts, as well as information about
            linking and where is it on an island list.
        */

        public:
            int get_population();

            void add_node(Node&);
            void remove_node(int);

            // shape object for geometry methods,
            // must be kept up to date in every operation
            Precinct_Group shape;

            // hard reset the `shape` object
            void update_shape(Graph&);

            Community(std::vector<int>& node_ids, Graph& graph);
            Community() {}
    };

    Communities karger_stein(Graph& graph, int n_communities);
    Communities get_communities(Graph& graph, Communities init_config, double pop_constraint, std::string output_dir, bool communities_run);
}
}

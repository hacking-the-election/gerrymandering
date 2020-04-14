/*=======================================
 graph.hpp:                     k-vernooy
 last modified:              Tues, Apr 14
 
 Definition of the community class, and
 declaration of community algorithm 
 methods.
========================================*/

#pragma once
#include "shape.hpp"


namespace Geometry {

    // for cleaner naming of types when writing community algorithm
    // typedef Precinct_Group Community;
    class Community;
    typedef std::vector<Community> Communities;

    class Community {
        /*
            Contains a list of precincts, as well as information about
            linking and where is it on an island list.
        */

        public:
            void save(std::string write_path);
            void load(std::string read_path);

            std::vector<int> node_ids;

            void add_node(Node node);
            Precinct_Group get_shape();
    };

}

Geometry::Communities get_initial_configuration(Geometry::Graph graph);
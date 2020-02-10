/*=======================================
 community.cpp:                 k-vernooy
 last modified:                Sat, Feb 9

 Definition of the community-generation
 algorithm for quantifying gerrymandering
 and redistricting.
========================================*/

#include "../../../include/shape.hpp"        // class definitions
#include "../../../include/util.hpp"         // array modification functions
#include "../../../include/geometry.hpp"
#include <math.h>                            // for rounding functions
#include <numeric>
#include <algorithm>

vector<Precinct_Group> State::generate_initial_communities(int num_communities) {
    
    /*
        Creates an initial random community configuration for a given state
        Returns config as a Precinct Group object.
    */

    vector<Precinct_Group> communities;
    int num_precincts = state_precincts.size();
    // determine amount of precincts to be added to each community
    vector<int> sizes;
    int base = floor(num_precincts / num_communities); // the base num
    int rem = num_precincts % num_communities; // how many need to be increased by 1

    for (int i = 0; i < num_communities - rem; i++)
        sizes.push_back(base);
    for (int i = 0; i < rem; i++)
        sizes.push_back(base + 1);

    // create array of indices of precincts available to be added
    p_index_set available_pre(num_precincts);
    std::iota(available_pre.begin(), available_pre.end(), 0);
    
    int index = 0;
    for (int i = 0; i < num_communities - 1; i++) {
        Precinct_Group community;
        p_index p = get_inner_boundary_precincts(*this)[0];

        for (int x = 0; x < sizes[index] - 1; x++) {
            p = get_addable_precinct(available_pre, p);
            community.add_precinct(this->state_precincts[p]);
            available_pre.erase(remove(available_pre.begin(), available_pre.end(), p), available_pre.end());
        }

        communities.push_back(community);
        
        index++;
    }

    Precinct_Group community;
    for (p_index precinct : available_pre) {
        community.add_precinct(this->state_precincts[precinct]);
    }
    
    communities.push_back(community);
    return communities;
}

vector<Precinct_Group> refine_compactness(vector<Precinct_Group> communities) {
    return communities;
}

vector<Precinct_Group> refine_partisan(vector<Precinct_Group> communities) {
    return communities;
}

vector<Precinct_Group> refine_population(vector<Precinct_Group> communities) {
    return communities;
}

int measure_difference(vector<Precinct_Group> communities, vector<Precinct_Group> new_communities) {
    return 3;
}

vector<Precinct_Group> State::generate_communities(int num_communities, float compactness_tolerance, float partisanship_tolerance, float population_tolerance) {
    vector<Precinct_Group> communities = generate_initial_communities(num_communities);
    
    int changed_precincts = 0,
        arbitrary_limit = 100,
        i = 0;

    // while (changed_precincts > arbitrary_limit) {
    //     cout << "On iteration " << i << endl;

    //     vector<Precinct_Group> new_communities = 
    //         refine_compactness(
                    // refine_partisan(
                    //     refine_population(communities)
                    //     )
                    // );
        
    //     changed_precincts = measure_difference(communities, new_communities);
    //     i++;
    // }

    return communities;
}   
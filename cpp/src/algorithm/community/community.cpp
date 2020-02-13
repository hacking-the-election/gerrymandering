/*=======================================
 community.cpp:                 k-vernooy
 last modified:               Mon, Feb 10

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
#include <boost/filesystem.hpp>

const int CHANGED_PRECINT_TOLERANCE = 10; // percent of precincts that can change from iteration

void save_community_state(Communities communities, string write_path) {
    /*
        Saves a community to a file at a specific point in the
        pipeline. Useful for visualization and checks
    */
   int c_index = 0;
   boost::filesystem::create_directory(write_path);

   for (Community c : communities) {
       c.write_binary(write_path + "/community_" + to_string(c_index));
       c_index++;
   }
}


Communities State::generate_initial_communities(int num_communities) {
    
    /*
        Creates an initial random community configuration for a given state
        Returns config as a Precinct Group object.
    */

    Communities communities;
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
        Community community;
        p_index p = get_inner_boundary_precincts(*this)[0];

        for (int x = 0; x < sizes[index] - 1; x++) {
            p = get_addable_precinct(available_pre, p);
            community.add_precinct(this->state_precincts[p]);
            available_pre.erase(remove(available_pre.begin(), available_pre.end(), p), available_pre.end());
        }

        communities.push_back(community);
        
        index++;
    }

    // add the last community that has no precincts yet
    Community community;
    for (p_index precinct : available_pre) {
        community.add_precinct(this->state_precincts[precinct]);
    }
    
    communities.push_back(community); // add last community
    return communities;
}

Communities refine_compactness(Communities communities) {
    return communities;
}

Communities refine_partisan(Communities communities) {
    return communities;
}

Communities refine_population(Communities communities) {
    return communities;
}

int measure_difference(Communities communities, Communities new_communities) {
    
    /*
        Measures and returns how many precincts have changed communities
        in a given list of old and new communities. Used for checking when
        to stop the algorithm.
    */
    
    int changed_precincts;

    // loop through each community
    for (int i = 0; i < communities.size(); i++) {
        // for each precinct in the community
        for (int x = 0; x < communities[i].precincts.size(); x++) {
            // check for precinct in corrosponding new_community array
            Precinct old_p = communities[i].precincts[x];
            vector<Precinct> plist = new_communities[i].precincts;
            bool found = false;
            int index = 0;

            while (!found && index < plist.size()) {
                Precinct p = plist[index];
                if (old_p == p) found = true;
                index++;
            }

            if (found) changed_precincts++;
        }
    }

    return changed_precincts;
}

Communities State::generate_communities(int num_communities, float compactness_tolerance, float partisanship_tolerance, float population_tolerance) {
    Communities communities = generate_initial_communities(num_communities);
    
    int changed_precincts = 0,
        i = 0;

    int precinct_change_tolerance = (CHANGED_PRECINT_TOLERANCE / 100) * this->precincts.size();

    // while (changed_precincts > precinct_change_tolerance) {
    while (i < 30) {
        cout << "On iteration " << i << endl;

        Communities new_communities = 
            refine_compactness(
                    refine_partisan(
                        refine_population(communities)
                        )
                    );
        
        changed_precincts = measure_difference(communities, new_communities);
        cout << changed_precincts << " precincts changed" << endl;
        i++;
    }

    return communities;
}   
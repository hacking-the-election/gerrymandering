/*==================================================
 community.cpp:                        k-vernooy
 last modified:                      Mon, Feb 15

 Definition of the community-generation algorithm for
 quantifying gerrymandering and redistricting. This
 algorithm is the main result of the project
 hacking-the-election, and detailed techincal
 documents can be found in the /docs/ root folder of
 this repository.

 Visit https://hacking-the-election.github.io for
 more information on this project.

 Useful links for explanations, visualizations,
 results, and other information:

 Our data sources for actually running this
 algorithm can be seen at /data/ or on GitHub:
 https://github.com/hacking-the-election/data

 This algorithm has been implemented in C++ and in
 Python. If you're interested in contributing to the
 open source political movement, please contact us
 at: hacking.the.election@gmail.com
==================================================*/

#include "../../../include/shape.hpp"    // class definitions
#include "../../../include/util.hpp"     // array modification functions
#include "../../../include/geometry.hpp" // geometry modification, border functions

#include <math.h>    // for rounding functions
#include <numeric>   // for std::iota
#include <algorithm> // sorting, seeking algorithms


/*================================================
 Define constants to be used in the algorithm.    
 These will not be passed to the algorithm
 as arguments, as they define things like stop
 conditions.
=================================================*/
const int CHANGED_PRECINT_TOLERANCE = 10; // percent of precincts that can change from iteration
const int MAX_ITERATIONS = 3; // max number of times we can change a community

// id's for processes:
const int PARTISANSHIP = 0;
const int COMPACTNESS = 1;
const int POPULATION = 2;


void State::generate_initial_communities(int num_communities) {
    /*
        Creates an initial random community configuration for a given state
        Returns config as a Precinct Group object.
    */

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

        this->state_communities.push_back(community);
        index++;
    }

    // add the last community that has no precincts yet
    Community community;
    for (p_index precinct : available_pre) {
        community.add_precinct(this->state_precincts[precinct]);
    }
    
    this->state_communities.push_back(community); // add last community
}

void State::refine_compactness(float compactness_tolerance) {
}

p_index State::get_next_community(float tolerance, int process) {
    /*
        Returns next candidate community depending on which process
        the algorithm is currently running. Used in algorithm to determine
        next community to optimize.
    */

    p_index i = -1;

    if (process = PARTISANSHIP) {
        /*
            Find community with standard deviation of partisanship
            ratios that are most outside range of tolerance
        */
        float max = 0;
        p_index x = 0;

        for (Community c : state_communities) {
            float stdev = get_standard_deviation_partisanship(c);
            if (stdev > tolerance && stdev > max) {
                i = x;
                max = stdev;
            }
            x++;
        }
    }
    else if (process == COMPACTNESS) {

    }
    else if (process == POPULATION) {
        /*
            Find community that is farthest away from the
            average of (total_pop / number_districts)
        */
        int aim = get_population() / state_districts.size();
        vector<int> range = {aim - (int)(tolerance * aim), aim + (int)(tolerance * aim)};
        int max = 0; // largest difference
        p_index x = 0; // keep track of iteration
        
        for (Community c : state_communities) {
            int diff = abs(aim - c.get_population());
            if ((diff > max) && (diff > (int)(tolerance * aim))) {
                // community is outside range and more so than the previous one
                i = x;
                max = diff;
            }
            x++;
        }
    }

    return i;
}

void State::give_precinct(p_index precinct, p_index community, string t_type) {
    /*
        Performs a precinct transaction by giving `precinct` from `community` to
        a possible other community (dependent on which function it's being used for).
        This is the only way community borders can change.
    */

    Precinct precinct_shape = this->state_communities[community].precincts[precinct];
    
    // get communities that border the current community
    p_index_set bordering_communities_i = get_bordering_shapes(this->state_communities, this->state_communities[community]);
    // convert to actual shape array
    Communities bordering_communities;
    for (p_index i : bordering_communities_i)
        bordering_communities.push_back(this->state_communities[i]);

    // of those communities, get the ones that also border the precinct
    p_index_set exchangeable_communities_i = get_bordering_shapes(bordering_communities, precinct_shape);
    // convert to shape array
    Communities exchangeable_communities;
    for (p_index i : exchangeable_communities_i)
        exchangeable_communities.push_back(this->state_communities[i]);

    p_index exchange_choice;

    if (t_type == "partisan") {
        // get closest average to precinct
        float min = abs(get_median_partisanship(exchangeable_communities[0]) - precinct_shape.get_ratio());
        p_index choice = 0;
        p_index index = 0;

        for (int i = 1; i < exchangeable_communities.size(); i++) {
            index++;
            Community c = exchangeable_communities[i];
            float diff = abs(get_median_partisanship(c) - precinct_shape.get_ratio());
            if (diff < min) {
                min = diff;
                choice = index;
            }
        }
        exchange_choice = choice;
    }

    Community chosen_c = exchangeable_communities[exchange_choice];
    // add precinct to new community
    chosen_c.add_precinct(precinct_shape);
    // remove precinct from previous community
    this->state_communities[community].precincts.erase(this->state_communities[community].precincts.begin() + precinct);

    // update relevant borders after transactions
    for (Community c : bordering_communities)
        c.border = generate_exterior_border(c);
    this->state_communities[community].border = generate_exterior_border(this->state_communities[community]);
    
    return;
}

void State::refine_partisan(float partisanship_tolerance) {
    /*
        A function to optimize the partisanship of a community -
        attempts to minimize the stdev of partisanship of precincts
        within each community.
    */
    
    p_index worst_community = get_next_community(partisanship_tolerance, PARTISANSHIP);
    bool is_done = (worst_community == -1);
    vector<int> num_changes(state_communities.size());

    while (!is_done) {
        Community c = state_communities[worst_community];
        
        float median = get_median_partisanship(c);
        p_index worst_precinct = 0, x = 0;
        float diff = 0;

        for (Precinct p : c.precincts) {
            float t_diff = abs(median - p.get_ratio());
            if (t_diff > diff) diff = t_diff;
            worst_precinct = x;
            x++;
        }

        give_precinct(worst_precinct, worst_community, "partisan");
        num_changes[worst_community] += 1; // update the changelist
        // update worst_community, check stop condition
        worst_community = get_next_community(partisanship_tolerance, PARTISANSHIP);
        // if the community is within the tolerance, or if it has been modified too many times
        is_done = (worst_community == -1 || num_changes[worst_community] == MAX_ITERATIONS);
    }
}

void State::refine_population(float population_tolerance) {
    /* 
        Optimize the state's communities for population. Attempts
        to minimize difference in population across the state
        with a tolerance for acceptable +- percent difference
    */

    // find the first community to be optimized
    p_index worst_community = get_next_community(population_tolerance, POPULATION);
    bool is_done = (worst_community == -1); // initialize stop condition
    vector<int> num_changes(state_communities.size());

    // calculate the range each community's population should be within
    int aim = get_population() / state_districts.size();
    vector<int> ideal_range = {aim - (int)(population_tolerance * aim), aim + (int)(population_tolerance * aim)};
    
    // begin main iterative loop
    while (!is_done) {
        Community c = state_communities[worst_community];
        while (c.get_population() )

        // update the changelist
        num_changes[worst_community] += 1; 
        
        // check stop condition, update the community that needs to be optimized
        worst_community = get_next_community(population_tolerance, POPULATION);
        is_done = (worst_community == -1 || num_changes[worst_community] == MAX_ITERATIONS);
    }
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
                // loop through precincts in new community,
                // compare to precinct you're checking
                Precinct p = plist[index];
                if (old_p == p) found = true;
                index++;
            }

            // this precinct was not found, so add a changed precinct 
            if (!found) changed_precincts++;
        }
    }

    return changed_precincts;
}

void State::generate_communities(int num_communities, float compactness_tolerance, float partisanship_tolerance, float population_tolerance) {

    /*
        The driver method for the communities algorithm. This method does
        a lot, but the general process for running the political-community 
        generation algorithm is calling void state methods that modify state variables. 
        
        At the start, `generate_initial_communities` generates
        a random configuration. Then, it uses the iterative method to
        refine for a variable until the number of precincts that change is
        within a tolerance, set above (see CHANGED_PRECINCT_TOLERANCE).

        This State method returns nothing - to access results, check
        the state.state_communities property.
    */

    generate_initial_communities(num_communities);
    
    int changed_precincts = 0, i = 0;
    int precinct_change_tolerance = // the acceptable number of precincts that can change each iteration
        (CHANGED_PRECINT_TOLERANCE / 100) * this->precincts.size();

    Communities old_communities; // to store communities at the beginning of the iteration

    
    /*
        Do 30 iterations, and see how many precincts change each iteration
        !! This is only until we have a good idea for a stop condition. We
           first will plot results on a graph and regress to see the optimal
           point of minimal change.

        At some point this will be changed to:
           while (changed_precincts > precinct_change_tolerance)
    */
   
    while (i < 30) {
        cout << "On iteration " << i << endl;
        old_communities = this->state_communities;

        refine_compactness(compactness_tolerance);
        refine_partisan(partisanship_tolerance);
        refine_population(population_tolerance);
        
        changed_precincts = measure_difference(old_communities, this->state_communities);
        cout << changed_precincts << " precincts changed" << endl;
        i++;
    }
}
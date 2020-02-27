/*==================================================
 community.cpp:                        k-vernooy
 last modified:                      Mon, Feb 15

 Definition of the community-generation algorithm for
 quantifying gerrymandering and redistricting. This
 algorithm is the main result of the project
 hacking-the-election, and detailed techincal
 documents can be found in the /docs root folder of
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
#include <numeric>   // include std::iota
#include <algorithm> // sorting, seeking algorithms

using namespace std;
using namespace GeoGerry;

#define VERBOSE 1
#define WRITE 0

/*
    Define constants to be used in the algorithm.    
    These will not be passed to the algorithm
    as arguments, as they define things like stop
    conditions.
*/

const int CHANGED_PRECINT_TOLERANCE = 10; // percent of precincts that can change from iteration
const int MAX_ITERATIONS = 3; // max number of times we can change a community

// ids for processes:
const int PARTISANSHIP = 0;
const int COMPACTNESS = 1;
const int POPULATION = 2;

void State::generate_initial_communities(int num_communities) {
    /*
        Creates an initial random community configuration for a given state.
        Reserves precincts on islands to prevent bad island linking.
        
        Fractional islands are defined here as those that do not fit
        an even combination of communities, and have leftover precincts that
        must be added to a separate community.

        Modifys the state's internal communities array so as to return void.
    */

    int num_precincts = precincts.size(); // total precinct amount

    vector<int> large_sizes; // number of communities of greater amount
    vector<int> base_sizes;

    int base = floor(num_precincts / num_communities); // the base num
    int rem = num_precincts % num_communities; // how many need to be increased by 1

    for (int i = 0; i < num_communities - rem; i++)
        base_sizes.push_back(base);
    for (int i = 0; i < rem; i++)
        large_sizes.push_back(base + 1);
    
    vector<p_index_set> available_precincts = islands;      // precincts that have yet to be used up
    p_index_set fractional_islands;                         // indices of islands that can't be made of base and large
    Communities c;                                          // Set of communities

    int island_index = 0;

    for (p_index_set island : islands) {
        /*
            Determine the amount of whole communities can
            be fit on each island. If an island contains fractional
            communities, add it to
        */

        map<int, array<int, 2> > vals; // to hold possible size combinations

        for (int x = 1; x <= large_sizes.size(); x++) {
            for (int y = 1; y <= base_sizes.size(); y++) {
                vals[(x * (base + 1)) + (y * base)] = {x, y};
            }
        }

        int x = 0; // number of base communities on island
        int y = 0; // number of base + 1 communities on island

        auto it = vals.find(island.size());
        
        if (it != vals.end()) {
            // this island can be made from whole communities
            x = vals[island.size()][1];
            y = vals[island.size()][0];
        }
        else {
            // this island must contain a fractional community
            fractional_islands.push_back(island_index);

            // Find the number of whole communities it can contain
            // regardless, by rounding down to the nearest array element
            int round;
            
            for (auto it = vals.begin(); it != vals.end(); it++) {
                if (it->first > island.size())
                    break;
                round = it->first;
            }
            
            x = vals[round][1];
            y = vals[round][0];
        }

        for (int i = 0; i < x; i++) {
            base_sizes.pop_back();
            Community com;
            com.size.push_back(base);
            com.location.push_back(island_index);
            c.push_back(com);
        }
        for (int j = 0; j < y; j++) {
            large_sizes.pop_back();
            Community com;
            com.size.push_back(base + 1);
            com.location.push_back(island_index);
            c.push_back(com);
        }

        island_index++;
    }

    vector<p_index> ignore_fractionals; // fractional islands to be ignored, not removed

    for (p_index fractional_island_i = 0; fractional_island_i < fractional_islands.size(); fractional_island_i++) {
        /*
            Loop through all ƒractional islands - those that need precincts
            from other islands to create communities - and create community
            objects with links
        */

        if (!(std::find(ignore_fractionals.begin(), ignore_fractionals.end(), fractional_islands[fractional_island_i]) != ignore_fractionals.end())) {
            // create community with location information
            Community community;
            community.location.push_back(fractional_islands[fractional_island_i]);
            p_index island_i = fractional_islands[fractional_island_i];
            p_index_set island = islands[island_i];

            // get average center of island from precinct centers
            coordinate island_center = {0,0};
            for (p_index p : island) {
                coordinate p_center = precincts[p].get_center();
                island_center[0] += p_center[0];
                island_center[1] += p_center[1];
            }
            if (fractional_islands[fractional_island_i] == 11) cout << island_center[0] << ", " << island_center[1] << ", " << island.size() << endl;
            island_center[0] /= island.size();
            island_center[1] /= island.size();

            int island_leftover = islands[fractional_islands[fractional_island_i]].size();

            for (Community community_c : c) {
                // subtract precincts that are taken up by communities already
                auto it = find(community_c.location.begin(), community_c.location.end(), fractional_islands[fractional_island_i]);
                if (it != community_c.location.end()) {
                    cout << community_c.size.size() << ", " << community_c.location.size() << endl;
                    island_leftover -= community_c.size[std::distance(community_c.location.begin(), it)]; // subtract whole communitites that are already added
                    // cout << "already has community sized " << community_c.size[std::distance(community_c.location.begin(), it)] << " at index " << std::distance(community_c.location.begin(), it) << endl;
                }
            }
            // cout << "New base island " << fractional_islands[fractional_island_i] << " which has size " << island_leftover << endl;

            community.size.push_back(island_leftover); // island leftover now contains # of available precincts
            int total_community_size; // the ultimate amount of precincts in the community

            // size the current community correctly
            if (large_sizes.size() > 0) {
                // there are still large sizes available
                total_community_size = base + 1;
                large_sizes.pop_back();
            }
            else {
                // there are only base sizes available
                total_community_size = base;
                base_sizes.pop_back();
            }

            // cout << "Community being made of size " << total_community_size << endl;
            int total_leftover = total_community_size - island_leftover;

            // total_leftover contains the amount of precincts still needed to be added

            while (total_leftover > 0) {
                // find the closest fractional island that can be linked
                double min_distance = pow(10, 80); // arbitrarily high number (easy min)
                p_index min_index = -1;

                // cout << ignore_fractionals.size() << endl;

                for (int compare_island = 0; compare_island < fractional_islands.size(); compare_island++) {
                    // find closest fractional community to link
                    if (compare_island != fractional_island_i && 
                        !(std::find(ignore_fractionals.begin(), ignore_fractionals.end(), fractional_islands[compare_island]) != ignore_fractionals.end())) {
                        // get average center of island from precinct centers
                        p_index_set island_c = islands[fractional_islands[compare_island]];
                        coordinate island_center_c = {0,0};

                        for (p_index p : island_c) {
                            coordinate p_center = precincts[p].get_center();
                            island_center_c[0] += p_center[0];
                            island_center_c[1] += p_center[1];
                        }

                        // average coordinates
                        island_center_c[0] /= island_c.size();
                        island_center_c[1] /= island_c.size();

                        // get distance to current island
                        double dist = get_distance(island_center, island_center_c);
                        if (fractional_islands[fractional_island_i] == 11) cout << island_center_c[0] << ", " << island_center_c[1] << ", " << island_center[0] << ", " << island_center[1] << endl;
                        // if this is the lowest distance, then update values
                        if (dist < min_distance) {
                            min_distance = dist;
                            min_index = compare_island;
                        }
                    }
                }

                // cout << "min index " << min_index << endl;

                int island_leftover_c = islands[fractional_islands[min_index]].size(); // size of island
                for (Community community_c : c) {
                    auto it = find(community_c.location.begin(), community_c.location.end(), fractional_islands[min_index]);
                    if (it != community_c.location.end()) {
                        island_leftover_c -= community_c.size[std::distance(community_c.location.begin(), it)]; // subtract sizes of communities on island
                    }
                }

                // island_leftover_c contains amount of available precincts on linking island
                // cout << "linking " << fractional_islands[fractional_island_i] << " and " << fractional_islands[min_index] << endl;

                community.is_linked = true;
                community.location.push_back(fractional_islands[min_index]);
                if (total_leftover - island_leftover_c < 0) {
                    int sum = 0;
                    for (int t : community.size)
                        sum += t;

                    // cout << "community size is now " << total_leftover + sum << endl;
                    community.size.push_back(total_leftover);
                }
                else {
                    int sum = 0;
                    for (int t : community.size)
                        sum += t;

                    // cout << "community size is now " << island_leftover_c + sum << endl;
                    community.size.push_back(island_leftover_c);
                }

                total_leftover -= island_leftover_c;

                if (total_leftover >= 0) {
                    // cout << "precincts left to add: " << total_leftover << endl << endl;
                    ignore_fractionals.push_back(fractional_islands[min_index] );
                }
                else {
                    int sum = 0;
                    for (int t : community.size)
                        sum += t;

                    // cout << "finished linking with size " << sum <<  " and total leftover of " << total_leftover << endl << endl;
                }

                // must now link compare_island and island
                // p_index link;
                // for (p_index p : islands[fractional_islands[min_index]]) {
                    
                // }
            }


            int t = 0;
            for (int s : community.size)
                t += s;

            c.push_back(community);
            // cout << "There are " << precincts.size() - total_pre_used << " precincts left to be claimed" << endl;
            ignore_fractionals.push_back(fractional_islands[fractional_island_i]);
        }

    }  // fractional linker

    cout << "there are " << c.size() << " communities now" << endl;

    for (Community community : c) {
        // fill linked communities with generation method
        for (int i = 0; i < community.location.size(); i++) {
            // get information about the current community
            int size  = community.size[i];
            int island_i = community.location[i];
            p_index_set precincts = available_precincts[i]; 

            for (int x = 0; x < size; x++) {
                int precinct = -1;
                vector<p_index> tried_precincts; // precincts that will create islands

                do {

                } while (precinct == -1);

                community.add_precinct(precinct);
            }
        }
    }
    // for (Community community : c) {
    //     if (!community.is_linked) {
    //         // fill community
    //     }
    // }

    // // add the last community that has no precincts yet
    // Community community;
    // for (p_index precinct : available_pre) {
    //     community.add_precinct(this->precincts[precinct]);
    // }
    
    this->state_communities = c; // add last community
}

p_index State::get_next_community(double tolerance, int process) {
    /*
        Returns next candidate community depending on which process
        the algorithm is currently running. Used in algorithm to determine
        next community to optimize.
    */

    p_index i = -1;

    if (process == PARTISANSHIP) {
        /*
            Find community with standard deviation of partisanship
            ratios that are most outside range of tolerance
        */
        double max = 0;
        p_index x = 0;

        for (Community c : state_communities) {
            double stdev = get_standard_deviation_partisanship(c);
            if (stdev > tolerance && stdev > max) {
                i = x;
                max = stdev;
            }
            x++;
        }
    }
    else if (process == COMPACTNESS) {
        unit_interval min = 1;
        p_index x = 0;

        for (Community c : state_communities) {
            unit_interval t_compactness = c.get_compactness();
            if (t_compactness < min && t_compactness < tolerance) {
                min = t_compactness;
                i = x;
            }
            x++;
        }
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

void State::give_precinct(p_index precinct, p_index community, int t_type) {
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

    if (t_type == PARTISANSHIP) {
        // get closest average to precinct
        double min = abs(get_median_partisanship(exchangeable_communities[0]) - precinct_shape.get_ratio());
        p_index choice = 0;
        p_index index = 0;

        for (int i = 1; i < exchangeable_communities.size(); i++) {
            index++;
            Community c = exchangeable_communities[i];
            double diff = abs(get_median_partisanship(c) - precinct_shape.get_ratio());
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
        c.border = generate_exterior_border(c).border;

    this->state_communities[community].border = generate_exterior_border(this->state_communities[community]).border;
    
    return;
}

void State::refine_compactness(double compactness_tolerance) {
    /* 
        Optimize the state's communities for population. Attempts
        to minimize difference in population across the state
        with a tolerance for acceptable +- percent difference
    */
    p_index worst_community = get_next_community(compactness_tolerance, COMPACTNESS);
    bool is_done = (worst_community == -1);
    vector<int> num_changes(state_communities.size());

    while (!is_done) {
        num_changes[worst_community] += 1; // update the changelist
        // update worst_community, check stop condition
        worst_community = get_next_community(compactness_tolerance, COMPACTNESS);
        // if the community is within the tolerance, or if it has been modified too many times
        is_done = (worst_community == -1 || num_changes[worst_community] == MAX_ITERATIONS);
    }
}

void State::refine_partisan(double partisanship_tolerance) {
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
        
        double median = get_median_partisanship(c);
        p_index worst_precinct = 0, x = 0;
        double diff = 0;

        for (Precinct p : c.precincts) {
            double t_diff = abs(median - p.get_ratio());
            if (t_diff > diff) diff = t_diff;
            worst_precinct = x;
            x++;
        }

        give_precinct(worst_precinct, worst_community, PARTISANSHIP);
        num_changes[worst_community] += 1; // update the changelist
        // update worst_community, check stop condition
        worst_community = get_next_community(partisanship_tolerance, PARTISANSHIP);
        // if the community is within the tolerance, or if it has been modified too many times
        is_done = (worst_community == -1 || num_changes[worst_community] == MAX_ITERATIONS);
    }
}

void State::refine_population(double population_tolerance) {
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
        while (c.get_population() < ideal_range[0] || c.get_population() > ideal_range[1]) {
            //!! Figure out which method we're using for precinct exchange
        }

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
            // check for precinct in corresponding new_community array
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

void State::generate_communities(int num_communities, double compactness_tolerance, double partisanship_tolerance, double population_tolerance) {
    /*
        The driver method for the communities algorithm. The general process for
        running the political-community generation algorithm is calling void state
        methods that modify state variables, so not much passing around is done. 
        
        At the start, `generate_initial_communities` generates
        a random configuration. Then, it uses the iterative method to
        refine for a variable until the number of precincts that change is
        within a tolerance, set above (see CHANGED_PRECINCT_TOLERANCE).

        This State method returns nothing - to access results, check
        the state.state_communities property.

        The #defined WRITE determines whether or not to write binary
        community objects to a predefined directory. These can then be loaded
        in order to visualize algorithms. See the `community_playback` function
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
   
    // while (i < 30) {
    //     cout << "On iteration " << i << endl;
    //     old_communities = this->state_communities;

    //     if (VERBOSE) cout << "refining compacntess..." << endl;
    //     refine_compactness(compactness_tolerance);
        
    //     if (VERBOSE) cout << "refining partisanship..." << endl;
    //     refine_partisan(partisanship_tolerance);
        
    //     if (VERBOSE) cout << "refining population..." << endl;
    //     refine_population(population_tolerance);
  
    //     if (VERBOSE) cout << "measuring precincts changed..." << endl;      
    //     changed_precincts = measure_difference(old_communities, this->state_communities);
    //     if (VERBOSE) cout << changed_precincts << " precincts changed." << endl;
        
    //     i++;
    // }
}
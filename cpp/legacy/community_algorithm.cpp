/*===============================================
 community.cpp:                        k-vernooy
 last modified:                     Fri, Feb 28
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
===============================================*/


#include <math.h>    // for rounding functions
#include <numeric>   // include std::iota
#include <boost/filesystem.hpp>

#include "../../../include/shape.hpp"    // class definitions
#include "../../../include/util.hpp"     // array modification functions
#include "../../../include/geometry.hpp" // geometry modification, border functions
#include "../../../include/canvas.hpp" // geometry modification, border functions

using namespace std;
using namespace GeoGerry;
using namespace GeoDraw;
using namespace boost::filesystem;

#define VERBOSE 1
#define WRITE 0
#define DEBUG_COMMUNITIES 1

/*
    Define constants to be used in the algorithm.    
    These will not be passed to the algorithm
    as arguments, as they define things like stop
    conditions.
*/

const int CHANGED_PRECINT_TOLERANCE = 10; // percent of precincts that can change from iteration
const int MAX_ITERATIONS = 13; // max number of times we can change a community
int TOTAL_MOVED_PRECINCTS = 0;  // number of times a precinct has been given to another district
vector<string> TOTAL_MOVED_PRECINCT_ID = {};

Anim full_animation(40);


void State::generate_initial_communities(int num_communities) {
    /*
        @desc:
            Updates states communities with initial random community configuration.
            Reserves precincts on islands to prevent bad island linking.
            Fractional islands are defined here as those that do not fit
            an even combination of communities, and have leftover precincts that
            must be added to a separate community.
    
        @params: `int` num_communities: the number of initial communities to generate
        @return: `void`
    */

    int num_precincts = precincts.size(); // total precinct amount
    vector<int> large_sizes; // number of communities of greater amount
    vector<int> base_sizes;

    int base = floor(num_precincts / num_communities); // the base num
    int rem = num_precincts % num_communities; // how many need to be increased by 1

    for (int i = 0; i < num_communities - rem; i++) base_sizes.push_back(base);
    for (int i = 0; i < rem; i++) large_sizes.push_back(base + 1);
    
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

        for (int x = 1; x <= base_sizes.size(); x++) {
            if (large_sizes.size() == 0) vals[(x * base)] = {x, 0};
            for (int y = 1; y <= large_sizes.size(); y++) {
                vals[(x * (base)) + (y * (base + 1))] = {x, y};
            }
        }

        int x = 0; // number of base communities on island
        int y = 0; // number of base + 1 communities on island


        if (vals.find(island.size()) != vals.end()) {
            // this island can be made from whole communities
            x = vals[island.size()][1];
            y = vals[island.size()][0];
        }
        else {
            // this island must contain a fractional community
            fractional_islands.push_back(island_index);

            // find the number of whole communities it can contain
            // regardless, by rounding down to the nearest array element
            int round;
            
            for (auto it = vals.begin(); it != vals.end(); it++) {
                if (it->first > island.size()) break;
                round = it->first;
            }
            
            x = vals[round][1];
            y = vals[round][0];
        }

        for (int i = 0; i < x; i++) {
            if (base_sizes.size() != 0) base_sizes.erase(base_sizes.begin());
            Community com;
            com.size.push_back(base);
            com.location.push_back(island_index);
            c.push_back(com);
        }

        for (int j = 0; j < y; j++) {
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
            Have fun trying to understand this code tomorrow
        */

        if (VERBOSE) cout << "linking fractional communities... " << fractional_islands.size() << endl;
        int n_fractional_island_i = fractional_island_i;

        if (!(std::find(ignore_fractionals.begin(), ignore_fractionals.end(), fractional_islands[fractional_island_i]) != ignore_fractionals.end())) {
            // create community with location information
            Community community;
            community.location.push_back(fractional_islands[fractional_island_i]);
            p_index island_i = fractional_islands[fractional_island_i];
            p_index_set island = islands[island_i];

            // get average center of island from precinct centers
            array<long long int, 2> island_center = {0,0};

            for (p_index p : island) {
                coordinate p_center = precincts[p].get_center();
                island_center[0] += p_center[0];
                island_center[1] += p_center[1];
            }

            island_center[0] = island_center[0] / island.size();
            island_center[1] = island_center[1] / island.size();

            int island_leftover = islands[fractional_islands[fractional_island_i]].size();

            for (Community community_c : c) {
                // subtract precincts that are taken up by communities already
                auto it = find(community_c.location.begin(), community_c.location.end(), fractional_islands[fractional_island_i]);
                if (it != community_c.location.end())
                    island_leftover -= community_c.size[std::distance(community_c.location.begin(), it)]; // subtract whole communitites that are already added
            }

            community.size.push_back(island_leftover); // island leftover now contains # of available precincts
            int total_community_size; // the ultimate amount of precincts in the community

            // size the current community correctly
            if (large_sizes.size() > 0) {
                // there are still large sizes available
                total_community_size = base + 1;
                if (large_sizes.size() != 0) large_sizes.erase(large_sizes.begin());
            }
            else {
                // there are only base sizes available
                total_community_size = base;
                if (base_sizes.size() != 0) base_sizes.erase(base_sizes.begin());
            }

            if (DEBUG_COMMUNITIES) cout << "need to make community of " << total_community_size << endl;
            // total_leftover contains the amount of precincts still needed to be added
            int total_leftover = total_community_size - island_leftover;

            while (total_leftover > 0) {
                if (DEBUG_COMMUNITIES) cout << "need community to link with " << n_fractional_island_i << endl;
                // find the closest fractional island that can be linked
                double min_distance = pow(10, 80); // arbitrarily high number (easy min)
                p_index min_index = -1;
                array<long long int, 2> min_island_center = {0,0};

                for (int compare_island = 0; compare_island < fractional_islands.size(); compare_island++) {
                    // find closest fractional community to link
                    if (compare_island != n_fractional_island_i && 
                        !(std::find(ignore_fractionals.begin(), ignore_fractionals.end(), fractional_islands[compare_island]) != ignore_fractionals.end())) {
                        // get average center of island from precinct centers
                        p_index_set island_c = islands[fractional_islands[compare_island]];
                        array<long long int, 2> island_center_c = {0,0};  

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
                        // if this is the lowest distance, then update values
                        if (dist < min_distance) {
                            min_distance = dist;
                            min_index = compare_island;
                            min_island_center = island_center_c;
                        }
                    }
                }

                int island_leftover_c = islands[fractional_islands[min_index]].size(); // size of island
                for (Community community_c : c) {
                    auto it = find(community_c.location.begin(), community_c.location.end(), fractional_islands[min_index]);
                    if (it != community_c.location.end())
                        island_leftover_c -= community_c.size[std::distance(community_c.location.begin(), it)]; // subtract sizes of communities on island
                }

                // island_leftover_c contains amount of available precincts on linking island
                if (DEBUG_COMMUNITIES) cout << "closeset community to link to is " << min_index << endl;
                p_index link, min_link;
                double min_p_distance = pow(10, 80); // arbitrarily high number (easy min)
                int i = 0;

                // find the precinct closest to the center of the island
                if (fractional_island_i == n_fractional_island_i) {
                    p_index_set ignore_p;
                    cout << "generating first link" << endl;
                    do {
                        for (p_index p : get_inner_boundary_precincts(islands[fractional_islands[min_index]], *this)) {
                        // for (p_index p : islands[fractional_islands[min_index]]) {
                            array<long long int, 2> p_center = {(long long int) precincts[p].get_center()[0], (long long int) precincts[p].get_center()[1]};
                            double dist = get_distance(p_center, island_center);

                            if (dist < min_p_distance && (std::find(ignore_p.begin(), ignore_p.end(), p) == ignore_p.end())) {
                                min_p_distance = dist;
                                link = i;
                            }
                            i++;
                        }

                        ignore_p.push_back(link);

                    } while (creates_island(islands[fractional_islands[min_index]], link, *this));
                }
                else {
                    link = community.link_position[community.link_position.size() - 1][1][1];
                }

                p_index_set ignore_p = {};
                min_p_distance = pow(10, 80);
                i = 0;

                do {
                    for (p_index p : get_inner_boundary_precincts(islands[fractional_islands[n_fractional_island_i]], *this)) {
                        array<long long int, 2> p_center = {(long long int) precincts[p].get_center()[0], (long long int) precincts[p].get_center()[1]};
                        double distc = get_distance(p_center, min_island_center);

                        if (distc < min_p_distance && (std::find(ignore_p.begin(), ignore_p.end(), p) == ignore_p.end())) {
                            min_p_distance = distc;
                            min_link = i;
                        }
                        i++;
                    }

                    ignore_p.push_back(link);

                } while (creates_island(islands[fractional_islands[n_fractional_island_i]], min_link, *this));

                if (DEBUG_COMMUNITIES) cout << "linking precinct " << link << " on " << n_fractional_island_i << " with " << min_link << " on " << min_index << endl;
                // cout << available_precincts.size() << endl;
                // cout << available_precincts[n_fractional_island_i].size() << endl;

                if (available_precincts[n_fractional_island_i].size() > link)
                    available_precincts[n_fractional_island_i].erase(available_precincts[n_fractional_island_i].begin() + link);
                if (available_precincts[min_index].size() > min_link)
                    available_precincts[min_index].erase(available_precincts[min_index].begin() + min_link);

                // set community meta information
                community.link_position.push_back({{n_fractional_island_i, link}, {min_index, min_link}});
                community.is_linked = true;
                community.location.push_back(fractional_islands[min_index]);

                
                if (total_leftover - island_leftover_c < 0) {
                    community.size.push_back(total_leftover);  
                }
                else {
                    community.size.push_back(island_leftover_c);
                    if (DEBUG_COMMUNITIES) cout << "Adding " << island_leftover_c << " precincts to community" << endl;
                }

                total_leftover -= island_leftover_c;

                // if used up a whole island, still have precincts you need to get from somewhere
                if (total_leftover >= 0) {
                    ignore_fractionals.push_back(fractional_islands[min_index]);
                    if (DEBUG_COMMUNITIES) cout << total_leftover << " precincts left" << endl;
                }

                ignore_fractionals.push_back(fractional_islands[n_fractional_island_i]);

                n_fractional_island_i = min_index;
                island_center = min_island_center;
            }

            int sum = 0;
            for (int s : community.size)
                sum += s;

            if (DEBUG_COMMUNITIES) cout << endl << endl;
            c.push_back(community);
        }

    }  // fractional linker

    if (VERBOSE) cout << "filling communities with precincts..." << endl;

    for (int c_index = 0; c_index < c.size() - 1; c_index++) {
        cout << "starting community " << c_index << endl;
        Community community = c[c_index];

        for (int i = 0; i < community.location.size(); i++) {
            // get information about the current community
            int size  = community.size[i];
            int island_i = community.location[i];
            p_index_set island_available_precincts = available_precincts[i]; 
            cout << "getting available precincts..." << flush;
            
            Precinct_Group island_available_shape;
            for (p_index pre : island_available_precincts) {
                Precinct pr = precincts[pre];
                pr.shape_id = to_string(pre);
                island_available_shape.add_precinct_n(pr);
            }

            island_available_shape.border = generate_exterior_border(island_available_shape).border;

            cout << "got" << endl;

            cout << "calculating start precinct..." << flush;
            p_index_set inner_precincts = get_inner_boundary_precincts(island_available_shape);

            p_index start_precinct = stoi(island_available_shape.precincts[inner_precincts[0]].shape_id);

            Canvas canvas(900,900);
            canvas.add_shape(*this);
            canvas.add_shape(precincts[start_precinct], false, Color(200, 50, 50), 2);
            canvas.draw();

            community.add_precinct_n(precincts[start_precinct]);
            island_available_shape.remove_precinct(precincts[start_precinct]);
            island_available_precincts.erase(
                    std::remove(
                        island_available_precincts.begin(),
                        island_available_precincts.end(),
                        start_precinct
                    ),
                    island_available_precincts.end()
                );


            int precincts_to_add = size;
            int precincts_added = 1;

            while (precincts_added < precincts_to_add) {
                int precinct = -1;
                p_index_set tried_precincts; // precincts that will create islands
                p_index start; // random precinct in border thats not linked

                // calculate border, avoid multipoly
                p_index_set bordering_precincts = get_ext_bordering_precincts(community, island_available_precincts, *this);
                bool can_do_one = false;

                for (p_index pre : bordering_precincts) {
                    if (precincts_added < precincts_to_add && !creates_island(island_available_shape, precincts[pre])) {
                        can_do_one = true;
                        cout << "adding precinct " << pre << endl;

                        island_available_shape.remove_precinct(precincts[pre]);
                        island_available_precincts.erase(
                                std::remove(
                                    island_available_precincts.begin(),
                                    island_available_precincts.end(),
                                    pre
                                ),
                                island_available_precincts.end()
                            );

                        community.add_precinct_n(precincts[pre]);
                        precincts_added++;
                    }
                    else cout << "creates island, refraining..." << endl;
                }

                if (!can_do_one) {
                    cout << "No precinct exchanges work!!" << endl;
                    exit(1);
                }
            }
            available_precincts[i] = island_available_precincts; 
        }
        c[c_index] = community;
    }

    for (p_index_set p : available_precincts) {
        for (p_index pi : p ) {
            c[c.size() - 1].add_precinct_n(precincts[pi]);
        }
    }

    this->state_communities = c; // assign state communities to generated array

    for (int i = 0; i < state_communities.size(); i++) {
        state_communities[i].border = generate_exterior_border(state_communities[i]).border;
    }

    return;
}


p_index State::get_next_community(double tolerance, int process) {
    /*
        @desc:
            gets next candidate community depending on which process
            the algorithm is currently running. Used in algorithm to determine
            next community to optimize.
    
        @params:
            `double` tolerance: the tolerance for any process
            `int` process: the id of the current running process. These are
                           defined in the top of this file as constants
        @return: `p_index` community to modify next
    */

    p_index i = -1;

    if (process == PARTISANSHIP) {
        /*
            Find community with standard deviation of partisanship
            ratios that are most outside range of tolerance
        */

        // double max = 0;
        // p_index x = 0;

        // for (Community c : state_communities) {
        //     double stdev = get_standard_deviation_partisanship(c);
        //     if (stdev > tolerance && stdev > max) {
        //         i = x;
        //         max = stdev;
        //     }
        //     x++;
        // }

        p_index x = rand_num(0, state_communities.size() - 1);
        // int itermax = state_communities.size() * 2;
        // int iter = 0;

        // do {
        //     x = rand_num(0, state_communities.size() - 1);
        //     iter++;
        // } while (get_standard_deviation_partisanship(state_communities[x]) < tolerance && iter < itermax);
        i = x;
    }
    else if (process == COMPACTNESS) {
        unit_interval min = 1;
        p_index x = 0;
        // int iter = 1;
        // while (state_communities[x].get_compactness() > tolerance) {
        //     x = rand_num(0, state_communities.size() - 1);
        //     iter++;
        //     if (iter == state_communities.size()) {
        //         x = -1;
        //         break;            
        //     }
        // }

        // i = x;
        
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

        int aim = get_population() / state_communities.size();
        vector<int> range = {aim - (int)(tolerance * aim), aim + (int)(tolerance * aim)};
        int max = 0; // largest difference
        int maxpop = 0;
        p_index x = 0; // keep track of iteration
        
        for (Community c : state_communities) {
            if (c.get_population() > maxpop && c.get_population() > range[1]) {
                // community is outside range and more so than the previous one
                i = x;
                maxpop = c.get_population();
            }
            x++;
        }
    }

    return i;
}


bool State::give_precinct(p_index precinct, p_index community, int t_type, bool animate) {
    /*
        @desc: 
            performs a precinct transaction by giving `precinct` from `community` to
            a possible other community (dependent on which function it's being used for).
            This is the only way community borders can change.
        @params:
            `p_index` precinct: The position of the precinct to give in the community
            `p_index` community: The position of the community in the state array
            `int` t_type: the currently running process (consts defined in community.cpp)
        @return: void
    */

    Precinct precinct_shape = this->state_communities[community].precincts[precinct];

    // Canvas canvas(900, 900);
    // canvas.add_shape(this->state_communities);
    // canvas.add_shape(this->state_communities[community], true, Color(100,255,0), 1);
    // canvas.add_shape(precinct_shape, true, Color(0,100,255), 2);
    // canvas.draw();
    
    // get communities that border the current community
    p_index_set bordering_communities_i = get_bordering_shapes(this->state_communities, this->state_communities[community]);

    // convert to actual shape array
    Communities bordering_communities;
    for (p_index i : bordering_communities_i)
        bordering_communities.push_back(this->state_communities[i]);

    // of those communities, get the ones that also border the precinct
    p_index_set exchangeable_communities_i = get_bordering_shapes(bordering_communities, precinct_shape);
    Communities exchangeable_communities;
    if (exchangeable_communities_i.size() == 0) return false;

    for (int i = 0; i < exchangeable_communities_i.size(); i++) {
        exchangeable_communities_i[i] = bordering_communities_i[exchangeable_communities_i[i]];
        exchangeable_communities.push_back(this->state_communities[exchangeable_communities_i[i]]);
    }
    
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
    else if (t_type == COMPACTNESS) {
        // get highest compactness score
        double min = exchangeable_communities[0].get_compactness();
        p_index choice = 0;

        for (int i = 1; i < exchangeable_communities.size(); i++) {
            Community c = exchangeable_communities[i];
            double n_compactness = c.get_compactness();
            if (n_compactness < min) {
                min = n_compactness;
                choice = i;
            }
        }

        exchange_choice = choice;
    }
    else if (t_type == POPULATION) {
        int min = exchangeable_communities[0].get_population();
        p_index choice = 0;
         
        for (int i = 1; i < exchangeable_communities.size(); i++) {
            Community c = exchangeable_communities[i];
            int pop = c.get_population();

            if (pop < min) {
                min = pop;
                choice = i;
            }
        }

        exchange_choice = choice;
    }

    exchange_choice = exchangeable_communities_i[exchange_choice];

    // add precinct to new community
    this->state_communities[exchange_choice].add_precinct(precinct_shape);
    this->state_communities[community].remove_precinct(precinct_shape);
    
    if (animate) {
        Canvas can(900, 900);
        can.add_shape(this->state_communities);
        can.add_shape(Multi_Polygon(this->state_communities[community].border), false, Color(247, 42, 42), 2);
        full_animation.frames.push_back(can);
        TOTAL_MOVED_PRECINCTS++;
        TOTAL_MOVED_PRECINCT_ID.push_back(precinct_shape.shape_id);
    }

    return true;
}


bool State::give_precinct(p_index precinct, p_index community, p_index community_give, bool p, bool animate) {
    /*
        @desc: 
            performs a precinct transaction by giving `precinct` from `community` to
            a possible other community specified in the parameters.
            This is the only way community borders can change.
        @params:
            `p_index` precinct: The position of the precinct to give in the community
            `p_index` community: The position of the community in the state array
            `p_index` community_give: The community to give to
        @return: void
    */

    Precinct precinct_shape = this->state_communities[community].precincts[precinct];

    // add precinct to new community
    this->state_communities[community_give].add_precinct(precinct_shape);
    this->state_communities[community].remove_precinct(precinct_shape);
    
    if (animate) {
        Canvas can(900, 900);
        can.add_shape(this->state_communities);
        can.add_shape(Multi_Polygon(this->state_communities[community].border), false, Color(247, 42, 42), 2);
        full_animation.frames.push_back(can);
        TOTAL_MOVED_PRECINCTS++;
        TOTAL_MOVED_PRECINCT_ID.push_back(precinct_shape.shape_id);
    }

    return true;
}


void State::refine_compactness(double compactness_tolerance) {
    /* 
        @desc:
            Optimize the state's communities for population. Attempts
            to minimize difference in population across the state
            with a tolerance for acceptable +- percent difference
        @params: `double` compactness_tolerance: tolerance for compactness
        @return: void
    */

    p_index worst_community = get_next_community(compactness_tolerance, COMPACTNESS);

    bool is_worst = false;
    for (int i = 0; i < state_communities.size(); i++) {
        if (state_communities[i].get_compactness() < compactness_tolerance) {
            is_worst = true;
            break;
        }
    }

    bool is_done = (!is_worst);
    bool override = false;

    int iter = 0;
    int loop = 0;
    double first_average = 0;

    for (Community c : state_communities)
        first_average += c.get_compactness();
    first_average /= state_communities.size();

    cout << "refining for compactness" << endl;

    while (!is_done) {
        Polygon circle;
        coordinate center = state_communities[worst_community].get_center();
        circle = generate_gon(center, sqrt(state_communities[worst_community].get_area() / PI), 30);
        p_index_set giveable = get_inner_boundary_precincts(state_communities[worst_community]);

        for (Community c : this->state_communities)
            cout << c.get_compactness() << ", ";
        cout << endl;

        // for  each precinct in edge of community;
        for (int x = 0; x < giveable.size(); x++) {
            Precinct pre = state_communities[worst_community].precincts[giveable[x]];

            if (state_communities[worst_community].get_compactness() < compactness_tolerance && !get_inside_first(pre.hull, circle.hull) && !creates_island(state_communities[worst_community], giveable[x])) {
                if (give_precinct(giveable[x], worst_community, COMPACTNESS, true))
                    for (int i = 0; i < giveable.size(); i++) giveable[i] = giveable[i] - 1;
            }
        }

        p_index old_c = worst_community;

        // update worst_community, check stop condition
        int old_worst = worst_community;
        worst_community = get_next_community(compactness_tolerance, COMPACTNESS);
        if (worst_community == old_worst) loop++;
        else loop = 0;

        double current_average = 0;
        for (Community c : state_communities)
            current_average += c.get_compactness();

        if (loop == 4) {

            if (!override && first_average > current_average) {
                override = true;
                do {
                    worst_community = rand_num(0, state_communities.size() - 1);
                } while (worst_community == old_worst);
            }
            else
                override = false;

            first_average = current_average;
        }
            
        // if the community is within the tolerance, or if it has been modified too many times
        bool is_worst = false;
        for (int i = 0; i < state_communities.size(); i++) {
            if (state_communities[i].get_compactness() < compactness_tolerance) {
                is_worst = true;
                break;
            }
        }

        is_done = (!is_worst || iter == MAX_ITERATIONS);
        iter++;
    }


    for (Community c : this->state_communities)
        cout << c.get_compactness() << ", ";
    cout << endl;
}


void State::refine_partisan(double partisanship_tolerance) {
    /*
        @desc:
            optimize the partisanship of a community - attempts
            to minimize the stdev of partisanship of precincts
            within each community.
        @params: `double` partisanship_tolerance: tolerance for partisanship
        @return: void
    */

    p_index worst_community = get_next_community(partisanship_tolerance, PARTISANSHIP);
    bool is_done = (get_standard_deviation_partisanship(this->state_communities) < partisanship_tolerance);
    int iter = 0;

    cout << "refining for partisanship" << endl;

    while (!is_done) {
        p_index_set border_precincts = get_inner_boundary_precincts(state_communities[worst_community]);
        for (Community c : this->state_communities)
            cout << get_standard_deviation_partisanship(c) << ", ";
        cout << endl;

        for (p_index p : border_precincts) {
            if (!creates_island(state_communities[worst_community], p)) {
                Communities before = this->state_communities;
                give_precinct(p, worst_community, PARTISANSHIP, false);
                double after = get_standard_deviation_partisanship(this->state_communities);

                if (after >= get_standard_deviation_partisanship(before))
                    this->state_communities = before;
                else for (int i = 0; i < border_precincts.size(); i++) border_precincts[i] = border_precincts[i] - 1;
            }
        }

        // update worst_community, check stop condition
        worst_community = get_next_community(partisanship_tolerance, PARTISANSHIP);
        // if the community is within the tolerance, or if it has been modified too many times
        is_done = (iter == MAX_ITERATIONS || get_standard_deviation_partisanship(this->state_communities) < partisanship_tolerance);
        iter++;
    }

    for (Community c : this->state_communities)
        cout << get_standard_deviation_partisanship(c) << ", ";
    cout << endl;
}


void State::refine_population(double population_tolerance) {
    /* 
        @desc:
            optimize the state's communities for population. Attempts
            to minimize difference in population across the state
            with a tolerance for acceptable +- percent difference
        @params: `double` population_tolerance: tolerance for population
        @return: void
    */

    population_tolerance /= 2;  // so the +- value can be passed normally

    // find the first community to be optimized
    p_index worst_community = get_next_community(population_tolerance, POPULATION);
    bool is_done = (worst_community == -1); // initialize stop condition

    // calculate the range each community's population should be within
    int aim = get_population() / state_communities.size();
    vector<int> ideal_range = {aim - (int)(population_tolerance * aim), aim + (int)(population_tolerance * aim)};
    int iter = 0;

    cout << "refining for population" << endl;

    // begin main iterative loop
    while (!is_done) {
        Community c = state_communities[worst_community];
        p_index_set border_precincts = get_inner_boundary_precincts(state_communities[worst_community]);
        int index = 0;

        for (Community c : this->state_communities)
            cout << c.get_population() << ", ";
        cout << endl;

        while (index < border_precincts.size() && 
               (state_communities[worst_community].get_population() < ideal_range[0] 
              || state_communities[worst_community].get_population() > ideal_range[1])) {
            if (!creates_island(state_communities[worst_community], border_precincts[index])) {
                Precinct precinct = state_communities[worst_community].precincts[border_precincts[index]];
                if (give_precinct(border_precincts[index], worst_community, POPULATION, true))
                    for (int i = 0; i < border_precincts.size(); i++) border_precincts[i] = border_precincts[i] - 1;
            }

            index++;
        }

        // check stop condition, update the community that needs to be optimized
        worst_community = get_next_community(population_tolerance, POPULATION);
        is_done = (worst_community == -1 || iter == MAX_ITERATIONS);
        iter++;
    }

    for (Community c : this->state_communities)
        cout << c.get_population() << ", ";
    cout << endl;
}


int measure_difference(Communities communities, Communities new_communities) {
    
    /*
        @desc:
            measures and returns how many precincts have changed communities
            in a given list of old and new communities. Used for checking when
            to stop the algorithm.
        @params:
            `Communities` communities, new_communities: community arrays to measure difference of
        @return: `int` difference in communities
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


void State::generate_communities(int num_communities, double compactness_tolerance, double partisanship_tolerance, double population_tolerance, string writedir) {
    /*
        @desc:
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
        
        @params:
            `int` num_communities: number of districts to generate
            `double` compactness_tolerance,
                     partisanship_tolerance,
                     population_tolerance: tolerances for processes
    
        @return: void
    */

    generate_initial_communities(num_communities);

    /*
        Do 30 iterations, and see how many precincts change each iteration
        ! This is only until we have a good idea for a stop condition. We
           first will plot results on a graph and regress to see the optimal
           point of minimal change.
        At some point this will be changed to:
           while (changed_precincts > precinct_change_tolerance)
    */
   
    // Canvas canvas(900, 900);
    // canvas.add_shape(this->state_communities);
    // canvas.draw();

    this->refine_communities(partisanship_tolerance, population_tolerance, compactness_tolerance, writedir);
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


void State::refine_communities(double part, double popt, double compt, string writedir) {

    int i = 0;

    do {
        TOTAL_MOVED_PRECINCTS = 0;
        TOTAL_MOVED_PRECINCT_ID.clear();
        
        // @warn save data here
        refine_compactness(compt);
        refine_partisan(part);
        refine_population(popt);

        if (VERBOSE) cout << TOTAL_MOVED_PRECINCTS << " precincts changed" << endl;
        save_iteration_data(this->state_communities, writedir, i);

        i++;
    } while (TOTAL_MOVED_PRECINCTS > 20);
    
    Canvas canvas(200,200);
    canvas.add_shape(this->state_communities);
    canvas.draw();
    
    full_animation.playback();
    full_animation = GeoDraw::Anim(40);
}


void GeoGerry::State::save_iteration_data(Communities cs, string folder, int iteration) {
    string compactness = readf(folder + "/compactness.list"),
           population = readf(folder + "/population.list"),
           stdev = readf(folder + "/partisan.list"),
           moved_precincts = readf(folder + "/moved_precincts.list");

    moved_precincts += join(TOTAL_MOVED_PRECINCT_ID, ", ") + "\n";

    for (Community c : cs) {
        compactness += std::to_string(c.get_compactness()) + ", ";
        population += std::to_string(c.get_population()) + ", ";
        stdev += std::to_string(get_standard_deviation_partisanship(c)) + ", ";
    }

    stdev = stdev.substr(0, stdev.size() - 2) + "\n";
    population = population.substr(0, population.size() - 2) + "\n";
    compactness = compactness.substr(0, compactness.size() - 2) + "\n";

    writef(compactness, folder + "/compactness.list");
    writef(population, folder + "/population.list");
    writef(stdev, folder + "/partisan.list");
    writef(moved_precincts, folder + "/moved_precincts.list");
    this->save_communities(folder + "/shapes/" + "iteration_" + to_string(iteration), this->state_communities);
}
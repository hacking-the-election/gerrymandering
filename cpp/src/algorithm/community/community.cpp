/*=======================================
 community.cpp:                 k-vernooy
 last modified:                Sat, Feb 1

 Definition of the community-generation
 algorithm for quantifying gerrymandering
 and redistricting.
========================================*/

#include "../../../include/shape.hpp"  // class definitions
#include "../../../include/util.hpp"   // array modification functions

vector<Precinct_Group> State::generate_initial_communities() {
    vector<Precinct_Group> communities;
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

vector<Precinct_Group> State::generate_communities(int num_communities, float compactness_tolerance){
    cout << "go make some communities!" << endl;
    vector<Precinct_Group> communities = generate_initial_communities();
    
    int changed_precincts = 0,
        arbitrary_limit = 100,
        i = 0;

    while (changed_precincts > arbitrary_limit) {
        cout << "On iteration " << i << endl;

        vector<Precinct_Group> new_communities = 
            refine_compactness(refine_partisan(refine_population(communities)));
        
        changed_precincts = measure_difference(communities, new_communities);
        i++;
    }

    return communities;
}   
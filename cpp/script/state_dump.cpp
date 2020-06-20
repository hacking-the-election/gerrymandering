/*=======================================
 state_dump.cpp:                k-vernooy
 last modified:               Thu, Jun 18
 
 Print information about a state object
========================================*/

#include <iostream>

#include "../include/util.hpp"
#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/community.hpp"
#include "../include/quantification.hpp"

using namespace std;
using namespace hte::Geometry;
using namespace hte::Graphics;

int main(int argc, char* argv[]) {
    State state = State::from_binary(string(argv[1]));
    cout << "=============================================" << endl;
    cout << "summary for state " << argv[1] << endl;
    cout << state.precincts.size() << " precincts" << endl;
    cout << state.districts.size() << " districts" << endl;

    double precinct_sum = 0;
    double district_sum = 0;
    for (Precinct p : state.precincts) {
        precinct_sum += p.get_area();
    }
    for (MultiPolygon district : state.districts) {
        district_sum += district.get_area();
    }

    cout << abs(precinct_sum / pow(pow(2, 18), 2)) << " total precinct area" << endl;
    cout << abs(district_sum / pow(pow(2, 18), 2)) << " total district area" << endl;

    map<POLITICAL_PARTY, int> total_voter_data;
    int precincts_missing_data = 0;
    int total_popu = 0;
    for (auto& par : state.precincts[0].voter_data) {
        total_voter_data[par.first] = 0;
    }

    for (Precinct pre : state.precincts) {
        bool missing = false;
        for (auto& par : pre.voter_data) {
            if (par.second == -1) missing = true;
            total_voter_data[par.first] += par.second;
        }
        total_popu += pre.pop;
        if (pre.pop == -1) missing = true;
        if (missing) precincts_missing_data++;
    }

    cout << precincts_missing_data << " precincts missing vote data" << endl;

    for (auto& par : total_voter_data) {
        cout << par.second << " ";
        
        if (par.first == POLITICAL_PARTY::TOTAL) cout << "total votes";
        else if (par.first == POLITICAL_PARTY::DEMOCRAT) cout << "democrat votes";
        else if (par.first == POLITICAL_PARTY::REPUBLICAN) cout << "republican votes";
        else if (par.first == POLITICAL_PARTY::LIBERTARIAN) cout << "libertarian votes";
        else if (par.first == POLITICAL_PARTY::GREEN) cout << "green votes";
        else if (par.first == POLITICAL_PARTY::INDEPENDENT) cout << "independent votes";
        else if (par.first == POLITICAL_PARTY::REFORM) cout << "reform votes";
        else if (par.first == POLITICAL_PARTY::OTHER) cout << "other votes";
        cout << endl;
    }

    cout << state.network.vertices.size() << " nodes in state network" << endl;
    // cout << state.network.edges.size() << " edges in state network" << endl;
    cout << state.network.getNumComponents() << " components in state network" << endl;
    cout << "=============================================" << endl;
}

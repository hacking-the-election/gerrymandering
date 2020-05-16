/*=======================================
 serialize_state.cpp:           k-vernooy
 last modified:               Sun, Jan 19
 
 Given precinct and district geodata, and
 precinct voter data, writes to a binary 
 state file.

 - precinct geodata from github @nvkelso
 - election data from harvard dataverse
========================================*/

#include <iostream>
#include "../include/shape.hpp"
#include "../include/canvas.hpp"
#include "../include/util.hpp"

using namespace std;
using namespace hte::Geometry;


int main(int argc, char* argv[]) {
    /* 
        @desc: parse data into a binary state object

        @params:
           `string` precinct_geoJSON: path to precinct geodata
           `string` voter_data: (opt) path to precinct voter data
           `string` district_geoJSON: path to district geodata

        @return: `int` status
    */


    string KEY = "--keys=";  // prefix to find specified options

    if (argc < 5) {
        // did not provide infiles and keys
        cerr << "serialize_state: usage: " <<
            "<geodata> <election> <district> --keys=[keys] outfile" << endl;
        return 1;
    }

    vector<string> new_argv{};

    map<ID_TYPE, string> ids;
    map<POLITICAL_PARTY, string> voter_heads;


    for (int i = 0; i < argc; i++) {
        // loop through and find key set
        string arg = string(argv[i]);

        if (arg.substr(0, KEY.size()) == KEY) {
            bool done = false;
            while (!done) {
                string rem = arg.substr(arg.find('"'), arg.size());
                string key = rem.substr(0, rem.find(":"));
                key = string(key.begin() + 1, key.end() - 1);

                string search = ",";
                if (rem.find(search) == string::npos) {
                    search = "}";
                    done = true;
                }

                string val = rem.substr(rem.find(":") + 1, rem.find(search) - rem.find(":") - 1);
                val = string(val.begin() + 1, val.end() - 1);

                // i'm so sorry about this if statement
                if (key == "GEO") ids[ID_TYPE::GEOID] = val;
                else if (key == "ELE") ids[ID_TYPE::ELECTIONID] = val;
                else if (key == "POP") ids[ID_TYPE::POPUID] = val;
                else if (key == "DEM") voter_heads[POLITICAL_PARTY::DEMOCRAT] = val;
                else if (key == "REP") voter_heads[POLITICAL_PARTY::REPUBLICAN] = val;
                else if (key == "LIB") voter_heads[POLITICAL_PARTY::LIBERTARIAN] = val;
                else if (key == "REF") voter_heads[POLITICAL_PARTY::REFORM] = val;
                else if (key == "GRE") voter_heads[POLITICAL_PARTY::GREEN] = val;
                else if (key == "IND") voter_heads[POLITICAL_PARTY::INDEPENDENT] = val;                
                else if (key == "OTH") voter_heads[POLITICAL_PARTY::OTHER] = val;
                else if (key == "TOT") voter_heads[POLITICAL_PARTY::TOTAL] = val;

                if (!done) arg = rem.substr(rem.find(","), rem.size() - rem.find(","));
            }
        }
        else {
            // not a key arg
            new_argv.push_back(arg);
        }
    }

    argc--;
    State state;
    string write_path;

    // generate state from files
    if (argc == 5) {
        // read files into strings
        string precinct_geoJSON = readf(new_argv[1]);
        string voter_data = readf(new_argv[2]);
        string district_geoJSON = readf(new_argv[3]);
        write_path = string(new_argv[4]);
        state = State::generate_from_file(precinct_geoJSON, voter_data, district_geoJSON, voter_heads, ids);
    }
    else {
        // read files into strings
        string precinct_geoJSON = readf(new_argv[1]);
        string district_geoJSON = readf(new_argv[2]);
        write_path = string(new_argv[3]);
        state = State::generate_from_file(precinct_geoJSON, district_geoJSON, voter_heads, ids);
    }

    state.to_binary(write_path);  // write as binary
    
    cout << "precincts:\t" << state.precincts.size() << endl;
    cout << "districts:\t" << state.districts.size() << endl;

    map<POLITICAL_PARTY, int> total_voter_data;
    int total_popu = 0;
    for (auto& par : state.precincts[0].voter_data) {
        total_voter_data[par.first] = 0;
    }

    for (Precinct pre : state.precincts) {
        for (auto& par : pre.voter_data) {
            total_voter_data[par.first] += par.second;
        }

        if (pre.shape_id == "560051901_s1") {
            for (auto& par : pre.voter_data) {
                cout << par.second << endl;
            }
        }

        total_popu += pre.pop;
    }

    for (auto& par : total_voter_data) {
        if (par.first == POLITICAL_PARTY::DEMOCRAT) cout << "dem:\t";
        else if (par.first == POLITICAL_PARTY::REPUBLICAN) cout << "rep:\t";
        else if (par.first == POLITICAL_PARTY::TOTAL) cout << "total:\t";
        else if (par.first == POLITICAL_PARTY::LIBERTARIAN) cout << "lib:\t";
        else if (par.first == POLITICAL_PARTY::GREEN) cout << "green:\t";
        else if (par.first == POLITICAL_PARTY::INDEPENDENT) cout << "ind:\t";
        else if (par.first == POLITICAL_PARTY::REFORM) cout << "reform:\t";
        else if (par.first == POLITICAL_PARTY::OTHER) cout << "other:\t";

        cout << par.second << endl;
    }

    cout << "population:\t" << total_popu << endl;
    cout << "state written to " << write_path << endl;

    return 0;
}

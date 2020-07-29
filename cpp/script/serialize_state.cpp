/*=======================================
 serialize_state.cpp:           k-vernooy
 last modified:               Sun, Jun 21
 
 Given precinct and district geodata, and
 precinct voter data, writes to a binary 
 state file.

 - precinct geodata from github @nvkelso
 - election data from harvard dataverse
========================================*/

#include <iostream>
#include "../include/hte.h"

using namespace std;
using namespace hte;


int main(int argc, char* argv[]) {
    string KEY = "--keys=";  // prefix to find specified options

    if (argc < 5) {
        // did not provide infiles and keys
        cerr << "serialize_state: usage: " <<
            "<geodata> <election> <district> --keys=[keys] outfile" << endl;
        return 1;
    }

    vector<string> new_argv{};

    map<IdType, string> ids;
    map<PoliticalParty, string> voter_heads;


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

                if (key == "GEO") ids[IdType::GEOID] = val;
                else if (key == "ELE") ids[IdType::ELECTIONID] = val;
                else if (key == "POP") ids[IdType::POPUID] = val;
                else if (key == "DEM") voter_heads[PoliticalParty::Democrat] = val;
                else if (key == "REP") voter_heads[PoliticalParty::Republican] = val;
                else if (key == "LIB") voter_heads[PoliticalParty::Libertarian] = val;
                else if (key == "REF") voter_heads[PoliticalParty::Reform] = val;
                else if (key == "GRE") voter_heads[PoliticalParty::Green] = val;
                else if (key == "IND") voter_heads[PoliticalParty::Independent] = val;                
                else if (key == "OTH") voter_heads[PoliticalParty::Other] = val;
                else if (key == "TOT") voter_heads[PoliticalParty::Total] = val;
                else cout << "unrecognized key " << key << endl;

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
        string precinct_geoJSON = ReadFile(new_argv[1]);
        string voter_data = ReadFile(new_argv[2]);
        string district_geoJSON = ReadFile(new_argv[3]);
        write_path = string(new_argv[4]);
        state = State::GenerateFromFile(precinct_geoJSON, voter_data, district_geoJSON, voter_heads, ids);
    }
    else {
        // read files into strings
        string precinct_geoJSON = ReadFile(new_argv[1]);
        string district_geoJSON = ReadFile(new_argv[2]);
        write_path = string(new_argv[3]);
        state = State::GenerateFromFile(precinct_geoJSON, district_geoJSON, voter_heads, ids);
    }

    state.toFile(write_path);
    
    cout << "precincts:\t" << state.precincts.size() << endl;
    cout << "districts:\t" << state.districts.size() << endl;

    map<PoliticalParty, int> total_voter_data;
    int total_popu = 0;
    for (auto& par : state.precincts[0].voterData) {
        total_voter_data[par.first] = 0;
    }

    for (Precinct pre : state.precincts) {
        for (auto& par : pre.voterData) {
            total_voter_data[par.first] += par.second;
        }

        total_popu += pre.pop;
    }

    for (auto& par : total_voter_data) {
        if (par.first == PoliticalParty::Democrat) cout << "dem:\t";
        else if (par.first == PoliticalParty::Republican) cout << "rep:\t";
        else if (par.first == PoliticalParty::Total) cout << "total:\t";
        else if (par.first == PoliticalParty::Libertarian) cout << "lib:\t";
        else if (par.first == PoliticalParty::Green) cout << "green:\t";
        else if (par.first == PoliticalParty::Independent) cout << "ind:\t";
        else if (par.first == PoliticalParty::Reform) cout << "reform:\t";
        else if (par.first == PoliticalParty::Other) cout << "other:\t";

        cout << par.second << endl;
    }

    cout << "population:\t" << total_popu << endl;
    cout << "state written to " << write_path << endl;

    return 0;
}

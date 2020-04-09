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
#include "../include/term_disp.hpp"

using namespace std;
using namespace GeoGerry;


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

    if (argc < 4) {
        // did not provide infiles and keys
        cerr << "serialize_state: usage: " <<
            "<geodata> <election> <district> --keys=[keys] outfile" << endl;
        return 1;
    }

    vector<vector<string> > opts = {};
    vector<string> new_argv{};

    for (int i = 0; i < argc; i++) {
        // loop through and find key set
        string arg = string(argv[i]);

        if (arg.substr(0, KEY.size()) == KEY) {
            // need to parse key set
            arg = arg.substr(KEY.size(), arg.size() - 1);
            int argsize;

            if (argc == 6) argsize = 5;
            else argsize = 3;

            for (int i = 0; i < argsize; i++) {
                string csv;
                vector<string> vals;
                int end = -1;

                if (arg.at(0) == '{') {
                    // need to parse multiply vals
                    csv = arg.substr(1, arg.find('}') - 1);
                    end = arg.find('}') + 2;
                }
                else {
                    // single val list
                    csv = arg.substr(0, arg.find(','));
                    end = arg.find(',') + 1;
                }

                size_t pos = 0;
                while ((pos = csv.find(",")) != std::string::npos) {
                    vals.push_back(csv.substr(0, pos));
                    csv.erase(0, pos + 1);
                }
                vals.push_back(csv);

                opts.push_back(vals);
                arg = arg.substr(end, arg.size() - 1);
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
        state = State::generate_from_file(precinct_geoJSON, voter_data, district_geoJSON, opts);
    }
    else {
        // read files into strings
        string precinct_geoJSON = readf(new_argv[1]);
        string district_geoJSON = readf(new_argv[2]);
        write_path = string(new_argv[3]);
        state = State::generate_from_file(precinct_geoJSON, district_geoJSON, opts);
    }

    state.write_binary(write_path);  // write as binary
    
    cout << "precincts:\t" << state.precincts.size() << endl;
    cout << "districts:\t" << state.districts.size() << endl;

    int dem = 0, rep = 0, pop = 0;

    for (Precinct pre : state.precincts) {
        if (pre.dem != -1) dem += pre.dem;
        if (pre.dem != -1) rep += pre.rep;
        if (pre.dem != -1) pop += pre.pop;
    }

    cout << "population:\t" << pop << endl;
    cout << "democrat:\t" << dem << endl;
    cout << "republican:\t" << rep << endl << endl;
    cout << "state written to " << write_path << endl;

    return 0;
}
/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                Sat, Feb 8
 
 Run community generation algorithm and 
 print coordinates as geojson for a given
 state object
========================================*/

#include <boost/filesystem.hpp>
#include <iostream>
#include <chrono>

#include "../include/util.hpp"
#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/community.hpp"
#include "../include/quantification.hpp"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace std;
using namespace hte::Geometry;
using namespace hte::Graphics;

#define MAX_TIME 40000


int main(int argc, char* argv[]) {
    
    /*
        A driver program for the community algorithm
        See community.cpp for the implementation of the algorithm
    */

    if (argc != 3) {
        cerr << "generate_communities: usage: <state.dat> <output_dir>" << endl;
        return 1;
    }

    srand(time(NULL));
    cout << "performing communities algorithm on " << string(argv[1]) << endl;

    // read binary file from path
    Canvas canvas(800, 800);
    string read_path = string(argv[1]);
    string output_dir = string(argv[2]);
    if (output_dir.substr(output_dir.size() - 1, 1) == "/") {
        output_dir = output_dir.substr(0, output_dir.size() - 1);
    }

    create_directory(path(output_dir));
    create_directory(path(output_dir + "/anim"));
    create_directory(path(output_dir + "/anim/redistricts"));
    create_directory(path(output_dir + "/anim/communities"));
    create_directory(path(output_dir + "/img"));
    create_directory(path(output_dir + "/stats"));
    create_directory(path(output_dir + "/stats/districts"));
    create_directory(path(output_dir + "/stats/redistricts"));
    create_directory(path(output_dir + "/stats/communities"));

    State state = State::from_binary(read_path);
    int n_communities = state.districts.size();
    
    // get initial random configuration
    Communities init_config = karger_stein(state.network, n_communities);
    cout << "got initial random configuration, saving to file..." << endl;
    save(init_config, output_dir + "/init_config_shape.txt");

    cout << "optimizing for political communities..." << endl;
    Communities communities = get_communities(state.network, init_config, 0.1, output_dir, true);
    save(communities, output_dir + "/communities_shape.txt");
    canvas.add_outlines(to_outline(communities));
    canvas.save_image(ImageFmt::BMP, output_dir + "/img/communities");

    cout << "optimizing for better districts..." << endl;
    // get districts, save to district output
    Communities districts = get_communities(state.network, communities, 0.01, output_dir, false);
    save(districts, output_dir + "/redistricts_shape.txt");
    canvas.clear();
    canvas.add_outlines(to_outline(districts));
    canvas.save_image(ImageFmt::BMP, output_dir + "/img/redistricts");

    cout << "quantifying current districts..." << endl;
    for (int i = 0; i < state.districts.size(); i++) {
        map<POLITICAL_PARTY, double> quantification = get_quantification(state.network, communities, state.districts[i]);
        double partisanship = 0.5;
        if (quantification[POLITICAL_PARTY::DEMOCRAT] + quantification[POLITICAL_PARTY::REPUBLICAN] != 0)
            partisanship = quantification[POLITICAL_PARTY::REPUBLICAN] / (quantification[POLITICAL_PARTY::DEMOCRAT] + quantification[POLITICAL_PARTY::REPUBLICAN]);
        double collapsed = collapse_vals(quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION], partisanship);

        // save quantification values to file

        // create visualizations of the output
        canvas.clear();
        canvas.add_outlines(to_outline(state.districts[i], quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION], true));
        canvas.save_image(ImageFmt::BMP, output_dir + "/img/current_abs_quant");
        canvas.clear();
        canvas.add_outlines(to_outline(state.districts[i], collapsed, false));
        canvas.save_image(ImageFmt::BMP, output_dir + "/img/current_0_1");
    }

    cout << "quantifying redistricted districts..." << endl;
    return 0;
}

/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                Thu, Jun 18
 
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
    Canvas canvas(900, 900);
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
    // Communities init_config = load("../output/louisiana/init_config_shape.txt", state.network);
    Communities init_config = karger_stein(state.network, n_communities);
    for (int i = 0; i < init_config.size(); i++) {
        init_config[i].update_shape(state.network);
    }
    save(init_config, output_dir + "/init_config_shape.txt");

    cout << "optimizing for political communities..." << endl;
    Communities communities = get_communities(state.network, init_config, 0.1, output_dir, true);
    save(communities, output_dir + "/communities_shape.txt");
    canvas.add_outlines(to_outline(communities));
    canvas.save_image(ImageFmt::BMP, output_dir + "/img/communities");

    cout << "quantifying current districts..." << endl;
    string abs_line;
    string coll_line;
    string part_line;

    Canvas absolute_q(900, 900);
    Canvas collapsed_q(900, 900);

    for (int i = 0; i < state.districts.size(); i++) {
        map<POLITICAL_PARTY, double> quantification = get_quantification(state.network, communities, state.districts[i]);
        double partisanship = quantification[POLITICAL_PARTY::REPUBLICAN];
        //if (quantification[POLITICAL_PARTY::DEMOCRAT] + quantification[POLITICAL_PARTY::REPUBLICAN] != 0)
            //partisanship = quantification[POLITICAL_PARTY::REPUBLICAN] / (quantification[POLITICAL_PARTY::DEMOCRAT] + quantification[POLITICAL_PARTY::REPUBLICAN]);
        double collapsed = collapse_vals(quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION], partisanship);

        // save quantification values to file
        part_line += "[";
        for (auto& pair : quantification) {
            if (pair.first == POLITICAL_PARTY::DEMOCRAT) part_line += "\"DEM\":";
            else if (pair.first == POLITICAL_PARTY::REPUBLICAN) part_line += "\"REP\":";
            else if (pair.first == POLITICAL_PARTY::GREEN) part_line += "\"GRE\":";
            else if (pair.first == POLITICAL_PARTY::INDEPENDENT) part_line += "\"IND\":";
            else if (pair.first == POLITICAL_PARTY::LIBERTARIAN) part_line += "\"LIB\":";
            else if (pair.first == POLITICAL_PARTY::REFORM) part_line += "\"REF\":";
            else if (pair.first == POLITICAL_PARTY::OTHER) part_line += "\"OTH\":";
            else break;
            part_line += to_string(pair.second);
            part_line += ",";
        }
    
        part_line.pop_back();
        part_line += "]";

        abs_line += to_string(quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION]);
        coll_line += to_string(collapsed);
        
        if (i != state.districts.size() - 1) {
            part_line += "\t";
            coll_line += "\t";
            abs_line += "\t"; 
        }

        // save quantification values to file
        // create visualizations of the output
        absolute_q.add_outline_group(to_outline(state.districts[i], quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION], true));
        collapsed_q.add_outline_group(to_outline(state.districts[i], collapsed, false));
    }

    absolute_q.save_image(ImageFmt::BMP, output_dir + "/img/current_abs_quant");
    collapsed_q.save_image(ImageFmt::BMP, output_dir + "/img/current_0_1");
    writef(abs_line + "\n" + coll_line + "\n" + part_line + "\n", output_dir + "/stats/districts/quantification.tsv");

    // get districts, save to district output
    cout << "optimizing for better districts..." << endl;
    Communities redistricts = get_communities(state.network, communities, 0.01, output_dir, false);
    save(redistricts, output_dir + "/redistricts_shape.txt");
    canvas.clear();
    canvas.add_outlines(to_outline(redistricts));
    canvas.save_image(ImageFmt::BMP, output_dir + "/img/redistricts");

    cout << "quantifying redistricted districts..." << endl;
    absolute_q.clear();
    collapsed_q.clear();
    abs_line.clear();
    coll_line.clear();
    part_line.clear();

    for (int i = 0; i < redistricts.size(); i++) {
        MultiPolygon ext = generate_exterior_border(redistricts[i].shape);
        map<POLITICAL_PARTY, double> quantification = get_quantification(state.network, communities, ext);
        double partisanship = 0.5;
        if (quantification[POLITICAL_PARTY::DEMOCRAT] + quantification[POLITICAL_PARTY::REPUBLICAN] != 0)
            partisanship = quantification[POLITICAL_PARTY::REPUBLICAN] / (quantification[POLITICAL_PARTY::DEMOCRAT] + quantification[POLITICAL_PARTY::REPUBLICAN]);
        double collapsed = collapse_vals(quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION], partisanship);

        // save quantification values to file
        part_line += "[";
        for (auto& pair : quantification) {
            if (pair.first == POLITICAL_PARTY::DEMOCRAT) part_line += "\"DEM\":";
            else if (pair.first == POLITICAL_PARTY::REPUBLICAN) part_line += "\"REP\":";
            else if (pair.first == POLITICAL_PARTY::GREEN) part_line += "\"GRE\":";
            else if (pair.first == POLITICAL_PARTY::INDEPENDENT) part_line += "\"IND\":";
            else if (pair.first == POLITICAL_PARTY::LIBERTARIAN) part_line += "\"LIB\":";
            else if (pair.first == POLITICAL_PARTY::REFORM) part_line += "\"REF\":";
            else if (pair.first == POLITICAL_PARTY::OTHER) part_line += "\"OTH\":";
            else break;
            part_line += to_string(pair.second);
            part_line += ",";
        }

        part_line.pop_back();
        part_line += "]";

        abs_line += to_string(quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION]);
        coll_line += to_string(collapsed);

        if (i != state.districts.size() - 1) {
            part_line += "\t";
            coll_line += "\t";
            abs_line += "\t"; 
        }

        // create visualizations of the output
        absolute_q.add_outline_group(to_outline(ext, quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION], true));
        collapsed_q.add_outline_group(to_outline(ext, collapsed, false));
    }

    writef(abs_line + "\n" + coll_line + "\n" + part_line + "\n", output_dir + "/stats/redistricts/quantification.tsv");
    absolute_q.save_image(ImageFmt::BMP, output_dir + "/img/redistricts_abs_quant");
    collapsed_q.save_image(ImageFmt::BMP, output_dir + "/img/redistricts_0_1");

    return 0;
}

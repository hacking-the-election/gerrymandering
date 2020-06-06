/*=======================================
 sandbox.cpp:                   k-vernooy
 last modified:               Sun May 31
 
 A simple testing environment with the
 hacking-the-election library
========================================*/

#include <chrono>
#include <iostream>
#include <boost/filesystem.hpp>

#include "../include/util.hpp"
#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/community.hpp"
#include "../include/quantification.hpp"

// for the rapidjson parser
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

using namespace boost::filesystem;
using namespace std;
using namespace hte::Geometry;
using namespace hte::Graphics;
using namespace rapidjson;


int main(int argc, char* argv[]) {
    
    /*
        A loaded environment with included namespaces ready for 
        graphics programs testing, compiling random binaries, and other scripts
    */

    srand(time(NULL));

    State state = State::from_binary(argv[1]);
    Communities cs = load(argv[2], state.network);
    Communities redistricts = load(argv[3], state.network);
    string output_dir = string(argv[4]);

    Canvas absolute_q(900, 900);
    Canvas collapsed_q(900, 900);
    // quantify current districts, save quantification.tsv and images
    string part_line = "", abs_line = "", coll_line = "";

    for (int i = 0; i < state.districts.size(); i++) {
        map<POLITICAL_PARTY, double> quantification = get_quantification(state.network, cs, state.districts[i]);
        double partisanship = quantification[POLITICAL_PARTY::REPUBLICAN];
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
    collapsed_q.save_image(ImageFmt::BMP, output_dir + "/img/current_part_quant");
    writef(abs_line + "\n" + coll_line + "\n" + part_line + "\n", output_dir + "/stats/districts/quantification.tsv");

    absolute_q.clear();
    collapsed_q.clear();
    part_line.clear();
    abs_line.clear();
    coll_line.clear();

    
    for (int i = 0; i < redistricts.size(); i++) {
        Multi_Polygon ext = generate_exterior_border(redistricts[i].shape);
        map<POLITICAL_PARTY, double> quantification = get_quantification(state.network, cs, ext);
        double partisanship = quantification[POLITICAL_PARTY::REPUBLICAN];
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
        absolute_q.add_outline_group(to_outline(ext, quantification[POLITICAL_PARTY::ABSOLUTE_QUANTIFICATION], true));
        collapsed_q.add_outline_group(to_outline(ext, collapsed, false));
    }

    absolute_q.save_image(ImageFmt::BMP, output_dir + "/img/redistrict_abs_quant");
    collapsed_q.save_image(ImageFmt::BMP, output_dir + "/img/redistrict_part_quant");
    writef(abs_line + "\n" + coll_line + "\n" + part_line + "\n", output_dir + "/stats/redistricts/quantification.tsv");

    return 0;
}

/*=======================================
 json.cpp:                      k-vernooy
 last modified:               Sun, Jan 19
 
 Writing state, precinct, and other
 objects to a json file for use by other
 researches. See specification for guide.
========================================*/

#include "../include/shape.hpp"
#include "../include/term_disp.hpp"

string State::to_json() {
    /*
        dumps a state object as json
        just uses string manipulation, no json parsing
    */
   
    // begin json string
    string str = "{" + N; 

    str += T + OQ + "state" + CQC + "{" + N
        + TAB(2) + OQ + "name" + CQC + OQ + name + CQ + C + N
        + TAB(2) + OQ + "precincts" + CQC + "[" + N;
    
    for ( Precinct precinct : state_precincts ) {
        str += TAB(3) + "{" + N;

        // print name of precinct
        if (precinct.shape_id.size() == 0) {
            str += TAB(4) + OQ + "name" + CQC 
                + OQ + "no_name" + CQ + C + N;
        }
        else {
            str += TAB(4) + OQ + "name" + CQC 
                + OQ + precinct.shape_id + CQ + C + N;
        }

        // print coordinates of precinct
        str += TAB(4) + OQ + "coordinates" + CQC + "[";
        for (vector<float> coordset: precinct.border) {
            str += "[" + to_string(coordset[0]) + ", " 
                + to_string(coordset[1]) + "], ";
        }

        // remove last comma char
        str = str.substr(0, str.size() - 2);
        str += "]" + C + N;

        // print voter data for the precinct
        str += TAB(4) + OQ + "voter_data" + CQC + "{"
            + OQ + "dem" + CQC + to_string(precinct.voter_data()[0]) + ", "
            + OQ + "rep" + CQC + to_string(precinct.voter_data()[1]) + "}" + N;
        
        str += TAB(3) + "}," + N;
    }

    str = str.substr(0, str.size() - 2) + N;
    str += TAB(2) + "]" + C + N; // close precincts array


    str += TAB(2) + OQ + "districts" + CQC + "[" + N;

    for ( District district : state_districts ) {
        str += TAB(3) + "{" + N;

        // print name of precinct
        if (district.shape_id.size() == 0) {
            str += TAB(4) + OQ + "name" + CQC 
                + OQ + "no_name" + CQ + C + N;
        }
        else {
            str += TAB(4) + OQ + "name" + CQC 
                + OQ + district.shape_id + CQ + C + N;
        }

        // print coordinates of precinct
        str += TAB(4) + OQ + "coordinates" + CQC + "[";
        for (vector<float> coordset : district.border) {
            str += "[" + to_string(coordset[0]) + ", " 
                + to_string(coordset[1]) + "], ";
        }

        // remove last comma char
        str = str.substr(0, str.size() - 2);
        str += "]" + N;

        str += TAB(3) + "}," + N;
    }

    str = str.substr(0, str.size() - 2) + N;
    str += TAB(2) + "]" + N; // close districts array

    str += T + "}" + N; // close state
    str += "}" + N; // close json
    return str;
}
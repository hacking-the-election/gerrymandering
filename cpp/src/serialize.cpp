/*=======================================
 serialize.cpp:                 k-vernooy
 last modified:               Sun, Jan 19
 
 Writing state, precinct, and other
 objects to a json file for use by other
 researches. See specification for guide.
========================================*/

#include "../include/shape.hpp"
#include "../include/term_disp.hpp"
#include <boost/filesystem.hpp>

string State::to_json() {
    /*
        dumps a state object as json
        just uses string manipulation, no json parsing
    */
   
    // begin json string
    string str = "{" + N; 

    // str += T + OQ + "state" + CQC + "{" + N
    //     + TAB(2) + OQ + "name" + CQC + OQ + name + CQ + C + N
    //     + TAB(2) + OQ + "precincts" + CQC + "[" + N;
    
    // for ( Precinct precinct : state_precincts ) {
    //     str += TAB(3) + "{" + N;

    //     // print name of precinct
    //     if (precinct.shape_id.size() == 0) {
    //         str += TAB(4) + OQ + "name" + CQC 
    //             + OQ + "no_name" + CQ + C + N;
    //     }
    //     else {
    //         str += TAB(4) + OQ + "name" + CQC 
    //             + OQ + precinct.shape_id + CQ + C + N;
    //     }

    //     // print coordinates of precinct
    //     str += TAB(4) + OQ + "coordinates" + CQC + "[";
    //     for (vector<float> coordset: precinct.border) {
    //         str += "[" + to_string(coordset[0]) + ", " 
    //             + to_string(coordset[1]) + "], ";
    //     }

    //     // remove last comma char
    //     str = str.substr(0, str.size() - 2);
    //     str += "]" + C + N;

    //     // print voter data for the precinct
    //     str += TAB(4) + OQ + "voter_data" + CQC + "{"
    //         + OQ + "dem" + CQC + to_string(precinct.voter_data()[0]) + ", "
    //         + OQ + "rep" + CQC + to_string(precinct.voter_data()[1]) + "}" + N;
        
    //     str += TAB(3) + "}," + N;
    // }

    // str = str.substr(0, str.size() - 2) + N;
    // str += TAB(2) + "]" + C + N; // close precincts array


    // str += TAB(2) + OQ + "districts" + CQC + "[" + N;

    // for ( Precinct_Group district : state_districts ) {
    //     str += TAB(3) + "{" + N;

    //     // print name of precinct
    //     if (district.shape_id.size() == 0) {
    //         str += TAB(4) + OQ + "name" + CQC 
    //             + OQ + "no_name" + CQ + C + N;
    //     }
    //     else {
    //         str += TAB(4) + OQ + "name" + CQC 
    //             + OQ + district.shape_id + CQ + C + N;
    //     }

    //     // print coordinates of precinct
    //     str += TAB(4) + OQ + "coordinates" + CQC + "[";
    //     for (coordinate coordset : district.border) {
    //         str += "[" + to_string(coordset[0]) + ", " 
    //             + to_string(coordset[1]) + "], ";
    //     }

    //     // remove last comma char
    //     str = str.substr(0, str.size() - 2);
    //     str += "]" + N;

    //     str += TAB(3) + "}," + N;
    // }

    // str = str.substr(0, str.size() - 2) + N;
    // str += TAB(2) + "]" + N; // close districts array

    // str += T + "}" + N; // close state
    // str += "}" + N; // close json
    return str;
}


/*
    Write and read state file to and from binary

    Any instance of & operator is overloaded for 
    reading and writing, to avoid duplicate methods
    (see boost documentation for more information)
*/

void State::write_binary(string path) {
    ofstream ofs(path); // open output stream
    boost::archive::binary_oarchive oa(ofs); // open archive stream
    oa << *this; // put this pointer into stream
    ofs.close(); // close stream
}

State State::read_binary(string path) {
    State state = State(); // blank state object

    ifstream ifs(path); // open input stream
    boost::archive::binary_iarchive ia(ifs); // open archive stream
    ia >> state; // read into state object

    return state; // return state object
}

void Precinct_Group::write_binary(string path) {
    ofstream ofs(path); // open output stream
    boost::archive::binary_oarchive oa(ofs); // open archive stream
    oa << *this; // put this pointer into stream
    ofs.close(); // close stream
}

Precinct_Group Precinct_Group::read_binary(string path) {
    Precinct_Group pg = Precinct_Group(); // blank object
    ifstream ifs(path); // open input stream
    boost::archive::binary_iarchive ia(ifs); // open archive stream
    ia >> pg;
    return pg; // return precinct group object
}

template<class Archive> void State::serialize(Archive & ar, const unsigned int version) {
    // write districts, precincts, name, and border
    ar & state_districts;
    ar & state_precincts;
    ar & name;
    ar & border;
    ar & pop;
}

template<class Archive> void Multi_Shape::serialize(Archive & ar, const unsigned int version) {
    ar & border;
    ar & shape_id;
    ar & pop;
}

/*
    The following are mirror serialization methods 
    to make sure that each sub-nested shape is actually
    written to the binary file

    ! this could probably be better structured
    TODO: Make sure that this actually is necessary
*/

template<class Archive> void Precinct_Group::serialize(Archive & ar, const unsigned int version) {
    // push id and border into the archive stream
    ar & id;
    ar & border;
    ar & pop;
    // ar & precincts;
}

 template<class Archive> void Precinct::serialize(Archive & ar, const unsigned int version) {
    // push shape, border and vote data
    ar & shape_id;
    ar & border;
    ar & dem;            
    ar & rep;
    ar & pop;
}

template<class Archive> void Shape::serialize(Archive & ar, const unsigned int version) {
    // push shape id and border to archive streamm
    ar & shape_id;
    ar & border;
    ar & pop;
}


void State::save_communities(string write_path) {
    /*
        Saves a community to a file at a specific point in the
        pipeline. Useful for visualization and checks.

        Save structure is as follows:
            write_path/
                community_1
                ...
                community_n
    */

   int c_index = 0;
   boost::filesystem::create_directory(write_path);

   for (Community c : state_communities) {
       c.write_binary(write_path + "/community_" + to_string(c_index));
       c_index++;
   }
}
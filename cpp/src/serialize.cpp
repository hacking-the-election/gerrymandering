/*=======================================
 serialize.cpp:                 k-vernooy
 last modified:               Sun, Jan 19
 
 Writing state, precinct, and other
 objects to a json file for use by other
 researches. See specification for guide.
========================================*/

#include <iterator>

#include "../include/shape.hpp"
#include "../include/term_disp.hpp"
#include "../include/canvas.hpp"
#include "../include/geometry.hpp"
#include "../include/util.hpp"

#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

std::string geojson_header = "{\"type\": \"FeatureCollection\", \"features\":[";

std::string GeoGerry::LinearRing::to_json() {
    /*
        @desc: converts a linear ring into a json array of coords
        @params: none
        @return: `string` json array
    */

    std::string str = "[";
    for (GeoGerry::coordinate c : border)
        str += "[" + std::to_string(c[0]) + ", " + std::to_string(c[1]) + "],";

    str = str.substr(0, str.size() - 1);
    str += "]";

    return str;
}

std::string GeoGerry::Precinct_Group::to_json() {
    /*
        @desc:
            converts a Precinct_Group object into a geojson document
            for viewing in mapshaper or elsewhere

        @params: none
        @return: `string` json array
    */

    std::string str = geojson_header;
    
    for (GeoGerry::Precinct p : precincts) {
        str += "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Polygon\",\"coordinates\":[";
        str += p.hull.to_json();
        for (GeoGerry::LinearRing hole : p.holes)
            str += "," + hole.to_json();
        str += "]}},";
    }

    str = str.substr(0, str.size() - 1); // remove comma
    str += "]}";
    return str;
}

std::string GeoGerry::Shape::to_json() {
    /*
        @desc: Converts a normal shape object into geojson
        @params: none
        @return: `string` geojson object
    */

    std::string str = geojson_header;
    str += hull.to_json();
    str += "]}}";

    return str;
}

std::string GeoGerry::Multi_Shape::to_json() {
    /*
        @desc: Converts a multiple shape object into geojson
        @params: none
        @return: `string` geojson object
    */

    std::string str = geojson_header + "{\"type\":\"Feature\",\"geometry\":{\"type\":\"MultiPolygon\",\"coordinates\":[";
    for (Shape s : border) {
        str += "[";
        str += s.hull.to_json();
        for (LinearRing h : s.holes) {
            str += h.to_json();
        }
        str += "],";
    }
    
    str = str.substr(0, str.size() - 1);
    str += "]}}";

    return str;
}

// std::string GeoGerry::State::to_json() {
    // /*
    //     dumps a state object as json
    //     just uses string manipulation, no json parsing
    // */
   
    // begin json string
    // std::string str = "{" + N; 

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
//     return str;
// }


/*
    Write and read state file to and from binary

    Any instance of & operator is overloaded for 
    reading and writing, to avoid duplicate methods
    (see boost documentation for more information)
*/

void GeoGerry::State::write_binary(std::string path) {
    std::ofstream ofs(path); // open output stream
    boost::archive::binary_oarchive oa(ofs); // open archive stream
    oa << *this; // put this pointer into stream
    ofs.close(); // close stream
}

GeoGerry::State GeoGerry::State::read_binary(std::string path) {
    GeoGerry::State state = GeoGerry::State(); // blank state object

    std::ifstream ifs(path); // open input stream
    boost::archive::binary_iarchive ia(ifs); // open archive stream
    ia >> state; // read into state object

    return state; // return state object
}

void GeoGerry::Precinct_Group::write_binary(std::string path) {
    std::ofstream ofs(path); // open output stream
    boost::archive::binary_oarchive oa(ofs); // open archive stream
    oa << *this; // put this pointer into stream
    ofs.close(); // close stream
}

GeoGerry::Precinct_Group GeoGerry::Precinct_Group::read_binary(std::string path) {
    GeoGerry::Precinct_Group pg = GeoGerry::Precinct_Group(); // blank object
    std::ifstream ifs(path); // open input stream
    boost::archive::binary_iarchive ia(ifs); // open archive stream
    ia >> pg;
    return pg; // return precinct group object
}


template<class Archive> void GeoGerry::LinearRing::serialize(Archive & ar, const unsigned int version) {
    ar & border;
}

template<class Archive> void GeoGerry::State::serialize(Archive & ar, const unsigned int version) {
    // write districts, precincts, name, and border
    ar & state_districts;
    ar & precincts;
    ar & islands;
    ar & name;
    ar & border;
    ar & pop;
}

template<class Archive> void GeoGerry::Multi_Shape::serialize(Archive & ar, const unsigned int version) {
    ar & border;
    ar & shape_id;
    ar & pop;
}

/*
    The following are serialization methods written
    to make sure that each sub-nested shape is actually
    written to the binary file

    ! this could probably be better structured
    TODO: Make sure that this actually is necessary
*/

template<class Archive> void GeoGerry::Precinct_Group::serialize(Archive & ar, const unsigned int version) {
    // push id and border into the archive stream
    ar & id;
    ar & border;
    ar & pop;
    // ar & precincts;
}

 template<class Archive> void GeoGerry::Precinct::serialize(Archive & ar, const unsigned int version) {
    // push shape, border and vote data
    ar & shape_id;
    ar & hull;
    ar & holes;
    ar & dem;            
    ar & rep;
    ar & pop;
}

template<class Archive> void GeoGerry::Shape::serialize(Archive & ar, const unsigned int version) {
    // push shape id and border to archive streamm
    ar & shape_id;
    ar & hull;
    ar & holes;
    ar & pop;
}


void GeoGerry::State::save_communities(std::string write_path, Communities communities) {
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
    std::string file = "";

    for (Community c : communities) {
        file += c.save_frame() + "\n";
        c_index++;
    }

    writef(file, write_path);
}

void GeoGerry::State::read_communities(std::string read_path) {
    this->state_communities = Community::load_frame(read_path, *this);
    for (int i = 0; i < state_communities.size(); i++)
        state_communities[i].border = generate_exterior_border(state_communities[i]).border;

    return;
}

void GeoGerry::State::playback_communities(std::string read_path) {
    GeoDraw::Anim animation(150);

    fs::path p(read_path);
    std::vector<fs::directory_entry> v;

    if (fs::is_directory(p)) {
        std::copy(fs::directory_iterator(p), fs::directory_iterator(), std::back_inserter(v));
        for (std::vector<fs::directory_entry>::const_iterator it = v.begin(); it != v.end();  ++ it ){
            GeoDraw::Canvas canvas(900, 900);
            Communities cs = Community::load_frame((*it).path().string(), *this);
            for (Community c : cs)
                canvas.add_shape(generate_exterior_border(c));
            animation.frames.push_back(canvas);
        }
    }

    animation.playback();
    
    return;
}
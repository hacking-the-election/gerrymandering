/*=======================================
 shape.cpp:                     k-vernooy
 last modified:                Thu Mar 12
 
 Definition of generic methods for shapes, 
 precincts, districts and states that do
 not include geometric and/or mathematical
 algorithms (see geometry.cpp).
========================================*/

#include "../include/shape.hpp"     // class definitions
#include "../include/geometry.hpp"  // class definitions
#include "../include/util.hpp"      // split and join

#define REP  // use rep / (dem + rep) as ratio

double GeoGerry::Precinct::get_ratio() {
    // retrieve ratio from precinct
    if (rep == 0 && dem == 0)
        return -1;

    #ifdef REP
        return (double) rep / ((double) dem + (double) rep);
    #endif
    #ifndef REP
        return dem / (dem + rep);
    #endif
}


double GeoGerry::Precinct_Group::get_ratio() {
    double average = 0;
    for (Precinct p : precincts)
        average += p.get_ratio();
    average /= precincts.size();

    return average;
}


std::vector<int> GeoGerry::Precinct::get_voter_data() {
    // get vector of voting data
    return {dem, rep};
}


int GeoGerry::Precinct_Group::get_population() {
    // Returns total population in a Precinct_Group

    int total = 0;
    for (GeoGerry::Precinct p : precincts)
        total += p.pop;

    return total;
}


void GeoGerry::Precinct_Group::add_precinct_n(Precinct pre) {
    precincts.push_back(pre);
}


void GeoGerry::Precinct_Group::remove_precinct_n(GeoGerry::Precinct pre) {

    if (std::find(precincts.begin(), precincts.end(), pre) != precincts.end()) {
        precincts.erase(std::remove(precincts.begin(), precincts.end(), pre), precincts.end());
    }
    else {
        throw GeoGerry::Exceptions::PrecinctNotInGroup();   
    }
}


void GeoGerry::Precinct_Group::remove_precinct_n(GeoGerry::p_index pre) {
    if (pre < 0 || pre >= precincts.size()) {
        throw GeoGerry::Exceptions::PrecinctNotInGroup();
        return;
    }
    else {
        precincts.erase(precincts.begin() + pre);
    }
}


void GeoGerry::Precinct_Group::remove_precinct(GeoGerry::p_index pre) {

    if (pre < 0 || pre >= precincts.size()) {
        throw GeoGerry::Exceptions::PrecinctNotInGroup();
        return;
    }
    else {
        precincts.erase(precincts.begin() + pre);

        ClipperLib::Paths subj;
        for (Polygon shape : this->border)
            subj.push_back(ring_to_path(shape.hull));

        ClipperLib::Paths clip;
        clip.push_back(ring_to_path(precincts[pre].hull));

        ClipperLib::Paths solutions;
        ClipperLib::Clipper c; // the executor

        // execute union on paths array
        c.AddPaths(subj, ClipperLib::ptSubject, true);
        c.AddPaths(clip, ClipperLib::ptClip, true);
        c.Execute(ClipperLib::ctDifference, solutions, ClipperLib::pftNonZero);

        this->border = paths_to_multi_shape(solutions).border;
    }
}


void GeoGerry::Precinct_Group::remove_precinct(GeoGerry::Precinct pre) {
    /*
        @desc: Removes a precinct from a Precinct Group and updates the border with a difference
        @params: `Precinct` pre: Precinct to be removed
        @return: none
    */

    if (std::find(precincts.begin(), precincts.end(), pre) != precincts.end()) {
        precincts.erase(std::remove(precincts.begin(), precincts.end(), pre), precincts.end());

        ClipperLib::Paths subj;
        for (Polygon shape : this->border)
            subj.push_back(ring_to_path(shape.hull));

        ClipperLib::Paths clip;
        clip.push_back(ring_to_path(pre.hull));

        ClipperLib::Paths solutions;
        ClipperLib::Clipper c; // the executor

        // execute union on paths array
        c.AddPaths(subj, ClipperLib::ptSubject, true);
        c.AddPaths(clip, ClipperLib::ptClip, true);
        c.Execute(ClipperLib::ctDifference, solutions, ClipperLib::pftNonZero);

        this->border = paths_to_multi_shape(solutions).border;
    }
    else {
        throw GeoGerry::Exceptions::PrecinctNotInGroup();
    }
}


void GeoGerry::Precinct_Group::add_precinct(GeoGerry::Precinct pre) {
    /*
        @desc: Adds a precinct to a Precinct Group and updates the border with a union
        @params: `Precinct` pre: Precinct to be added
        @return: none
    */
   
    precincts.push_back(pre);
    if (border.size() == 0)
        border.push_back(pre.hull);
    else {
        ClipperLib::Paths subj;
        for (Polygon shape : border)
            subj.push_back(ring_to_path(shape.hull));

        ClipperLib::Paths clip;
        clip.push_back(ring_to_path(pre.hull));

        ClipperLib::Paths solutions;
        ClipperLib::Clipper c; // the executor

        // execute union on paths array
        c.AddPaths(subj, ClipperLib::ptSubject, true);
        c.AddPaths(clip, ClipperLib::ptClip, true);
        c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

        this->border = paths_to_multi_shape(solutions).border;
    }
}


std::string GeoGerry::Community::save_frame() {
    /*
        @desc:
            Saves a community configuration into a string by using
            the precinct id's with a comma separated list.
        @params: none
        @return: `string` saved communities
    */

    std::string line;
    for (Precinct p : precincts) {
        line += "\"" + p.shape_id + "\", ";
    }

    line = line.substr(0, line.size() - 2);
    // cout << line << endl;
    return line;
}


GeoGerry::Communities GeoGerry::Community::load_frame(std::string read_path, State precinct_list) {
    /*
        @desc:
            Given file path and precinct reference, reads a saved
            community configuration into an array of communities
        @params:
            `string` read_path: path to the saved community frame
            `GeoGerry::State` precinct_list: reference to get precinct geodata given id's
        @return: `Communities` loaded community array
    */

    Communities cs;
    std::string file = readf(read_path);
    std::stringstream fs(file);
    std::string line;

    while (getline(fs, line)) {
        Community c;
        std::vector<std::string> vals = split(line, "\"");

        for (std::string v : vals) {
            if (v != ", ") { // v contains a precinct id
                for (Precinct p : precinct_list.precincts) {
                    if (p.shape_id == v) {
                        c.add_precinct_n(p);
                    }
                } 
            }
        }

        cs.push_back(c);
    }

    return cs;
}


bool GeoGerry::operator== (GeoGerry::LinearRing l1, GeoGerry::LinearRing l2) {
    return (l1.border == l2.border);
}


bool GeoGerry::operator!= (GeoGerry::LinearRing l1, GeoGerry::LinearRing l2) {
    return (l1.border != l2.border);
}


bool GeoGerry::operator== (GeoGerry::Polygon p1, GeoGerry::Polygon p2) {
    return (p1.hull == p2.hull && p1.holes == p2.holes);
}


bool GeoGerry::operator!= (GeoGerry::Polygon p1, GeoGerry::Polygon p2) {
    return (p1.hull != p2.hull || p1.holes != p2.holes);
}


bool GeoGerry::operator== (GeoGerry::Precinct p1, GeoGerry::Precinct p2) {
    return (p1.hull == p2.hull && p1.holes == p2.holes && p1.dem == p2.dem && p1.rep == p2.rep && p1.pop == p2.pop);
}


bool GeoGerry::operator!= (GeoGerry::Precinct p1, GeoGerry::Precinct p2) {
    return (p1.hull != p2.hull || p1.holes != p2.holes || p1.dem != p2.dem || p1.rep != p2.rep || p1.pop != p2.pop);
}


bool GeoGerry::operator== (GeoGerry::Multi_Polygon& s1, GeoGerry::Multi_Polygon& s2) {
    return (s1.border == s2.border);
}


bool GeoGerry::operator!= (GeoGerry::Multi_Polygon& s1, GeoGerry::Multi_Polygon& s2) {
    return (s1.border != s2.border);
}


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


// void GeoGerry::Precinct_Group::write_binary(std::string path) {
//     std::ofstream ofs(path); // open output stream
//     boost::archive::binary_oarchive oa(ofs); // open archive stream
//     oa << *this; // put this pointer into stream
//     ofs.close(); // close stream
// }


// GeoGerry::Precinct_Group GeoGerry::Precinct_Group::read_binary(std::string path) {
//     GeoGerry::Precinct_Group pg = GeoGerry::Precinct_Group(); // blank object
//     std::ifstream ifs(path); // open input stream
//     boost::archive::binary_iarchive ia(ifs); // open archive stream
//     ia >> pg;
//     return pg; // return precinct group object
// }


template<class Archive> void GeoGerry::LinearRing::serialize(Archive & ar, const unsigned int version) {
    ar & border;
}


template<class Archive> void GeoGerry::State::serialize(Archive & ar, const unsigned int version) {
    // write districts, precincts, name, and border
    ar & districts;
    ar & precincts;
    ar & name;
    ar & border;
    ar & pop;
}


template<class Archive> void GeoGerry::Multi_Polygon::serialize(Archive & ar, const unsigned int version) {
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


template<class Archive> void GeoGerry::Polygon::serialize(Archive & ar, const unsigned int version) {
    // push shape id and border to archive streamm
    ar & shape_id;
    ar & hull;
    ar & holes;
    ar & pop;
}


// void GeoGerry::State::save_communities(std::string write_path, Communities communities) {
//     /*
//         Saves a community to a file at a specific point in the
//         pipeline. Useful for visualization and checks.

//         Save structure is as follows:
//             write_path/
//                 community_1
//                 ...
//                 community_n
//     */

//     int c_index = 0;
//     std::string file = "";

//     for (Community c : communities) {
//         file += c.save_frame() + "\n";
//         c_index++;
//     }

//     writef(file, write_path);
// }


// void GeoGerry::State::read_communities(std::string read_path) {
//     this->state_communities = Community::load_frame(read_path, *this);
//     for (int i = 0; i < state_communities.size(); i++)
//         state_communities[i].border = generate_exterior_border(state_communities[i]).border;

//     return;
// }


// void GeoGerry::State::playback_communities(std::string read_path) {
//     GeoDraw::Anim animation(150);

//     fs::path p(read_path);
//     std::vector<fs::directory_entry> v;

//     if (fs::is_directory(p)) {
//         std::copy(fs::directory_iterator(p), fs::directory_iterator(), std::back_inserter(v));
//         for (std::vector<fs::directory_entry>::const_iterator it = v.begin(); it != v.end();  ++ it ){
//             GeoDraw::Canvas canvas(900, 900);
//             Communities cs = Community::load_frame((*it).path().string(), *this);
//             for (Community c : cs)
//                 canvas.add_shape(generate_exterior_border(c));
//             animation.frames.push_back(canvas);
//         }
//     }

//     animation.playback();
    
//     return;
// }


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


std::string GeoGerry::Polygon::to_json() {
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


std::string GeoGerry::Multi_Polygon::to_json() {
    /*
        @desc: Converts a multiple shape object into geojson
        @params: none
        @return: `string` geojson object
    */

    std::string str = geojson_header + "{\"type\":\"Feature\",\"geometry\":{\"type\":\"MultiPolygon\",\"coordinates\":[";
    for (Polygon s : border) {
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
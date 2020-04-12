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

#include <new>
#include <iostream>

using std::cout;
using std::endl;

#define REP  // use rep / (dem + rep) as ratio


double Geometry::Precinct::get_ratio() {
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


double Geometry::Precinct_Group::get_ratio() {
    double average = 0;
    for (Precinct p : precincts)
        average += p.get_ratio();
    average /= precincts.size();

    return average;
}


std::vector<int> Geometry::Precinct::get_voter_data() {
    // get vector of voting data
    return {dem, rep};
}


int Geometry::Precinct_Group::get_population() {
    // Returns total population in a Precinct_Group

    int total = 0;
    for (Geometry::Precinct p : precincts)
        total += p.pop;

    return total;
}


void Geometry::Precinct_Group::add_precinct_n(Precinct pre) {
    precincts.push_back(pre);
}


void Geometry::Precinct_Group::remove_precinct_n(Geometry::Precinct pre) {

    if (std::find(precincts.begin(), precincts.end(), pre) != precincts.end()) {
        precincts.erase(std::remove(precincts.begin(), precincts.end(), pre), precincts.end());
    }
    else {
        throw Geometry::Exceptions::PrecinctNotInGroup();   
    }
}


void Geometry::Precinct_Group::remove_precinct_n(Geometry::p_index pre) {
    if (pre < 0 || pre >= precincts.size()) {
        throw Geometry::Exceptions::PrecinctNotInGroup();
        return;
    }
    else {
        precincts.erase(precincts.begin() + pre);
    }
}


void Geometry::Precinct_Group::remove_precinct(Geometry::p_index pre) {

    if (pre < 0 || pre >= precincts.size()) {
        throw Geometry::Exceptions::PrecinctNotInGroup();
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


void Geometry::Precinct_Group::remove_precinct(Geometry::Precinct pre) {
    /*
        @desc: Removes a precinct from a Precinct Group and updates the border with a difference
        @params: `Precinct` pre: Precinct to be removed
        @return: `void`
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
        throw Geometry::Exceptions::PrecinctNotInGroup();
    }
}


void Geometry::Precinct_Group::add_precinct(Geometry::Precinct pre) {
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


std::string Geometry::Community::save_frame() {
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


Geometry::Communities Geometry::Community::load_frame(std::string read_path, State precinct_list) {
    /*
        @desc:
            Given file path and precinct reference, reads a saved
            community configuration into an array of communities
        @params:
            `string` read_path: path to the saved community frame
            `Geometry::State` precinct_list: reference to get precinct geodata given id's
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


int Geometry::Graph::get_node(int id) {
    for (int i = 0; i < this->vertices.size(); i++) {
        if (this->vertices[i].id == id) return i;
    }

    return -1;
}


bool Geometry::operator== (Geometry::LinearRing l1, Geometry::LinearRing l2) {
    return (l1.border == l2.border);
}


bool Geometry::operator!= (Geometry::LinearRing l1, Geometry::LinearRing l2) {
    return (l1.border != l2.border);
}


bool Geometry::operator== (Geometry::Polygon p1, Geometry::Polygon p2) {
    return (p1.hull == p2.hull && p1.holes == p2.holes);
}


bool Geometry::operator!= (Geometry::Polygon p1, Geometry::Polygon p2) {
    return (p1.hull != p2.hull || p1.holes != p2.holes);
}


bool Geometry::operator== (Geometry::Precinct p1, Geometry::Precinct p2) {
    return (p1.hull == p2.hull && p1.holes == p2.holes && p1.dem == p2.dem && p1.rep == p2.rep && p1.pop == p2.pop);
}


bool Geometry::operator!= (Geometry::Precinct p1, Geometry::Precinct p2) {
    return (p1.hull != p2.hull || p1.holes != p2.holes || p1.dem != p2.dem || p1.rep != p2.rep || p1.pop != p2.pop);
}


bool Geometry::operator== (Geometry::Multi_Polygon& s1, Geometry::Multi_Polygon& s2) {
    return (s1.border == s2.border);
}


bool Geometry::operator!= (Geometry::Multi_Polygon& s1, Geometry::Multi_Polygon& s2) {
    return (s1.border != s2.border);
}


bool Geometry::operator< (Node l1, Node l2) {
    return (l1.edges.size() < l2.edges.size());
}

/*
    Write and read state file to and from binary

    Any instance of & operator is overloaded for 
    reading and writing, to avoid duplicate methods
    (see boost documentation for more information)
*/

// void Geometry::State::save_communities(std::string write_path, Communities communities) {
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


// void Geometry::State::read_communities(std::string read_path) {
//     this->state_communities = Community::load_frame(read_path, *this);
//     for (int i = 0; i < state_communities.size(); i++)
//         state_communities[i].border = generate_exterior_border(state_communities[i]).border;

//     return;
// }


// void Geometry::State::playback_communities(std::string read_path) {
//     Graphics::Anim animation(150);

//     fs::path p(read_path);
//     std::vector<fs::directory_entry> v;

//     if (fs::is_directory(p)) {
//         std::copy(fs::directory_iterator(p), fs::directory_iterator(), std::back_inserter(v));
//         for (std::vector<fs::directory_entry>::const_iterator it = v.begin(); it != v.end();  ++ it ){
//             Graphics::Canvas canvas(900, 900);
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


std::string Geometry::LinearRing::to_json() {
    /*
        @desc: converts a linear ring into a json array of coords
        @params: none
        @return: `string` json array
    */

    std::string str = "[";
    for (Geometry::coordinate c : border)
        str += "[" + std::to_string(c[0]) + ", " + std::to_string(c[1]) + "],";

    str = str.substr(0, str.size() - 1);
    str += "]";

    return str;
}


std::string Geometry::Precinct_Group::to_json() {
    /*
        @desc:
            converts a Precinct_Group object into a geojson document
            for viewing in mapshaper or elsewhere

        @params: none
        @return: `string` json array
    */

    std::string str = geojson_header;
    
    for (Geometry::Precinct p : precincts) {
        str += "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Polygon\",\"coordinates\":[";
        str += p.hull.to_json();
        for (Geometry::LinearRing hole : p.holes)
            str += "," + hole.to_json();
        str += "]}},";
    }

    str = str.substr(0, str.size() - 1); // remove comma
    str += "]}";
    return str;
}


std::string Geometry::Polygon::to_json() {
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


std::string Geometry::Multi_Polygon::to_json() {
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


void Geometry::State::write_binary(std::string path) {

    std::ofstream ofs(path);
    boost::archive::binary_oarchive oa(ofs);
    oa << *this;

    return;
}


Geometry::State Geometry::State::read_binary(std::string path) {
    State state;

    std::ifstream ifs(path);
    boost::archive::binary_iarchive ia(ifs);
    ia >> state;
    
    for (int i = 0; i < state.network.vertices.size(); i++) {
        state.network.vertices[i].precinct = &state.precincts[i];
    }

    return state;
}


namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize(Archive & ar, Geometry::State& s, const unsigned int version) {
            ar & s.districts;
            ar & s.network;
            ar & s.name;
            ar & boost::serialization::base_object<Geometry::Precinct_Group>(s);
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::Precinct_Group& s, const unsigned int version) {
            ar & s.precincts;
            ar & boost::serialization::base_object<Geometry::Multi_Polygon>(s);
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::Multi_Polygon& s, const unsigned int version) {
            ar & s.border;
            ar & s.shape_id;
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::Polygon& s, const unsigned int version) {
            ar & s.hull;
            ar & s.holes;
            ar & s.shape_id;
            ar & s.pop;
            ar & s.is_part_of_multi_polygon;
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::LinearRing& s, const unsigned int version) {
            ar & s.border;
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::Graph& s, const unsigned int version) {
            ar & s.edges;
            ar & s.vertices;
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::Node& s, const unsigned int version) {
            // ar & s.precinct;
            ar & s.edges;
            ar & s.id;
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::Precinct& s, const unsigned int version) {
            ar & s.dem;
            ar & s.rep;
            ar & boost::serialization::base_object<Geometry::Polygon>(s);
        }
    }
}
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


#include <boost/serialization/split_free.hpp>
#include <boost/serialization/utility.hpp>

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>

using std::cout;
using std::endl;

#define REP  // use rep / (dem + rep) as ratio

std::string geojson_header = "{\"type\": \"FeatureCollection\", \"features\":[";


/*
    This file is in need of some maintenence. The
    following should be done:

    - Split this into several files for organization
    - Fix documentation on all these methods
    - Remove duplicate template methods

*/


double Geometry::Precinct::get_ratio() {
    // retrieve ratio from precinct

    if (rep == -1 && dem == -1)
        return -1;

    #ifdef REP
        if (rep == 0 && dem == 0) return 0.5;
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


void Geometry::Precinct_Group::remove_precinct_n(int pre) {
    if (pre < 0 || pre >= precincts.size()) {
        throw Geometry::Exceptions::PrecinctNotInGroup();
        return;
    }
    else {
        precincts.erase(precincts.begin() + pre);
    }
}


void Geometry::Precinct_Group::remove_precinct(int pre) {

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


bool Geometry::operator< (const Node& l1, const Node& l2) {
    return (l1.edges.size() < l2.edges.size());
}


bool Geometry::operator== (const Node& l1, const Node& l2) {
    return (l1.id == l2.id);
}


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


void Geometry::State::to_binary(std::string path) {
    cout << "writing binary" << endl;
    std::ofstream ofs(path);
    boost::archive::binary_oarchive oa(ofs);
    oa << *this;
    return;
}


Geometry::State Geometry::State::from_binary(std::string path) {
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
        /*
            Add functions for serializing all shape
            and state nested objects. They now call base
            classes for more efficient serialization
        */
       
        template<class Archive>
        void serialize(Archive & ar, Geometry::State& s, const unsigned int version) {
            ar & boost::serialization::base_object<Geometry::Precinct_Group>(s);
            ar & s.districts;
            ar & s.name;
            ar & s.network;
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
            ar & s.edges;
            ar & s.id;
        }


        template<class Archive, class Key, class T>
        void serialize(Archive & ar, tsl::ordered_map<Key, T>& map, const unsigned int version) {
            split_free(ar, map, version); 
        }

        template<class Archive, class Key, class T>
        void save(Archive & ar, const tsl::ordered_map<Key, T>& map, const unsigned int /*version*/) {
            auto serializer = [&ar](const auto& v) { ar & v; };
            map.serialize(serializer);
        }

        template<class Archive, class Key, class T>
        void load(Archive & ar, tsl::ordered_map<Key, T>& map, const unsigned int /*version*/) {
            deserializer<Archive> des = deserializer<Archive>(ar);
            map = tsl::ordered_map<Key, T>::deserialize(des);
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::Precinct& s, const unsigned int version) {
            ar & s.dem;
            ar & s.rep;
            ar & s.pop;
            ar & boost::serialization::base_object<Geometry::Polygon>(s);
        }
    }
}
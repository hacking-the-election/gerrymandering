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

using namespace hte;

using std::cout;
using std::endl;
using std::string;

const string geojson_header = "{\"type\": \"FeatureCollection\", \"features\":[";

/*
    This file is in need of some maintenence. The
    following should be done:

    - Split this into several files for organization
    - Fix documentation on all these methods
    - Remove duplicate template methods

*/


int Geometry::Precinct_Group::get_population() {
    // Returns total population in a Precinct_Group

    int total = 0;
    for (Geometry::Precinct p : precincts)
        total += p.pop;

    return total;
}


void Geometry::Precinct_Group::remove_precinct(Geometry::Precinct pre) {
    /*
        @desc: Removes a precinct from a Precinct Group and updates the border with a difference
        @params: `Precinct` pre: Precinct to be removed
        @return: `void`

        @warn:
            a potentially expensive and dangerous std::find
            as the first operation is pretty bad so there's
            probably a better way with id's or ptrs or something
    */

    if (std::find(precincts.begin(), precincts.end(), pre) != precincts.end()) {
        precincts.erase(std::remove(precincts.begin(), precincts.end(), pre), precincts.end());
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

    // just add the precinct to the precinct group
    precincts.push_back(pre);

    // if (border.size() == 0)
    //     border.push_back(pre.hull);
    // else {
    //     ClipperLib::Paths subj;
    //     for (Polygon shape : border)
    //         subj.push_back(ring_to_path(shape.hull));

    //     ClipperLib::Paths clip;
    //     clip.push_back(ring_to_path(pre.hull));

    //     ClipperLib::Paths solutions;
    //     ClipperLib::Clipper c; // the executor

    //     // execute union on paths array
    //     c.AddPaths(subj, ClipperLib::ptSubject, true);
    //     c.AddPaths(clip, ClipperLib::ptClip, true);
    //     c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

    //     this->border = paths_to_multi_shape(solutions).border;
    // }
}


bool Geometry::operator== (const Geometry::LinearRing& l1, const Geometry::LinearRing& l2) {
    return (l1.border == l2.border);
}


bool Geometry::operator!= (const Geometry::LinearRing& l1, const Geometry::LinearRing& l2) {
    return (l1.border != l2.border);
}


bool Geometry::operator== (const Geometry::Polygon& p1, const Geometry::Polygon& p2) {
    return (p1.hull == p2.hull && p1.holes == p2.holes);
}


bool Geometry::operator!= (const Geometry::Polygon& p1, const Geometry::Polygon& p2) {
    return (p1.hull != p2.hull || p1.holes != p2.holes);
}


bool Geometry::operator== (const Geometry::Precinct& p1, const Geometry::Precinct& p2) {
    return (p1.hull == p2.hull && p1.holes == p2.holes && p1.voter_data == p2.voter_data && p1.pop == p2.pop);
}


bool Geometry::operator!= (const Geometry::Precinct& p1, const Geometry::Precinct& p2) {
    return (p1.hull != p2.hull || p1.holes != p2.holes || p1.voter_data != p2.voter_data || p1.pop != p2.pop);
}


bool Geometry::operator== (const Geometry::Multi_Polygon& s1, const Geometry::Multi_Polygon& s2) {
    return (s1.border == s2.border);
}


bool Geometry::operator!= (const Geometry::Multi_Polygon& s1, const Geometry::Multi_Polygon& s2) {
    return (s1.border != s2.border);
}


bool Geometry::operator< (const Node& l1, const Node& l2) {
    // return (l1.edges.size() < l2.edges.size());
    return (l1.precinct->get_centroid()[0] < l2.precinct->get_centroid()[0]);
}


bool Geometry::operator== (const Node& l1, const Node& l2) {
    return (l1.id == l2.id);
}


string Geometry::LinearRing::to_json() {
    /*
        @desc: converts a linear ring into a json array of coords
        @params: none
        @return: `string` json array
    */

    string str = "[";
    for (Geometry::coordinate c : border)
        str += "[" + std::to_string(c[0]) + ", " + std::to_string(c[1]) + "],";

    str = str.substr(0, str.size() - 1);
    str += "]";

    return str;
}


string Geometry::Precinct_Group::to_json() {
    /*
        @desc:
            converts a Precinct_Group object into a geojson document
            for viewing in mapshaper or elsewhere

        @params: none
        @return: `string` json array
    */

    string str = geojson_header;
    
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


string Geometry::Polygon::to_json() {
    /*
        @desc: Converts a normal shape object into geojson
        @params: none
        @return: `string` geojson object
    */

    string str = geojson_header;
    str += hull.to_json();
    str += "]}}";

    return str;
}


string Geometry::Multi_Polygon::to_json() {
    /*
        @desc: Converts a multiple shape object into geojson
        @params: none
        @return: `string` geojson object
    */

    string str = geojson_header + "{\"type\":\"Feature\",\"geometry\":{\"type\":\"MultiPolygon\",\"coordinates\":[";
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


void Geometry::State::to_binary(string path) {
    cout << "writing binary" << endl;
    std::ofstream ofs(path);
    boost::archive::binary_oarchive oa(ofs);
    oa << *this;
    return;
}


Geometry::State Geometry::State::from_binary(string path) {
    State state;
    std::ifstream ifs(path);
    boost::archive::binary_iarchive ia(ifs);
    ia >> state;

    for (int i = 0; i < state.network.vertices.size(); i++) {
        state.network.vertices[i].precinct = &state.precincts[i];
    }

    return state;
}


template<class Archive> class deserializer {
    public:
        deserializer(Archive& ar): m_ar(ar) {}
        
        template<typename T> T operator()() {
            T t; 
            m_ar & t; 
            
            return t;
        }

    private:
        Archive& m_ar;
};


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
            ar & s.centroid;
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
            ar & s.voter_data;
            ar & s.pop;
            ar & boost::serialization::base_object<Geometry::Polygon>(s);
        }
    }
}

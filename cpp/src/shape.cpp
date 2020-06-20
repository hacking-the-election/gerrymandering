/*=======================================
 shape.cpp:                     k-vernooy
 last modified:               Fri, Jun 19
 
 Definition of generic methods for shapes, 
 precincts, districts and states that do
 not include geometric and/or mathematical
 algorithms (see geometry.cpp).
========================================*/


#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "../include/hte.h"


using namespace hte;
using std::cout;
using std::endl;
using std::string;


const string geojsonHeader = "{\"type\": \"FeatureCollection\", \"features\":[";


int Data::PrecinctGroup::getPopulation() {
    // Returns total population in a Precinct_Group

    int total = 0;
    for (Data::Precinct p : precincts)
        total += p.pop;

    return total;
}


void Data::PrecinctGroup::removePrecinct(Data::Precinct pre) {
    /*
        @desc: Removes a precinct from a Precinct Group and updates the border with a difference
        @params: `Precinct` pre: Precinct to be removed
        @return: `void`
    */

    auto it = std::find(precincts.begin(), precincts.end(), pre);
    if (it != precincts.end()) {
        precincts.erase(it);
    }
    else {
        throw Util::Exceptions::PrecinctNotInGroup();
    }
}


void Data::PrecinctGroup::addPrecinct(Data::Precinct pre) {
    /*
        @desc: Adds a precinct to a Precinct Group and updates the border with a union
        @params: `Precinct` pre: Precinct to be added
        @return: none
    */

    // just add the precinct to the precinct group
    precincts.push_back(pre);
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


bool Data::operator== (const Data::Precinct& p1, const Data::Precinct& p2) {
    return (p1.shapeId == p2.shapeId);
}


bool Data::operator!= (const Data::Precinct& p1, const Data::Precinct& p2) {
    return (p1.shapeId != p2.shapeId);
}


bool Geometry::operator== (const Geometry::MultiPolygon& s1, const Geometry::MultiPolygon& s2) {
    return (s1.border == s2.border);
}


bool Geometry::operator!= (const Geometry::MultiPolygon& s1, const Geometry::MultiPolygon& s2) {
    return (s1.border != s2.border);
}


bool Algorithm::operator< (const Algorithm::Node& l1, const Algorithm::Node& l2) {
    return (l1.edges.size() < l2.edges.size());
}


bool Algorithm::operator== (const Algorithm::Node& l1, const Algorithm::Node& l2) {
    return (l1.id == l2.id);
}


string Geometry::LinearRing::toJson() {
    /*
        @desc: converts a linear ring into a json array of coords
        @params: none
        @return: `string` json array
    */

    string str = "[";
    for (Geometry::Point2d c : border)
        str += "[" + std::to_string(c.x) + ", " + std::to_string(c.y) + "],";

    str = str.substr(0, str.size() - 1);
    str += "]";

    return str;
}


string Data::PrecinctGroup::toJson() {
    /*
        @desc:
            converts a Precinct_Group object into a geojson document
            for viewing in mapshaper or elsewhere

        @params: none
        @return: `string` json array
    */

    string str = geojsonHeader;
    
    for (Data::Precinct p : precincts) {
        str += "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Polygon\",\"coordinates\":[";
        str += p.hull.toJson();
        for (Geometry::LinearRing hole : p.holes)
            str += "," + hole.toJson();
        str += "]}},";
    }

    str = str.substr(0, str.size() - 1); // remove comma
    str += "]}";
    return str;
}


string Geometry::Polygon::toJson() {
    /*
        @desc: Converts a normal shape object into geojson
        @params: none
        @return: `string` geojson object
    */

    string str = geojsonHeader;
    str += hull.toJson();
    str += "]}}";

    return str;
}


string Geometry::MultiPolygon::toJson() {
    /*
        @desc: Converts a multiple shape object into geojson
        @params: none
        @return: `string` geojson object
    */

    string str = geojsonHeader + "{\"type\":\"Feature\",\"geometry\":{\"type\":\"MultiPolygon\",\"coordinates\":[";
    for (Geometry::Polygon s : border) {
        str += "[";
        str += s.hull.toJson();
        for (LinearRing h : s.holes) {
            str += h.toJson();
        }
        str += "],";
    }
    
    str = str.substr(0, str.size() - 1);
    str += "]}}";

    return str;
}


void Data::State::toFile(string path) {
    std::ofstream ofs(path);
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
    return;
}


Data::State Data::State::fromFile(string path) {
    State state;
    std::ifstream ifs(path);
    boost::archive::text_iarchive ia(ifs);
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


/**
 * \cond
 */
namespace boost {
    namespace serialization {
        /*
            Add functions for serializing all shape
            and state nested objects. They now call base
            classes for more efficient serialization
        */

        template<class Archive>
        void serialize(Archive & ar, Data::State& s, const unsigned int version) {
            ar & boost::serialization::base_object<Geometry::Precinct_Group>(s);
            ar & s.districts;
            ar & s.network;
        }


        template<class Archive>
        void serialize(Archive & ar, Data::PrecinctGroup& s, const unsigned int version) {
            ar & s.precincts;
            ar & boost::serialization::base_object<Geometry::MultiPolygon>(s);
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::MultiPolygon& s, const unsigned int version) {
            ar & s.border;
            ar & s.shapeId;
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::Polygon& s, const unsigned int version) {
            ar & s.hull;
            ar & s.holes;
            ar & s.shape_id;
            ar & s.pop;
            ar & s.isPartOfMultiPolygon;
        }


        template<class Archive>
        void serialize(Archive & ar, Geometry::LinearRing& s, const unsigned int version) {
            ar & s.border;
            ar & s.centroid;
        }


        template<class Archive>
        void serialize(Archive & ar, Algorithm::Graph& s, const unsigned int version) {
            ar & s.edges;
            ar & s.vertices;
        }


        template<class Archive>
        void serialize(Archive & ar, Algorithm::Node& s, const unsigned int version) {
            ar & s.edges;
            ar & s.id;
        }


        template<class Archive, class Key, class T>
        void serialize(Archive & ar, tsl::ordered_map<Key, T>& map, const unsigned int version) {
            split_free(ar, map, version); 
        }

        template<class Archive, class Key, class T>
        void save(Archive & ar, const tsl::ordered_map<Key, T>& map, const unsigned int version) {
            auto serializer = [&ar](const auto& v) { ar & v; };
            map.serialize(serializer);
        }

        template<class Archive, class Key, class T>
        void load(Archive & ar, tsl::ordered_map<Key, T>& map, const unsigned int version) {
            deserializer<Archive> des = deserializer<Archive>(ar);
            map = tsl::ordered_map<Key, T>::deserialize(des);
        }


        template<class Archive>
        void serialize(Archive & ar, Data::Precinct& s, const unsigned int version) {
            ar & s.voterData;
            ar & s.pop;
            ar & boost::serialization::base_object<Geometry::Polygon>(s);
        }
    }
}

/**
 * \endcond
 */

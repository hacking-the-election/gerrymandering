#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include "../include/hte_common.h"

// using namespace hte;
// using std::cout;
// using std::endl;
// using std::string;

// const string geojsonHeader = "{\"type\": \"FeatureCollection\", \"features\":[";


// hte::MultiPolygon::MultiPolygon(std::vector<hte::Precinct> s) {
//     // constructor with assignment
//     for (Precinct p : s) {
//         // copy precinct data to shape object
//         Polygon s = Polygon(p.hull, p.holes, p.shapeId);
//         border.push_back(s);
//     }
// }


// int hte::PrecinctGroup::getPopulation() {
//     int total = 0;
//     for (hte::Precinct p : precincts)
//         total += p.pop;
//     return total;
// }


// void hte::PrecinctGroup::removePrecinct(hte::Precinct pre) {
//     /*
//         @desc: Removes a precinct from a Precinct Group and updates the border with a difference
//         @params: `Precinct` pre: Precinct to be removed
//         @return: `void`
//     */

//     auto it = std::find(precincts.begin(), precincts.end(), pre);
//     if (it != precincts.end()) {
//         precincts.erase(it);
//     }
//     else {
//         throw Exceptions::PrecinctNotInGroup();
//     }
// }


// void hte::PrecinctGroup::addPrecinct(hte::Precinct pre) {
//     /*
//         @desc: Adds a precinct to a Precinct Group and updates the border with a union
//         @params: `Precinct` pre: Precinct to be added
//         @return: none
//     */

//     // just add the precinct to the precinct group
//     precincts.push_back(pre);
// }


// bool hte::operator== (const hte::Point2d& l1, const hte::Point2d& l2) {
//     return (l1.x == l2.x && l1.y == l2.y);
// }


// bool hte::operator!= (const hte::Point2d& l1, const hte::Point2d& l2) {
//     return (l1.x != l2.x || l1.y != l2.y);
// }


// bool hte::operator== (const hte::LinearRing& l1, const hte::LinearRing& l2) {
//     return (l1.border == l2.border);
// }


// bool hte::operator!= (const hte::LinearRing& l1, const hte::LinearRing& l2) {
//     return (l1.border != l2.border);
// }


// bool hte::operator== (const hte::Polygon& p1, const hte::Polygon& p2) {
//     return (p1.hull == p2.hull && p1.holes == p2.holes);
// }


// bool hte::operator!= (const hte::Polygon& p1, const hte::Polygon& p2) {
//     return (p1.hull != p2.hull || p1.holes != p2.holes);
// }


// bool hte::operator== (const hte::Precinct& p1, const hte::Precinct& p2) {
//     return (p1.shapeId == p2.shapeId);
// }


// bool hte::operator!= (const hte::Precinct& p1, const hte::Precinct& p2) {
//     return (p1.shapeId != p2.shapeId);
// }


// bool hte::operator== (const hte::MultiPolygon& s1, const hte::MultiPolygon& s2) {
//     return (s1.border == s2.border);
// }


// bool hte::operator!= (const hte::MultiPolygon& s1, const hte::MultiPolygon& s2) {
//     return (s1.border != s2.border);
// }


// bool hte::operator< (const hte::Node& l1, const hte::Node& l2) {
//     return (l1.edges.size() < l2.edges.size());
// }


// bool hte::operator== (const hte::Node& l1, const hte::Node& l2) {
//     return (l1.id == l2.id);
// }


// string hte::LinearRing::toJson() {
//     /*
//         @desc: converts a linear ring into a json array of coords
//         @params: none
//         @return: `string` json array
//     */

//     string str = "[";
//     for (hte::Point2d c : border)
//         str += "[" + std::to_string(c.x) + ", " + std::to_string(c.y) + "],";

//     str = str.substr(0, str.size() - 1);
//     str += "]";

//     return str;
// }


// string hte::PrecinctGroup::toJson() {
//     /*
//         @desc:
//             converts a PrecinctGroup object into a geojson document
//             for viewing in mapshaper or elsewhere

//         @params: none
//         @return: `string` json array
//     */

//     string str = geojsonHeader;
    
//     for (hte::Precinct p : precincts) {
//         str += "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Polygon\",\"coordinates\":[";
//         str += p.hull.toJson();
//         for (hte::LinearRing hole : p.holes)
//             str += "," + hole.toJson();
//         str += "]}},";
//     }

//     str = str.substr(0, str.size() - 1); // remove comma
//     str += "]}";
//     return str;
// }


// string hte::Polygon::toJson() {
//     /*
//         @desc: Converts a normal shape object into geojson
//         @params: none
//         @return: `string` geojson object
//     */

//     string str = geojsonHeader;
//     str += hull.toJson();
//     str += "]}}";

//     return str;
// }


// string hte::MultiPolygon::toJson() {
//     /*
//         @desc: Converts a multiple shape object into geojson
//         @params: none
//         @return: `string` geojson object
//     */

//     string str = geojsonHeader + "{\"type\":\"Feature\",\"geometry\":{\"type\":\"MultiPolygon\",\"coordinates\":[";
//     for (hte::Polygon s : border) {
//         str += "[";
//         str += s.hull.toJson();
//         for (LinearRing h : s.holes) {
//             str += h.toJson();
//         }
//         str += "],";
//     }
    
//     str = str.substr(0, str.size() - 1);
//     str += "]}}";

//     return str;
// }


// void hte::State::toFile(string path) {
//     std::ofstream ofs(path);
//     boost::archive::text_oarchive oa(ofs);
//     oa << *this;
//     return;
// }


// hte::State hte::State::fromFile(string path) {
//     State state;
//     std::ifstream ifs(path);
//     boost::archive::text_iarchive ia(ifs);
//     ia >> state;

//     for (int i = 0; i < state.network.vertices.size(); i++) {
//         state.network.vertices[i].precinct = &state.precincts[i];
//     }

//     return state;
// }


// template<class Archive> class deserializer {
//     public:
//         deserializer(Archive& ar): m_ar(ar) {}
        
//         template<typename T> T operator()() {
//             T t; 
//             m_ar & t; 
            
//             return t;
//         }

//     private:
//         Archive& m_ar;
// };


// /**
//  * \cond
//  */
// namespace boost {
//     namespace serialization {
//         /*
//             Add functions for serializing all shape
//             and state nested objects. They now call base
//             classes for more efficient serialization
//         */

//         template<class Archive>
//         void serialize(Archive & ar, hte::Point2d& c, const unsigned int version) {
//             ar & c.x;
//             ar & c.y;
//         }

//         template<class Archive>
//         void serialize(Archive & ar, hte::State& s, const unsigned int version) {
//             ar & boost::serialization::base_object<hte::PrecinctGroup>(s);
//             ar & s.districts;
//             ar & s.network;
//         }


//         template<class Archive>
//         void serialize(Archive & ar, hte::PrecinctGroup& s, const unsigned int version) {
//             ar & s.precincts;
//             ar & boost::serialization::base_object<hte::MultiPolygon>(s);
//         }


//         template<class Archive>
//         void serialize(Archive & ar, hte::MultiPolygon& s, const unsigned int version) {
//             ar & s.border;
//             ar & s.shapeId;
//         }


//         template<class Archive>
//         void serialize(Archive & ar, hte::Polygon& s, const unsigned int version) {
//             ar & s.hull;
//             ar & s.holes;
//             ar & s.shapeId;
//             ar & s.pop;
//             ar & s.isPartOfMultiPolygon;
//         }


//         template<class Archive>
//         void serialize(Archive & ar, hte::LinearRing& s, const unsigned int version) {
//             ar & s.border;
//             ar & s.centroid;
//         }


//         template<class Archive>
//         void serialize(Archive & ar, hte::Graph& s, const unsigned int version) {
//             ar & s.edges;
//             ar & s.vertices;
//         }


//         template<class Archive>
//         void serialize(Archive & ar, hte::Node& s, const unsigned int version) {
//             ar & s.edges;
//             ar & s.id;
//         }


//         template<class Archive, class Key, class T>
//         void serialize(Archive & ar, tsl::ordered_map<Key, T>& map, const unsigned int version) {
//             split_free(ar, map, version); 
//         }

//         template<class Archive, class Key, class T>
//         void save(Archive & ar, const tsl::ordered_map<Key, T>& map, const unsigned int version) {
//             auto serializer = [&ar](const auto& v) { ar & v; };
//             map.serialize(serializer);
//         }

//         template<class Archive, class Key, class T>
//         void load(Archive & ar, tsl::ordered_map<Key, T>& map, const unsigned int version) {
//             deserializer<Archive> des = deserializer<Archive>(ar);
//             map = tsl::ordered_map<Key, T>::deserialize(des);
//         }


//         template<class Archive>
//         void serialize(Archive & ar, hte::Precinct& s, const unsigned int version) {
//             ar & s.voterData;
//             ar & s.pop;
//             ar & boost::serialization::base_object<hte::Polygon>(s);
//         }
//     }
// }

// /**
//  * \endcond
//  */

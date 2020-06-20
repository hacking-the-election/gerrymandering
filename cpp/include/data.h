/*=======================================
 data.hpp                       k-vernooy
 last modified:               Fri, Jun 19
 
 Declarations of classes and functions
 for political data storage, parsing, and
 generation.
========================================*/

#ifndef DATA_H
#define DATA_H

#include <map>
#include <vector>

#include "geometry.h"
#include "algorithm.h"


namespace hte {
    namespace Data {
        enum class PoliticalParty;
        enum class IdType;
        class DataParser;

        /**
         * \brief An enum storing different political parties.
         */
        enum class PoliticalParty {
            Democrat, Republican, Green, Independent,
            Libertarian, Reform, Other, Total, AbsoluteQuantification
        };

        /**
         * \brief Different types of parsing ID's in geodata and election data
         */
        enum class IdType {
            GEOID,
            ELECTIONID,
            POPUID
        };


        /**
         * \brief Derived shape class for defining a precinct.
         * 
         * Contains border data, holes, and constituency data
         * Has total population, election data in the format of
         * the POLITICAL_PARTY enum, likely from the 2008 presidential election
         */
        class Precinct : public Geometry::Polygon {
            public:
                Precinct(){} // default constructor
                Precinct(Geometry::LinearRing ext)
                    : Geometry::Polygon(ext) {}
                Precinct(Geometry::LinearRing ext, ::std::string id)
                    : Geometry::Polygon(ext, id) {}
                Precinct(Geometry::LinearRing ext, ::std::vector<Geometry::LinearRing> interior)
                    : Geometry::Polygon(ext, interior) {}
                Precinct(Geometry::LinearRing ext, ::std::vector<Geometry::LinearRing> interior, ::std::string id)
                    : Geometry::Polygon(ext, interior, id) {}
                Precinct(Geometry::LinearRing ext, int popu, ::std::string id)
                    :  Geometry::Polygon(ext, id) {
                    this->pop = popu;
                }

                // add operator overloading for object equality
                friend bool operator== (const Precinct& p1, const Precinct& p2);
                friend bool operator!= (const Precinct& p1, const Precinct& p2);

                ::std::map<PoliticalParty, int> voterData;  //!< Voter data in the form `{POLITICAL_PARTY, count}`
        };


        /**
         * A class for defining a group of precincts.
         * Used for all geometric calculations in the
         * community generation algorithm
         */
        class PrecinctGroup : public Geometry::MultiPolygon {
            public:

                PrecinctGroup(){}
                PrecinctGroup(::std::vector<Polygon> shapes)
                    : MultiPolygon(shapes) {}
                
                PrecinctGroup(::std::vector<Precinct> precincts)
                    : MultiPolygon(precincts), precincts(precincts) {}


                // array of precinct objects
                ::std::vector<Precinct> precincts;

                ::std::string  toJson();
                int            getPopulation();
                void           removePrecinct(Precinct);
                void           addPrecinct(Precinct);
                Precinct       getPrecinctFromId(::std::string);
                double         getArea();

                Geometry::BoundingBox   getBoundingBox();
                Geometry::Point2d       getCentroid();
                
                static PrecinctGroup from_graph(Algorithm::Graph& g);

        };


        /**
         * \brief Shape class for defining a state.
         *        Includes arrays of precincts, and districts.
         * 
         * Contains methods for generating from binary files
         * or from raw data with proper district + precinct data
         * and serialization methods for binary and json.
        */
        class State : public PrecinctGroup {
            public:

                State(){}
                State(::std::vector<MultiPolygon> districts, ::std::vector<Precinct> t_precincts, ::std::vector<Polygon> shapes)
                    : PrecinctGroup(shapes), districts(districts) {
                    // simple assignment constructor for
                    // nonstatic superclass member
                    precincts = t_precincts;
                }

                // generate a file from proper raw input with and without additional voter data files
                static State GenerateFromFile(DataParser&);
                static State GenerateFromFile(::std::string, ::std::string, ::std::map<PoliticalParty, string>, ::std::map<IdType, ::std::string>);
                static State GenerateFromFile(::std::string, ::std::string, ::std::string, ::std::map<PoliticalParty, ::std::string>, ::std::map<IdType, ::std::string>);

                ::hte::Algorithm::Graph network; // represents the precinct network of the state
                ::std::vector<MultiPolygon> districts; // the actual districts of the state

                // serialize and read to and from binary, json
                void            toFile(::std::string path);
                static State    fromFile(::std::string path);
        };


        /**
         * Convert a string that represents a geojson polygon into an MultiPolygon object.
         * \param str The string to be converted into a vector of Polygons
         * \param texas_coordinates Whether or not to use texas-scaled coordinates 
         * \return The converted polygon object
         * 
         * \see MultiPolygon
        */
        Geometry::MultiPolygon StringToMultiPoly(::std::string str, bool texas_coordinates);
        
        /**
         * Convert a string that represents a geojson polygon into an object.
         * \param str The string to be converted into a Polygon
         * \param texas_coordinates Whether or not to use texas-scaled coordinates 
         * \return The converted polygon object
         * 
         * \see Polygon
        */
        Geometry::Polygon StringToPoly(::std::string str, bool texas_coordinates);

        // parsing functions for tsv files
        ::std::vector<::std::vector<::std::string > > parseSV(::std::string, ::std::string);
    }
}

#endif

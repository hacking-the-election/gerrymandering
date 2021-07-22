#ifndef _HTE_DATA_H
#define _HTE_DATA_H

#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "graph.h"
#include "geometry.h"


namespace hte {

template<typename T>
class LinearRing;

template<typename T>
class Polygon;

template<typename T>
class MultiPolygon;

class State;

typedef double UnitInterval;
enum class PoliticalParty {DEM, REP, GRE, IND, LIB, REF, OTH};
enum class FileType {PRE_GEO, BLOCK_GEO, PRE_DEMOGRAPHICS, BLOCK_DEMOGRAPHICS};
enum class IdType {GEOID, ELECTIONID, POPUID};


class GeoUnit : public Polygon<double>
{
public:
    template<typename ...Args>
    GeoUnit(Args&&... args) {Polygon<double>(std::forward<Args>(args)...);}

    // using Polygon<double>::Polygon;

private:
    std::string GEOID;
    std::map<PoliticalParty, int> voterData;

friend bool operator!= (const GeoUnit& p1, const GeoUnit& p2);
friend bool operator== (const GeoUnit& p1, const GeoUnit& p2);
};


// class GeoGroup : public MultiPolygon<double>
// {
// public:
//     using MultiPolygon<double>::MultiPolygon;

    // int getPopulation(bool update = false);
    // void removeUnit();
    // void addUnit();
    // static GeoGroup fromGraph(const Graph<GeoUnit*>& g);
// };


class DataParser
{
public:
    DataParser(const std::unordered_map<FileType, std::string>& fileLocations) : fileLocations(fileLocations) {};
    void parseToState(State& state);

private:
    std::unordered_map<FileType, std::string> fileLocations;
    static const std::unordered_map<FileType, void (*)(const std::string&) > parseFunctionPtrs;

    void parseGeoUnits(const std::string&);

    std::vector<GeoUnit> precincts;
    std::vector<std::shared_ptr<GeoUnit>> blocks;

    DataParser() = delete;
};


class State
{
public:
    State() {};

private:
    AdjacencyListGraph<std::shared_ptr<GeoUnit>, int> blockNetwork;
    std::vector<GeoUnit> blocks;

friend class DataParser;
};


// class Community : public Graph {
//     public:
//         double quantification;
//         double partisanQuantification;
        
//         // shape object for geometry methods,
//         // must be kept up to date in every operation
//         PrecinctGroup shape;

//         int  getPopulation();
//         void addNode(Node&);
//         void removeNode(int);
//         void resetShape(Graph&);

//         Community(std::vector<int>& nodeIds, Graph& graph);
//         Community() {}
// };


// double Accumulate(Communities& cs, std::function<double(const Community&)>);


// Communities LoadCommunitiesFromFile(std::string, Graph&);
// Communities LoadCommunitiesWithQuantification(std::string, Graph&, std::string);
// std::vector<std::vector<double> > LoadQuantification(std::string tsv);

// bool ExchangePrecinct(Graph& g, Communities& cs, int nodeToTake, int communityToTake);
// std::vector<std::array<int, 2> > GetAllExchanges(Graph& g, Communities& cs);

// void GradientDescentOptimization(Graph& g, Communities& cs, double (*measure)(Communities&));
// void SimulatedAnnealingOptimization(Graph& g, Communities& cs, double (*measure)(Community&));

// // calculate an accumulated value for a polygonal mask over a group of precincts
// template<typename T>
// T AccumulateForMask(PrecinctGroup pg, MultiPolygon mp);
}

#endif
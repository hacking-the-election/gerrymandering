/*=======================================
 parse.cpp:                     k-vernooy
 last modified:               Wed, Jun 17

 Definitions of state methods for parsing
 from geodata and election data (see data
 specs in root directory for information)
========================================*/

#include <iostream>      // std::cout and cin
#include <algorithm>     // for std::find and std::distance
#include <numeric>       // include std::iota
#include <iomanip>       // setprecision for debug
#include <iterator>      // for find algorithms

// for the rapidjson parser
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"
#include "../include/hte.h"

#define VERBOSE 1
// #define TEXAS_COORDS

using namespace rapidjson;
using namespace std;
using namespace hte;

// the number to multiply floating coordinates by
const long int COORD_SCALER = pow(2, 18);

// identifications for files
map<Data::IdType, string> idHeaders;
map<Data::PoliticalParty, string> electionHeaders;
const vector<string> nonPrecinctIds = {"9999", "WV", "ZZZZZ", "LAKE", "WWWWWW", "1808904150", "1812700460", "39095123ZZZ", "39043043ACN", "39123123ZZZ", "39043043ZZZ", "39093093999", "39035007999", "3908500799", "3900700799"};


class NodePair {
    public:
        array<int, 2> nodeIds;
        double distance;
        NodePair() {};

        bool operator<(const NodePair& l2) const {
            return this->distance < l2.distance;
        }
};


vector<vector<string> > parseSV(string tsv, string delimiter) {
    /*
        @desc:
            takes a tsv file as string, finds two
            dimensional array of cells and rows

        @params:
            `string` tsv: A delimiter separated file
            `string` delimiter: A delimiter to split by
        
        @return: `vector<vector<string> >` parsed array of data

        @warn:
            for some reason this will glitch
            when the id's are at the end of the line
    */

    stringstream file(tsv);
    string line;
    vector<vector<string> > data;

    while (getline(file, line)) {
        vector<string> row;
        size_t pos = 0;

        while ((pos = line.find(delimiter)) != string::npos) {
            row.push_back(line.substr(0, pos));
            line.erase(0, pos + delimiter.length());
        }

        row.push_back(line);
        data.push_back(row);
    }

    return data;
}


map<string, map<Data::PoliticalParty, int> > parseVoterData(string voterData) {
    /*
        @desc:
            from a string in the specified format,
            creates a map with the key of the precinct
            name and vector as `"name": {dem_vote, rep vote}`

        @params: `string` voter_data: tab separated voting file
        @return: `map<string, array<int, 2>>` parsed data
    */

    vector<vector<string> > dataList // two dimensional
        = parseSV(voterData, "\t"); // array of voter data

    int precinctIdCol = -1; // the column index that holds precinct id's
    map<Data::PoliticalParty, int> electionCols;

    for (int i = 0; i < dataList[0].size(); i++) {
        // val holds header string
        string val = dataList[0][i];
        if (val == idHeaders[Data::IdType::ELECTIONID])
            precinctIdCol = i;

        for (auto& vid : electionHeaders) {
            if (vid.second == val) {
                electionCols[vid.first] = i;
            }
        }
    }

    map<string, map<Data::PoliticalParty, int> > parsedData;

    // iterate over each precinct, skipping header
    for (int x = 1; x < dataList.size(); x++) {
        string id;

        // remove quotes from string
        if (dataList[x][precinctIdCol].substr(0, 1) == "\"")
            id = Util::Split(dataList[x][precinctIdCol], "\"")[1];
        else id = dataList[x][precinctIdCol];
                
        map<Data::PoliticalParty, int> precinctVoterData;

        // get the right voter columns, add to party total
        for (auto& partyCol : electionCols) {
            string d = dataList[x][partyCol.second];

            if (Util::IsNumber(d)) precinctVoterData[partyCol.first] += stoi(d);
        }

        parsedData[id] = precinctVoterData;
        // set the voter data of the precinct in the map
    }

    return parsedData; // return the filled map
}


Geometry::Polygon Data::StringToPoly(string str, bool texasCoord) {
    /*
        @desc: takes a json array string and returns a parsed shape object
        @params: `string` str: data to be parsed
        @return: `Polygon` parsed shape

        @warn:
            this function is so screwed up right now
            it takes stringified JSON as input and then 
            CONVERTS IT RIGHT BACK TO JSON WTH

            but I guess speed is really not important for
            a one time operation so not going to change this

            oohhhh yeah i forgot how to deduce types so this
            monstrosity happened
    */

    Geometry::Polygon v;
    Document mp;
    mp.Parse(str.c_str());

    Geometry::LinearRing hull;
    for (int i = 0; i < mp[0].Size(); i++) {
        double x, y;
        if (!texasCoord) {
            x = mp[0][i][0].GetDouble() * COORD_SCALER;
            y = mp[0][i][1].GetDouble() * COORD_SCALER;
        }
        else {
            x = mp[0][i][0].GetDouble();
            y = mp[0][i][1].GetDouble();
        }

        hull.border.push_back({static_cast<long>(x), static_cast<long>(y)});
    }

    // make sure that hole forms a complete LinearRing
    if (mp[0][0][0] != mp[0][mp[0].Size() - 1][0] || 
        mp[0][0][1] != mp[0][mp[0].Size() - 1][1]) {       
        hull.border.push_back(hull.border[0]);
    }

    v.hull = hull;

    for (int i = 1; i < mp.Size(); i++) {
        Geometry::LinearRing hole;
        for (int j = 0; j < mp[i].Size(); j++) {
            if (!texasCoord) {
                hole.border.push_back({
                    static_cast<long>(mp[i][j][0].GetDouble() * COORD_SCALER),
                    static_cast<long>(mp[i][j][1].GetDouble() * COORD_SCALER)
                });
            }
            else {
                hole.border.push_back({
                    static_cast<long>(mp[i][j][0].GetDouble()),
                    static_cast<long>(mp[i][j][1].GetDouble())
                });
            }
        }

        // make sure that all holes are closed
        if (mp[i][0][0] != mp[i][mp[i].Size() - 1][0] || 
            mp[i][0][1] != mp[i][mp[i].Size() - 1][1]) {       
            hole.border.push_back(hole.border[0]);
        }

        v.holes.push_back(hole);
    }

    return v;
}


Geometry::MultiPolygon Data::StringToMultiPoly(string str, bool texasCoord) {
    /*
        @desc: takes a json array string and returns a parsed multishape
        @params: `string` str: data to be parsed
        @return: `Polygon` parsed multishape
    */

    Geometry::MultiPolygon v;

    Document mp;
    mp.Parse(str.c_str());

    for (int i = 0; i < mp.Size(); i++) {
        // for each polygon
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);
        mp[i].Accept(writer);

        Geometry::Polygon polygon = StringToPoly(buffer.GetString(), texasCoord);
        v.border.push_back(polygon);
    }

    return v;
}


vector<Data::Precinct> ParsePrecinctData(string geoJSON) {
    /* 
        @desc:
            Parses a geoJSON file into an array of Polygon
            objects - finds ID using top-level defined constants,
            and splits multipolygons into separate shapes (of the same id)
    
        @params: `string` geoJSON: geojson precincts to be parsed
        @return: `vector<Polygon>` precinct objects with all data
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Data::Precinct> shapesVector;

    for (int i = 0; i < shapes["features"].Size(); i++) {
        string coords;
        string id = "";
        int pop = 0;

        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember(idHeaders[Data::IdType::GEOID].c_str())) {
            if (shapes["features"][i]["properties"][idHeaders[Data::IdType::GEOID].c_str()].IsInt()) {
                id = std::to_string(shapes["features"][i]["properties"][idHeaders[Data::IdType::GEOID].c_str()].GetInt());
            }
            else if (shapes["features"][i]["properties"][idHeaders[Data::IdType::GEOID].c_str()].IsString()) {
                id = shapes["features"][i]["properties"][idHeaders[Data::IdType::GEOID].c_str()].GetString();
            }
        }
        else {
            std::cout << "\e[31merror: \e[0mYou have no precinct id." << endl;
            std::cout << "If future k-vernooy runs into this error, it means that GEOID10 in your geoJSON in your voter data is missing. To fix... maybe try a loose comparison of the names?" << endl;
        }

        // get voter data from geodata
        map<Data::PoliticalParty, int> voterData;

        // get all democrat data from JSON, and add it to the total
        for (auto& partyHeader : electionHeaders) {
            int vote = -1;
            if (shapes["features"][i]["properties"].HasMember(partyHeader.second.c_str())) {
                if (shapes["features"][i]["properties"][partyHeader.second.c_str()].IsInt()) {
                    vote = shapes["features"][i]["properties"][partyHeader.second.c_str()].GetInt();
                }
                else if (shapes["features"][i]["properties"][partyHeader.second.c_str()].IsDouble()) {
                    vote = (int) shapes["features"][i]["properties"][partyHeader.second.c_str()].GetDouble();
                }
                else if (shapes["features"][i]["properties"][partyHeader.second.c_str()].IsString()) {
                    string str = shapes["features"][i]["properties"][partyHeader.second.c_str()].GetString();
                    if (str != "NA" && str != "" && str != " ")
                        vote = stoi(shapes["features"][i]["properties"][partyHeader.second.c_str()].GetString());
                }
                else std::cout << "VOTER DATA IN UNRECOGNIZED OR UNPARSABLE TYPE." << endl;
            }
            else std::cout << "\e[31merror: \e[0mNo voter data near parse.cpp:314 " << partyHeader.second << endl;
            voterData[partyHeader.first] = vote;
        }
        

        // get the population data from geodata
        if (shapes["features"][i]["properties"].HasMember(idHeaders[Data::IdType::POPUID].c_str())) {
            if (shapes["features"][i]["properties"][idHeaders[Data::IdType::POPUID].c_str()].IsInt()) {
                pop = shapes["features"][i]["properties"][idHeaders[Data::IdType::POPUID].c_str()].GetInt();
            }
            else if (shapes["features"][i]["properties"][idHeaders[Data::IdType::POPUID].c_str()].IsString()) {
                string test = shapes["features"][i]["properties"][idHeaders[Data::IdType::POPUID].c_str()].GetString();
                if (test != "" && test != "NA") {
                    pop = stoi(test);
                }
            }
        }
        else std::cout << "\e[31merror: \e[0mNo population data" << endl;

        bool texasCoordinates = false;
        #ifdef TEXAS_COORDS
        texasCoordinates = true;
        #endif

        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();

        if (shapes["features"][i]["geometry"]["type"] == "Polygon") {
            // vector parsed from coordinate string
            Geometry::Polygon geo = Data::StringToPoly(coords, texasCoordinates);
            Data::Precinct test();
            Data::Precinct precinct(geo.hull, pop, id);
            precinct.shapeId = id;

            if (electionHeaders.find(Data::PoliticalParty::Other) == electionHeaders.end()) {
                if (electionHeaders.find(Data::PoliticalParty::Total) != electionHeaders.end()) {
                    int other = voterData[Data::PoliticalParty::Total];
                    for (auto& party : electionHeaders) {
                        if (party.first != Data::PoliticalParty::Total) {
                            other -= voterData[party.first];
                        }
                    }
                    voterData[Data::PoliticalParty::Other] = other;
                }
            }

            precinct.voterData = voterData;

            for (int i = 0; i < geo.holes.size(); i++)
                precinct.holes.push_back(geo.holes[i]);

            shapesVector.push_back(precinct);
        }
        else {
            Geometry::MultiPolygon geo = Data::StringToMultiPoly(coords, texasCoordinates);
            geo.shapeId = id;

            // calculate area of multipolygon
            double totalArea = abs(geo.getSignedArea());
            int append = 0;

            for (Geometry::Polygon s : geo.border) {
                double fract = abs(s.getSignedArea()) / totalArea;
                pop = round(static_cast<double>(pop) * static_cast<double>(fract));

                map<Data::PoliticalParty, int> adjusted;
                for (auto& partyData : voterData) {
                    adjusted[partyData.first] = static_cast<int>(static_cast<double>(partyData.second) * fract);
                }

                if (electionHeaders.find(Data::PoliticalParty::Other) == electionHeaders.end()) {
                    if (electionHeaders.find(Data::PoliticalParty::Total) != electionHeaders.end()) {
                        int other = adjusted[Data::PoliticalParty::Total];

                        for (auto& party : electionHeaders) {
                            if (party.first != Data::PoliticalParty::Total) {
                                other -= adjusted[party.first];
                            }
                        }

                        adjusted[Data::PoliticalParty::Other] = other;
                    }
                }

                Data::Precinct precinct(s.hull, pop, (id + "_s" + std::to_string(append)));
                precinct.holes = s.holes;
                precinct.voterData = adjusted;
                shapesVector.push_back(precinct);
                append++;
            }
        }
    }

    return shapesVector;
}


vector<Geometry::Polygon> ParsePrecinctCoordinates(string geoJSON) {
    /* 
        @desc:
            Parses a geoJSON file into an array of Polygon
            objects - finds ID using top-level defined constants,
            and splits multipolygons into separate shapes (of the same id)
    
        @params: `string` geoJSON: geojson precincts to be parsed
        @return: `vector<Polygon>` precinct objects with coordinate data
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Geometry::Polygon> shapesVector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        string coords;
        string id = "";
        int pop = 0;
 
        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember(idHeaders[Data::IdType::GEOID].c_str())) {
            if (shapes["features"][i]["properties"][idHeaders[Data::IdType::GEOID].c_str()].IsInt()) {
                id = std::to_string(shapes["features"][i]["properties"][idHeaders[Data::IdType::GEOID].c_str()].GetInt());
            }
            else if (shapes["features"][i]["properties"][idHeaders[Data::IdType::GEOID].c_str()].IsString()) {
                id = shapes["features"][i]["properties"][idHeaders[Data::IdType::GEOID].c_str()].GetString();
            }
        }
        else {
            std::cout << idHeaders[Data::IdType::GEOID] << endl;
            std::cout << "\e[31merror: \e[0mYou have no precinct id." << endl;
        }

        // get the population from geodata
        if (shapes["features"][i]["properties"].HasMember(idHeaders[Data::IdType::POPUID].c_str())) {
            if (shapes["features"][i]["properties"][idHeaders[Data::IdType::POPUID].c_str()].IsInt())
                pop = shapes["features"][i]["properties"][idHeaders[Data::IdType::POPUID].c_str()].GetInt();
            else if (shapes["features"][i]["properties"][idHeaders[Data::IdType::POPUID].c_str()].IsString()){
                string tmp = shapes["features"][i]["properties"][idHeaders[Data::IdType::POPUID].c_str()].GetString();
                if (tmp != "" && tmp != "NA") pop = stoi(tmp);
            }
            else {
                std::cout << "Population data in unparseable format." << endl;
            }
        }
        else {
            cerr << idHeaders[Data::IdType::POPUID] << endl;
            cerr << "\e[31merror: \e[0mNo population data" << endl;
        }

        bool texasCoordinates = false;
        #ifdef TEXAS_COORDS
            texasCoordinates = true;
        #endif
        
        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();


        if (shapes["features"][i]["geometry"]["type"] == "Polygon") {
            // vector parsed from coordinate string
            Geometry::Polygon geo = Data::StringToPoly(coords, texasCoordinates);
            Geometry::LinearRing border = geo.hull;
            Geometry::Polygon shape(border, id);
            shape.pop = pop;
            shape.shapeId = id;

            for (int i = 0; i < geo.holes.size(); i++)
                shape.holes.push_back(geo.holes[i]);
            shapesVector.push_back(shape);
        }
        else {
            Geometry::MultiPolygon geo = Data::StringToMultiPoly(coords, texasCoordinates);
            geo.shapeId = id;
            double total_area = abs(geo.getSignedArea());

            // create many shapes with the same ID, add them to the array            
            int append = 0;
            for (Geometry::Polygon s : geo.border) {
                Geometry::Polygon shape(s.hull, s.holes, id);
                shape.isPartOfMultiPolygon = append;
                double fract = abs(shape.getSignedArea()) / total_area;
                shape.pop = static_cast<int>(round(pop * fract));
                shapesVector.push_back(shape);
                append++;
            }
        }
    }

    return shapesVector;
}


vector<Geometry::MultiPolygon> ParseDistrictCoordinates(string geoJSON) {
    /* 
        @desc: Parses a geoJSON file into an array of MultiPolygon district objects
        @params: `string` geoJSON: geojson precincts to be parsed
        @return: `vector<MultiPolygon>` district objects with coordinate data
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Geometry::MultiPolygon> shapesVector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        string coords;
        int pop = 0;

        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();

        if (shapes["features"][i]["geometry"]["type"] == "Polygon") {
            // vector parsed from coordinate string
            Geometry::Polygon border = Data::StringToPoly(coords, false);
            Geometry::MultiPolygon ms({border});
            shapesVector.push_back(ms);
        }
        else {
            Geometry::MultiPolygon borders = Data::StringToMultiPoly(coords, false);
            shapesVector.push_back(borders);
        }
    }

    return shapesVector;
}


vector<Data::Precinct> MergeData(vector<Geometry::Polygon> precinctShapes, map<string, map<Data::PoliticalParty, int> > voterData) {
    /*
        @desc:
            returns an array of precinct objects given
            geodata (shape objects) and voter data
            in the form of a map for a list of precincts

        @params:
            `vector<Polygon>` precinct_shapes: coordinate data with id's
            `map<string, vector<int>>` voter_data: voting data with id's
    */

    vector<Data::Precinct> precincts;

    int x = 0;

    for (Geometry::Polygon precinctShape : precinctShapes) {
        // iterate over shapes array, get the id of the current shape
        string pId = precinctShape.shapeId;
        map<Data::PoliticalParty, int> pData = {}; // the voter data to be filled

        if (voterData.find(pId) == voterData.end() ) {
            // there is no matching id in the voter data
            std::cout << "error: the id \e[41m" << pId << "\e[0m, has no matching key in voter_data, will not be filled" << endl;
        }
        else {
            // get the voter data of the precinct
            pData = voterData[pId];
        }

        // create a precinct object and add it to the array
        if (precinctShape.isPartOfMultiPolygon != -1) {
            double total_area = abs(precinctShape.getSignedArea());

            for (int i = 0; i < precinctShapes.size(); i++) {
                if (i != x && precinctShapes[i].shapeId == pId) {
                    total_area += abs(precinctShapes[i].getSignedArea());
                }
            }

            double ratio = abs(precinctShape.getSignedArea()) / total_area;

            Data::Precinct precinct = Data::Precinct(
                precinctShape.hull, 
                precinctShape.holes, 
                pId + "_s" + std::to_string(precinctShape.isPartOfMultiPolygon)
            );
            
            for (auto const& x : pData) {
                precinct.voterData[x.first] = static_cast<int>(static_cast<double>(x.second) * static_cast<double>(ratio));
            }

            if (electionHeaders.find(Data::PoliticalParty::Other) == electionHeaders.end()) {
                if (electionHeaders.find(Data::PoliticalParty::Total) != electionHeaders.end()) {
                    int other = precinct.voterData[Data::PoliticalParty::Total];
                    for (auto& party : electionHeaders) {
                        if (party.first != Data::PoliticalParty::Total) {
                            other -= precinct.voterData[party.first];
                        }
                    }
                    precinct.voterData[Data::PoliticalParty::Other] = other;
                }
            }

            precinct.pop = precinctShape.pop;
            precincts.push_back(precinct);
        }
        else {
            Data::Precinct precinct = 
                Data::Precinct(precinctShape.hull, precinctShape.holes, pId);
            
            if (electionHeaders.find(Data::PoliticalParty::Other) == electionHeaders.end()) {
                if (electionHeaders.find(Data::PoliticalParty::Total) != electionHeaders.end()) {
                    int other = pData[Data::PoliticalParty::Total];

                    for (auto& party : electionHeaders) {
                        if (party.first != Data::PoliticalParty::Total) {
                            other -= pData[party.first];
                        }
                    }

                    pData[Data::PoliticalParty::Other] = other;
                }
            }

            precinct.voterData = pData;
            precinct.pop = precinctShape.pop;
            precincts.push_back(precinct);
        }
        x++;
    }

    return precincts; // return precincts array
}


Data::PrecinctGroup CombineHoles(Data::PrecinctGroup pg) {
    /*
        Takes a precinct group, iterates through precincts
        with holes, and combines internal precinct data to
        eliminate holes from the precinct group
    */

    vector<Data::Precinct> precincts;
    vector<int> precinctsToIgnore;

    vector<Geometry::BoundingBox> bounds;
    for (Data::Precinct p : pg.precincts) {
        bounds.push_back(p.getBoundingBox());
    }


    for (int x = 0; x < pg.precincts.size(); x++) {
        // for each precinct in the pg array
        Data::Precinct p = pg.precincts[x];

        // define starting precinct metadata
        Geometry::LinearRing precinctBorder = p.hull;
        string id = p.shapeId;
        map<Data::PoliticalParty, int> voter = p.voterData;
        int pop = p.pop;

        if (p.holes.size() > 0) {
            // need to remove precinct holes
            int interior_pre = 0; // precincts inside the hole
            for (int j = 0; j < pg.precincts.size(); j++) {
                // check all other precincts for if they're inside
                Data::Precinct pC = pg.precincts[j];

                if (Geometry::GetBoundOverlap(bounds[x], bounds[j])) {
                    if (j != x && Geometry::GetInside(pC.hull, p.hull)) {
                        // precinct j is inside precinct x,
                        // add the appropriate data from j to x
                        for (auto const& x : pg.precincts[j].voterData) {
                            voter[x.first] += x.second;
                        }

                        // demv += pg.precincts[j].dem;
                        // repv += pg.precincts[j].rep;
                        pop += pg.precincts[j].pop;

                        // this precinct will not be returned
                        precinctsToIgnore.push_back(j);
                        interior_pre++;
                    }
                }
            }
        }

        // create new precinct from updated data
        Data::Precinct np = Data::Precinct(precinctBorder, pop, id);
        np.voterData = voter;
        precincts.push_back(np);
    }

    vector<Data::Precinct> newPre; // the new precinct array to return

    for (int i = 0; i < precincts.size(); i++) {
        if (!(std::find(precinctsToIgnore.begin(), precinctsToIgnore.end(), i) != precinctsToIgnore.end())) {
            // it is not a hole precinct, so add it
            newPre.push_back(precincts[i]);
        }
    }

    return Data::PrecinctGroup(newPre);
}


/**
 * \brief Determines the network graph of a given precinct group
 * 
 * Given a list of precincts, determines their connection network
 * and links islands.
 * \param pg: A Precinct_Group to get graph of
 * \return: A graph of the connection network
*/
Algorithm::Graph GenerateGraph(Data::PrecinctGroup pg) {
    // assign all precincts to be nodes
    Algorithm::Graph graph;

    for (int i = 0; i < pg.precincts.size(); i++) {
        // create all vertices with no edges
        Algorithm::Node n(&pg.precincts[i]);
        n.id = i;
        // assign precinct to node
        n.precinct = &pg.precincts[i];
        graph.vertices[n.id] = n;
    }

    // get bounding boxes for border-checks
    vector<Geometry::BoundingBox> boundingBoxes;
    for (Data::Precinct p : pg.precincts) {
        boundingBoxes.push_back(p.getBoundingBox());
    }

    // add bordering precincts as edges to the graph
    for (int i = 0; i < pg.precincts.size(); i++) {
        // check all unique combinations of precincts with get_bordering
        for (int j = i + 1; j < pg.precincts.size(); j++) {
            // check bounding box overlapping
            if (Geometry::GetBoundOverlap(boundingBoxes[i], boundingBoxes[j])) {
                // check clip because bounding boxes overlap
                if (Geometry::GetBordering(pg.precincts[i], pg.precincts[j])) {
                    graph.addEdge({j, i});
                }
            }
        }
    }

    // link components with closest precincts
    if (graph.getNumComponents() > 1) {
        // determine all centers of precincts
        std::map<int, Geometry::Point2d> centers;
        for (int i = 0; i < graph.vertices.size(); i++) {
            // add center of precinct to the map
            int key = (graph.vertices.begin() + i).key();
            Algorithm::Node pre = (graph.vertices[key]);
            Geometry::Point2d center = (graph.vertices.begin() + i).value().precinct->getCentroid();
            centers.insert({key, center});
        }

        while (!graph.isConnected()) {
            // add edges between two precincts on two islands
            // until `graph` is connected
            vector<Algorithm::Graph> components = graph.getComponents();
            Algorithm::Edge shortestPair;
            double shortestDistance = 100000000000;

            for (size_t i = 0; i < components.size(); i++) {
                for (size_t j = i + 1; j < components.size(); j++) {
                    // for each distinct combination of islands
                    for (size_t p = 0; p < components[i].vertices.size(); p++) {
                        for (size_t q = 0; q < components[j].vertices.size(); q++) {
                            // for each distinct precinct combination of the
                            // two current islands, determine distance between centers
                            int keyi = (components[i].vertices.begin() + p).key(), keyj = (components[j].vertices.begin() + q).key();
                            double distance = Geometry::GetDistance(centers[keyi], centers[keyj]);
                            
                            if (distance < shortestDistance) {
                                shortestDistance = distance;
                                shortestPair = {keyi, keyj};
                            }
                        }
                    }
                }
            }

            // link the shortest pair
            graph.addEdge(shortestPair);
        }
    } // else the graph is linked already

    return graph;
}


Geometry::LinearRing BoundToRing(Geometry::BoundingBox box) {
    return (Geometry::LinearRing({{box[3], box[0]}, {box[3], box[1]}, {box[2], box[1]}, {box[2], box[0]}}));
}


void ScalePrecinctsToDistrict(Data::State& state) {
    // determine bounding box of districts
    if (VERBOSE) std::cout << "scaling precincts to district bounds..." << endl;

    Geometry::BoundingBox districtB {-214748364, 214748364, 214748364, -214748364};
    Geometry::BoundingBox precinctB {-214748364, 214748364, 214748364, -214748364};

    for (Geometry::MultiPolygon p : state.districts) {
        Geometry::BoundingBox t = p.getBoundingBox();
        if (t[0] > districtB[0]) districtB[0] = t[0];
        if (t[1] < districtB[1]) districtB[1] = t[1];
        if (t[2] < districtB[2]) districtB[2] = t[2];
        if (t[3] > districtB[3]) districtB[3] = t[3];
    }

    for (Data::Precinct p : state.precincts) {
        Geometry::BoundingBox t = p.getBoundingBox();
        if (t[0] > precinctB[0]) precinctB[0] = t[0];
        if (t[1] < precinctB[1]) precinctB[1] = t[1];
        if (t[2] < precinctB[2]) precinctB[2] = t[2];
        if (t[3] > precinctB[3]) precinctB[3] = t[3];
    }

    // top distance of district / top distance of precinct
    double scaleTop = static_cast<double>(districtB[3] - districtB[2]) / static_cast<double>(precinctB[3] - precinctB[2]);
    double scaleRight = static_cast<double>(districtB[0] - districtB[1]) / static_cast<double>(precinctB[0] - precinctB[1]);

    precinctB[0] *= scaleRight;
    precinctB[1] *= scaleRight;
    precinctB[2] *= scaleTop;
    precinctB[3] *= scaleTop;

    int tu = districtB[1] - precinctB[1]; // how much to translate up
    int tr = districtB[2] - precinctB[2]; // how much to translate right

    for (int i = 0; i < state.precincts.size(); i++) {
        for (int j = 0; j < state.precincts[i].hull.border.size(); j++) {
            state.precincts[i].hull.border[j].y *= scaleRight;
            state.precincts[i].hull.border[j].x *= scaleTop;
            state.precincts[i].hull.border[j].x += tr;
            state.precincts[i].hull.border[j].y += tu;
        }
    }
}


Data::State Data::State::GenerateFromFile(string precinctGeoJSON, string voterData, string districtGeoJSON, map<Data::PoliticalParty, string> pId, map<Data::IdType, string> tId) {
    /*
        @desc:
            Parse precinct and district geojson, along with
            precinct voter data, into a State object.
    
        @params:
            `string` precinct_geoJSON: A string file with geodata for precincts
            `string` voter_data: A string file with tab separated voter data
            `string` district_geoJSON: A string file with geodata for districts

        @return: `State` parsed state object
    */

    electionHeaders = pId;
    idHeaders = tId;

    // generate shapes from coordinates
    if (VERBOSE) std::cout << "generating coordinate arrays..." << endl;
    vector<Geometry::Polygon> precinctShapes = ParsePrecinctCoordinates(precinctGeoJSON);
    vector<Geometry::MultiPolygon> districtShapes = ParseDistrictCoordinates(districtGeoJSON);
    
    // get voter data from election data file
    if (VERBOSE) std::cout << "parsing voter data from tsv..." << endl;
    map<string, map<Data::PoliticalParty, int> > precinctVoterData = parseVoterData(voterData);

    // create a vector of precinct objects from border and voter data
    if (VERBOSE) std::cout << "merging geodata with voter data into precincts..." << endl;
    vector<Data::Precinct> precincts = MergeData(precinctShapes, precinctVoterData);
    
    // remove water precincts from data
    std::cout << "removing water precincts... ";

    int nRemoved = 0;
    for (int i = 0; i < precincts.size(); i++) {
        string id = precincts[i].shapeId;
        for (string str : nonPrecinctIds) {
            if (id.find(str) != string::npos) {
                precincts.erase(precincts.begin() + i);
                i--;
                nRemoved++;
            }
        }
    }

    std::cout << nRemoved << endl;
    Data::PrecinctGroup preGroup(precincts);

    if (VERBOSE) std::cout << "removing holes..." << endl;
    // remove holes from precinct data
    preGroup = CombineHoles(preGroup);
    vector<Geometry::Polygon> stateShapeVec; // dummy exterior border

    // generate state data from files
    if (VERBOSE) std::cout << "generating state with precinct and district arrays..." << endl;
    Data::State state = Data::State(districtShapes, preGroup.precincts, stateShapeVec);

    #ifdef TEXAS_COORDS
        scale_precincts_to_district(state);
    #endif

    std::cout << "getting precinct centroids with boost" << endl;
    for (int i = 0; i < state.precincts.size(); i++) {
        BoostPoint2d center;
        boost::geometry::centroid(Geometry::RingToBoostPoly(state.precincts[i].hull), center);
        state.precincts[i].hull.centroid = {center.x(), center.y()};
    }

    state.network = GenerateGraph(state);
    std::cout << "complete!" << endl;
    return state; // return the state object
}


Data::State Data::State::GenerateFromFile(string precinctGeoJSON, string districtGeoJSON, map<Data::PoliticalParty, string> pId, map<Data::IdType, string> tId) {

    /*
        @desc:
            Parse precinct and district geojson, along with
            precinct voter data, into a State object.

        @params:
            `string` precinct_geoJSON: A string file with geodata for precincts
            `string` voter_data: A string file with tab separated voter data
            `string` district_geoJSON: A string file with geodata for districts

        @return: `State` parsed state object
    */

    electionHeaders = pId;
    idHeaders = tId;

    // generate shapes from coordinates
    if (VERBOSE) std::cout << "generating coordinate array from precinct file..." << endl;
    vector<Data::Precinct> precinctShapes = ParsePrecinctData(precinctGeoJSON);
    if (VERBOSE) std::cout << "generating coordinate array from district file..." << endl;
    vector<Geometry::MultiPolygon> districtShapes = ParseDistrictCoordinates(districtGeoJSON);


    if (VERBOSE) std::cout << "removing water precincts... ";
    int nRemoved = 0;
    for (int i = 0; i < precinctShapes.size(); i++) {
        string id = precinctShapes[i].shapeId;
        for (string str : nonPrecinctIds) {
            if (id.find(str) != string::npos) {
                precinctShapes.erase(precinctShapes.begin() + i);
                i--;
                nRemoved++;
            }
        }
    }

    std::cout << nRemoved << endl;


    // create a vector of precinct objects from border and voter data
    Data::PrecinctGroup preGroup(precinctShapes);

    // remove holes from precinct data
    if (VERBOSE) std::cout << "combining holes in precinct geodata..." << endl;
    preGroup = CombineHoles(preGroup);

    vector<Geometry::Polygon> stateShapeVec;  // dummy exterior border

    // generate state data from files
    if (VERBOSE) std::cout << "generating state with precinct and district arrays..." << endl;
    Data::State state = Data::State(districtShapes, preGroup.precincts, stateShapeVec);
    
    #ifdef TEXAS_COORDS
        scale_precincts_to_district(state);
    #endif

    std::cout << "getting centroids of precincst" << endl; 

    for (int i = 0; i < state.precincts.size(); i++) {
        BoostPoint2d center;
        boost::geometry::centroid(Geometry::RingToBoostPoly(state.precincts[i].hull), center);
        state.precincts[i].hull.centroid = {center.x(), center.y()};
    }

    state.network = GenerateGraph(state);
    if (VERBOSE) std::cout << "state serialized!" << endl;
    return state; // return the state object
}

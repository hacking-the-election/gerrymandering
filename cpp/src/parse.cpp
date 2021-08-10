#include <iostream> 
#include <algorithm>
#include <fstream>

#include "../include/hte_common.h"

#ifdef HTE_DATA_BUILD
#include "../lib/rapidjson/include/rapidjson/rapidjson.h"
#endif

namespace hte {

// vector<vector<string> > parseSV(string tsv, string delimiter) {
//     stringstream file(tsv);
//     string line;
//     vector<vector<string> > data;

//     while (getline(file, line)) {
//         vector<string> row;
//         size_t pos = 0;

//         while ((pos = line.find(delimiter)) != string::npos) {
//             row.push_back(line.substr(0, pos));
//             line.erase(0, pos + delimiter.length());
//         }

//         row.push_back(line);
//         data.push_back(row);
//     }

//     return data;
// }


// map<string, map<PoliticalParty, int> > parseVoterData(string voterData) {
//     vector<vector<string> > dataList = parseSV(voterData, "\t");
//     int precinctIdCol = -1;

//     map<PoliticalParty, int> electionCols;

//     for (int i = 0; i < dataList[0].size(); i++) {
//         string val = dataList[0][i];
//         if (val == idHeaders[IdType::ELECTIONID])
//             precinctIdCol = i;

//         for (auto& vid : electionHeaders) {
//             if (vid.second == val) {
//                 electionCols[vid.first] = i;
//             }
//         }
//     }

//     map<string, map<PoliticalParty, int> > parsedData;

//     // iterate over each precinct, skipping header
//     for (int x = 1; x < dataList.size(); x++) {
//         string id;

//         // remove quotes from string
//         if (dataList[x][precinctIdCol].substr(0, 1) == "\"")
//             id = Split(dataList[x][precinctIdCol], "\"")[1];
//         else id = dataList[x][precinctIdCol];
                
//         map<PoliticalParty, int> precinctVoterData;

//         // get the right voter columns, add to party total
//         for (auto& partyCol : electionCols) {
//             string d = dataList[x][partyCol.second];

//             if (IsNumber(d)) precinctVoterData[partyCol.first] += stoi(d);
//         }

//         parsedData[id] = precinctVoterData;
//         // set the voter data of the precinct in the map
//     }

//     return parsedData; // return the filled map
// }


// std::vector<GeoUnit> DataParser::parseGeoUnits(const std::string& jsonFile) {
//     std::ifstream ifs(jsonFile.c_str());
//     rapidjson::IStreamWrapper isw(ifs);
//     rapidjson::Document d;
//     d.ParseStream(isw);

//     std::vector<GeoUnit> units;

//     for (int i = 0; i < shapes["features"].Size(); i++) {
//         // see if the geoJSON contains the shape id
//         // if (shapes["features"][i]["properties"].HasMember() {
//         //     if (shapes["features"][i]["properties"][idHeaders[IdType::GEOID].c_str()].IsInt()) {
//         //         id = std::to_string(shapes["features"][i]["properties"][idHeaders[IdType::GEOID].c_str()].GetInt());
//         //     }
//         //     else if (shapes["features"][i]["properties"][idHeaders[IdType::GEOID].c_str()].IsString()) {
//         //         id = shapes["features"][i]["properties"][idHeaders[IdType::GEOID].c_str()].GetString();
//         //     }
//         // }
//         // else {
//         //     std::cout << "\e[31merror: \e[0mYou have no precinct id." << endl;
//         // }

//         // get voter data from geodata
//         std::unordered_map<PoliticalParty, int> voterData;

//         // get all democrat data from JSON, and add it to the total
//         for (auto& partyHeader : electionHeaders) {
//             int vote = -1;
//             if (shapes["features"][i]["properties"].HasMember(partyHeader.second.c_str())) {
//                 if (shapes["features"][i]["properties"][partyHeader.second.c_str()].IsInt()) {
//                     vote = shapes["features"][i]["properties"][partyHeader.second.c_str()].GetInt();
//                 }
//                 else if (shapes["features"][i]["properties"][partyHeader.second.c_str()].IsDouble()) {
//                     vote = (int) shapes["features"][i]["properties"][partyHeader.second.c_str()].GetDouble();
//                 }
//                 else if (shapes["features"][i]["properties"][partyHeader.second.c_str()].IsString()) {
//                     string str = shapes["features"][i]["properties"][partyHeader.second.c_str()].GetString();
//                     if (str != "NA" && str != "" && str != " ")
//                         vote = stoi(shapes["features"][i]["properties"][partyHeader.second.c_str()].GetString());
//                 }
//                 else std::cout << "VOTER DATA IN UNRECOGNIZED OR UNPARSABLE TYPE." << endl;
//             }
//             else std::cout << "\e[31merror: \e[0mNo voter data near parse.cpp:314 " << partyHeader.second << endl;
//             voterData[partyHeader.first] = vote;
//         }
        

//         // get the population data from geodata
//         if (shapes["features"][i]["properties"].HasMember(idHeaders[IdType::POPUID].c_str())) {
//             if (shapes["features"][i]["properties"][idHeaders[IdType::POPUID].c_str()].IsInt()) {
//                 pop = shapes["features"][i]["properties"][idHeaders[IdType::POPUID].c_str()].GetInt();
//             }
//             else if (shapes["features"][i]["properties"][idHeaders[IdType::POPUID].c_str()].IsString()) {
//                 string test = shapes["features"][i]["properties"][idHeaders[IdType::POPUID].c_str()].GetString();
//                 if (test != "" && test != "NA") {
//                     pop = stoi(test);
//                 }
//             }
//         }
//         else std::cout << "\e[31merror: \e[0mNo population data" << endl;

        

//         if (shapes["features"][i]["geometry"]["type"] == "Polygon") {
//             // vector parsed from coordinate string
//             Polygon geo = StringToPoly(coords, texasCoordinates);
//             Precinct precinct(geo.hull, pop, id);
//             precinct.shapeId = id;

//             if (electionHeaders.find(PoliticalParty::Other) == electionHeaders.end()) {
//                 if (electionHeaders.find(PoliticalParty::Total) != electionHeaders.end()) {
//                     int other = voterData[PoliticalParty::Total];
//                     for (auto& party : electionHeaders) {
//                         if (party.first != PoliticalParty::Total) {
//                             other -= voterData[party.first];
//                         }
//                     }
//                     voterData[PoliticalParty::Other] = other;
//                 }
//             }

//             precinct.voterData = voterData;

//             for (int i = 0; i < geo.holes.size(); i++)
//                 precinct.holes.push_back(geo.holes[i]);

//             shapesVector.push_back(precinct);
//         }
//         else {
//             MultiPolygon geo = StringToMultiPoly(coords, texasCoordinates);
//             geo.shapeId = id;

//             // calculate area of multipolygon
//             double totalArea = abs(geo.getSignedArea());
//             int append = 0;

//             for (Polygon s : geo.border) {
//                 double fract = abs(s.getSignedArea()) / totalArea;
//                 pop = round(static_cast<double>(pop) * static_cast<double>(fract));

//                 map<PoliticalParty, int> adjusted;
//                 for (auto& partyData : voterData) {
//                     adjusted[partyData.first] = static_cast<int>(static_cast<double>(partyData.second) * fract);
//                 }

//                 if (electionHeaders.find(PoliticalParty::Other) == electionHeaders.end()) {
//                     if (electionHeaders.find(PoliticalParty::Total) != electionHeaders.end()) {
//                         int other = adjusted[PoliticalParty::Total];

//                         for (auto& party : electionHeaders) {
//                             if (party.first != PoliticalParty::Total) {
//                                 other -= adjusted[party.first];
//                             }
//                         }

//                         adjusted[PoliticalParty::Other] = other;
//                     }
//                 }

//                 Precinct precinct(s.hull, pop, (id + "_s" + std::to_string(append)));
//                 precinct.holes = s.holes;
//                 precinct.voterData = adjusted;
//                 shapesVector.push_back(precinct);
//                 append++;
//             }
//         }
//     }

//     return shapesVector;
// }


// vector<MultiPolygon> ParseDistrictCoordinates(string geoJSON) {
//     Document shapes;
//     shapes.Parse(geoJSON.c_str()); // parse json

//     // vector of shapes to be returned
//     vector<MultiPolygon> shapesVector;

//     for ( int i = 0; i < shapes["features"].Size(); i++ ) {
//         string coords;
//         int pop = 0;

//         // create empty string buffer
//         StringBuffer buffer;
//         buffer.Clear();
//         Writer<StringBuffer> writer(buffer);

//         // write the coordinate array to a string
//         shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
//         coords = buffer.GetString();

//         if (shapes["features"][i]["geometry"]["type"] == "Polygon") {
//             // vector parsed from coordinate string
//             Polygon border = StringToPoly(coords, false);
//             MultiPolygon ms({border});
//             shapesVector.push_back(ms);
//         }
//         else {
//             MultiPolygon borders = StringToMultiPoly(coords, false);
//             shapesVector.push_back(borders);
//         }
//     }

//     return shapesVector;
// }


// vector<Precinct> MergeData(vector<Polygon> precinctShapes, map<string, map<PoliticalParty, int> > voterData) {
//     /*
//         @desc:
//             returns an array of precinct objects given
//             geodata (shape objects) and voter data
//             in the form of a map for a list of precincts

//         @params:
//             `vector<Polygon>` precinct_shapes: coordinate data with id's
//             `map<string, vector<int>>` voter_data: voting data with id's
//     */

//     vector<Precinct> precincts;

//     int x = 0;

//     for (Polygon precinctShape : precinctShapes) {
//         // iterate over shapes array, get the id of the current shape
//         string pId = precinctShape.shapeId;
//         map<PoliticalParty, int> pData = {}; // the voter data to be filled

//         if (voterData.find(pId) == voterData.end() ) {
//             // there is no matching id in the voter data
//             std::cout << "error: the id \e[41m" << pId << "\e[0m, has no matching key in voter_data, will not be filled" << endl;
//         }
//         else {
//             // get the voter data of the precinct
//             pData = voterData[pId];
//         }

//         // create a precinct object and add it to the array
//         if (precinctShape.isPartOfMultiPolygon != -1) {
//             double totalArea = abs(precinctShape.getSignedArea());

//             for (int i = 0; i < precinctShapes.size(); i++) {
//                 if (i != x && precinctShapes[i].shapeId == pId) {
//                     totalArea += abs(precinctShapes[i].getSignedArea());
//                 }
//             }

//             double ratio = abs(precinctShape.getSignedArea()) / totalArea;

//             Precinct precinct = Precinct(
//                 precinctShape.hull, 
//                 precinctShape.holes, 
//                 pId + "_s" + std::to_string(precinctShape.isPartOfMultiPolygon)
//             );
            
//             for (auto const& x : pData) {
//                 precinct.voterData[x.first] = static_cast<int>(static_cast<double>(x.second) * static_cast<double>(ratio));
//             }

//             if (electionHeaders.find(PoliticalParty::Other) == electionHeaders.end()) {
//                 if (electionHeaders.find(PoliticalParty::Total) != electionHeaders.end()) {
//                     int other = precinct.voterData[PoliticalParty::Total];
//                     for (auto& party : electionHeaders) {
//                         if (party.first != PoliticalParty::Total) {
//                             other -= precinct.voterData[party.first];
//                         }
//                     }
//                     precinct.voterData[PoliticalParty::Other] = other;
//                 }
//             }

//             precinct.pop = precinctShape.pop;
//             precincts.push_back(precinct);
//         }
//         else {
//             Precinct precinct = 
//                 Precinct(precinctShape.hull, precinctShape.holes, pId);
            
//             if (electionHeaders.find(PoliticalParty::Other) == electionHeaders.end()) {
//                 if (electionHeaders.find(PoliticalParty::Total) != electionHeaders.end()) {
//                     int other = pData[PoliticalParty::Total];

//                     for (auto& party : electionHeaders) {
//                         if (party.first != PoliticalParty::Total) {
//                             other -= pData[party.first];
//                         }
//                     }

//                     pData[PoliticalParty::Other] = other;
//                 }
//             }

//             precinct.voterData = pData;
//             precinct.pop = precinctShape.pop;
//             precincts.push_back(precinct);
//         }
//         x++;
//     }

//     return precincts; // return precincts array
// }


// PrecinctGroup CombineHoles(PrecinctGroup pg) {
//     /*
//         Takes a precinct group, iterates through precincts
//         with holes, and combines internal precinct data to
//         eliminate holes from the precinct group
//     */

//     vector<Precinct> precincts;
//     vector<int> precinctsToIgnore;

//     vector<BoundingBox> bounds;
//     for (Precinct p : pg.precincts) {
//         bounds.push_back(p.getBoundingBox());
//     }


//     for (int x = 0; x < pg.precincts.size(); x++) {
//         // for each precinct in the pg array
//         Precinct p = pg.precincts[x];

//         // define starting precinct metadata
//         LinearRing precinctBorder = p.hull;
//         string id = p.shapeId;
//         map<PoliticalParty, int> voter = p.voterData;
//         int pop = p.pop;

//         if (p.holes.size() > 0) {
//             // need to remove precinct holes
//             int interior_pre = 0; // precincts inside the hole
//             for (int j = 0; j < pg.precincts.size(); j++) {
//                 // check all other precincts for if they're inside
//                 Precinct pC = pg.precincts[j];

//                 if (GetBoundOverlap(bounds[x], bounds[j])) {
//                     if (j != x && GetInside(pC.hull, p.hull)) {
//                         // precinct j is inside precinct x,
//                         // add the appropriate data from j to x
//                         for (auto const& x : pg.precincts[j].voterData) {
//                             voter[x.first] += x.second;
//                         }

//                         // demv += pg.precincts[j].dem;
//                         // repv += pg.precincts[j].rep;
//                         pop += pg.precincts[j].pop;

//                         // this precinct will not be returned
//                         precinctsToIgnore.push_back(j);
//                         interior_pre++;
//                     }
//                 }
//             }
//         }

//         // create new precinct from updated data
//         Precinct np = Precinct(precinctBorder, pop, id);
//         np.voterData = voter;
//         precincts.push_back(np);
//     }

//     vector<Precinct> newPre; // the new precinct array to return

//     for (int i = 0; i < precincts.size(); i++) {
//         if (!(std::find(precinctsToIgnore.begin(), precinctsToIgnore.end(), i) != precinctsToIgnore.end())) {
//             // it is not a hole precinct, so add it
//             newPre.push_back(precincts[i]);
//         }
//     }

//     return PrecinctGroup(newPre);
// }


// /**
//  * \brief Determines the network graph of a given precinct group
//  * 
//  * Given a list of precincts, determines their connection network
//  * and links islands.
//  * \param pg: A Precinct_Group to get graph of
//  * \return: A graph of the connection network
// */
// Graph GenerateGraph(PrecinctGroup pg) {
//     // assign all precincts to be nodes
//     Graph graph;

//     for (int i = 0; i < pg.precincts.size(); i++) {
//         // create all vertices with no edges
//         Node n(&pg.precincts[i]);
//         n.id = i;
//         // assign precinct to node
//         n.precinct = &pg.precincts[i];
//         graph.vertices[n.id] = n;
//     }

//     // get bounding boxes for border-checks
//     vector<BoundingBox> boundingBoxes;
//     for (Precinct p : pg.precincts) {
//         boundingBoxes.push_back(p.getBoundingBox());
//     }

//     // add bordering precincts as edges to the graph
//     for (int i = 0; i < pg.precincts.size(); i++) {
//         // check all unique combinations of precincts with get_bordering
//         for (int j = i + 1; j < pg.precincts.size(); j++) {
//             // check bounding box overlapping
//             if (GetBoundOverlap(boundingBoxes[i], boundingBoxes[j])) {
//                 // check clip because bounding boxes overlap
//                 if (GetBordering(pg.precincts[i], pg.precincts[j])) {
//                     graph.addEdge({j, i});
//                 }
//             }
//         }
//     }

//     // link components with closest precincts
//     if (graph.getNumComponents() > 1) {
//         // determine all centers of precincts
//         std::map<int, Point2d> centers;
//         for (int i = 0; i < graph.vertices.size(); i++) {
//             // add center of precinct to the map
//             int key = (graph.vertices.begin() + i).key();
//             Node pre = (graph.vertices[key]);
//             Point2d center = (graph.vertices.begin() + i).value().precinct->getCentroid();
//             centers.insert({key, center});
//         }

//         while (!graph.isConnected()) {
//             // add edges between two precincts on two islands
//             // until `graph` is connected
//             vector<Graph> components = graph.getComponents();
//             Edge shortestPair;
//             double shortestDistance = 100000000000;

//             for (size_t i = 0; i < components.size(); i++) {
//                 for (size_t j = i + 1; j < components.size(); j++) {
//                     // for each distinct combination of islands
//                     for (size_t p = 0; p < components[i].vertices.size(); p++) {
//                         for (size_t q = 0; q < components[j].vertices.size(); q++) {
//                             // for each distinct precinct combination of the
//                             // two current islands, determine distance between centers
//                             int keyi = (components[i].vertices.begin() + p).key(), keyj = (components[j].vertices.begin() + q).key();
//                             double distance = GetDistance(centers[keyi], centers[keyj]);
                            
//                             if (distance < shortestDistance) {
//                                 shortestDistance = distance;
//                                 shortestPair = {keyi, keyj};
//                             }
//                         }
//                     }
//                 }
//             }

//             // link the shortest pair
//             graph.addEdge(shortestPair);
//         }
//     } // else the graph is linked already

//     return graph;
// }


// LinearRing BoundToRing(BoundingBox box) {
//     return (LinearRing({{box[3], box[0]}, {box[3], box[1]}, {box[2], box[1]}, {box[2], box[0]}}));
// }


// void ScalePrecinctsToDistrict(State& state) {
//     // determine bounding box of districts
//     if (VERBOSE) std::cout << "scaling precincts to district bounds..." << endl;

//     BoundingBox districtB {-214748364, 214748364, 214748364, -214748364};
//     BoundingBox precinctB {-214748364, 214748364, 214748364, -214748364};

//     for (MultiPolygon p : state.districts) {
//         BoundingBox t = p.getBoundingBox();
//         if (t[0] > districtB[0]) districtB[0] = t[0];
//         if (t[1] < districtB[1]) districtB[1] = t[1];
//         if (t[2] < districtB[2]) districtB[2] = t[2];
//         if (t[3] > districtB[3]) districtB[3] = t[3];
//     }

//     for (Precinct p : state.precincts) {
//         BoundingBox t = p.getBoundingBox();
//         if (t[0] > precinctB[0]) precinctB[0] = t[0];
//         if (t[1] < precinctB[1]) precinctB[1] = t[1];
//         if (t[2] < precinctB[2]) precinctB[2] = t[2];
//         if (t[3] > precinctB[3]) precinctB[3] = t[3];
//     }

//     // top distance of district / top distance of precinct
//     double scaleTop = static_cast<double>(districtB[3] - districtB[2]) / static_cast<double>(precinctB[3] - precinctB[2]);
//     double scaleRight = static_cast<double>(districtB[0] - districtB[1]) / static_cast<double>(precinctB[0] - precinctB[1]);

//     precinctB[0] *= scaleRight;
//     precinctB[1] *= scaleRight;
//     precinctB[2] *= scaleTop;
//     precinctB[3] *= scaleTop;

//     int tu = districtB[1] - precinctB[1]; // how much to translate up
//     int tr = districtB[2] - precinctB[2]; // how much to translate right

//     for (int i = 0; i < state.precincts.size(); i++) {
//         for (int j = 0; j < state.precincts[i].hull.border.size(); j++) {
//             state.precincts[i].hull.border[j].y *= scaleRight;
//             state.precincts[i].hull.border[j].x *= scaleTop;
//             state.precincts[i].hull.border[j].x += tr;
//             state.precincts[i].hull.border[j].y += tu;
//         }
//     }
// }


// State State::GenerateFromFile(string precinctGeoJSON, string voterData, string districtGeoJSON, map<PoliticalParty, string> pId, map<IdType, string> tId) {
//     /*
//         @desc:
//             Parse precinct and district geojson, along with
//             precinct voter data, into a State object.
    
//         @params:
//             `string` precinct_geoJSON: A string file with geodata for precincts
//             `string` voter_data: A string file with tab separated voter data
//             `string` district_geoJSON: A string file with geodata for districts

//         @return: `State` parsed state object
//     */

//     electionHeaders = pId;
//     idHeaders = tId;

//     // generate shapes from coordinates
//     if (VERBOSE) std::cout << "generating coordinate arrays..." << endl;
//     vector<Polygon> precinctShapes = ParsePrecinctCoordinates(precinctGeoJSON);
//     vector<MultiPolygon> districtShapes = ParseDistrictCoordinates(districtGeoJSON);
    
//     // get voter data from election data file
//     if (VERBOSE) std::cout << "parsing voter data from tsv..." << endl;
//     map<string, map<PoliticalParty, int> > precinctVoterData = parseVoterData(voterData);

//     // create a vector of precinct objects from border and voter data
//     if (VERBOSE) std::cout << "merging geodata with voter data into precincts..." << endl;
//     vector<Precinct> precincts = MergeData(precinctShapes, precinctVoterData);
    
//     // remove water precincts from data
//     std::cout << "removing water precincts... ";

//     int nRemoved = 0;
//     for (int i = 0; i < precincts.size(); i++) {
//         string id = precincts[i].shapeId;
//         for (string str : nonPrecinctIds) {
//             if (id.find(str) != string::npos) {
//                 precincts.erase(precincts.begin() + i);
//                 i--;
//                 nRemoved++;
//             }
//         }
//     }

//     std::cout << nRemoved << endl;
//     PrecinctGroup preGroup(precincts);

//     if (VERBOSE) std::cout << "removing holes..." << endl;
//     // remove holes from precinct data
//     preGroup = CombineHoles(preGroup);
//     vector<Polygon> stateShapeVec; // dummy exterior border

//     // generate state data from files
//     if (VERBOSE) std::cout << "generating state with precinct and district arrays..." << endl;
//     State state = State(districtShapes, preGroup.precincts, stateShapeVec);

//     #ifdef TEXAS_COORDS
//         ScalePrecinctsToDistrict(state);
//     #endif

//     std::cout << "getting precinct centroids with boost" << endl;
//     for (int i = 0; i < state.precincts.size(); i++) {
//         BoostPoint2d center;
//         boost::geometry::centroid(RingToBoostPoly(state.precincts[i].hull), center);
//         state.precincts[i].hull.centroid = {center.x(), center.y()};
//     }

//     state.network = GenerateGraph(state);
//     std::cout << "complete!" << endl;
//     return state; // return the state object
// }


// State DataParser::GenerateFromFile(string precinctGeoJSON, string districtGeoJSON, map<PoliticalParty, string> pId, map<IdType, string> tId) {
//     // generate shapes from coordinates
//     vector<Precinct> precinctShapes = ParsePrecinctData(precinctGeoJSON);
//     if (VERBOSE) std::cout << "generating coordinate array from district file..." << endl;
//     vector<MultiPolygon> districtShapes = ParseDistrictCoordinates(districtGeoJSON);


//     if (VERBOSE) std::cout << "removing water precincts... ";
//     int nRemoved = 0;
//     for (int i = 0; i < precinctShapes.size(); i++) {
//         string id = precinctShapes[i].shapeId;
//         for (string str : nonPrecinctIds) {
//             if (id.find(str) != string::npos) {
//                 precinctShapes.erase(precinctShapes.begin() + i);
//                 i--;
//                 nRemoved++;
//             }
//         }
//     }

//     std::cout << nRemoved << endl;


//     // create a vector of precinct objects from border and voter data
//     PrecinctGroup preGroup(precinctShapes);

//     // remove holes from precinct data
//     if (VERBOSE) std::cout << "combining holes in precinct geodata..." << endl;
//     preGroup = CombineHoles(preGroup);

//     vector<Polygon> stateShapeVec;  // dummy exterior border

//     // generate state data from files
//     if (VERBOSE) std::cout << "generating state with precinct and district arrays..." << endl;
//     State state = State(districtShapes, preGroup.precincts, stateShapeVec);
//     std::cout << "getting centroids of precincst" << endl; 

//     for (int i = 0; i < state.precincts.size(); i++) {
//         BoostPoint2d center;
//         boost::geometry::centroid(RingToBoostPoly(state.precincts[i].hull), center);
//         state.precincts[i].hull.centroid = {center.x(), center.y()};
//     }

//     state.network = GenerateGraph(state);
//     if (VERBOSE) std::cout << "state serialized!" << endl;
//     return state; // return the state object
// }



// const std::unordered_map<FileType, void (*)(const std::string&)> DataParser::parseFunctionPtrs = {
//     // {FileType::BLOCK_GEO, parseGeometry}
// };

LinearRing<double> parseRing(const rapidjson::Value& coordinates)
{
    LinearRing<double> lr = LinearRing<double>();
    lr.reserve(coordinates.GetArray().Size());

    for (const auto& coordinate : coordinates.GetArray())
    {
        lr.emplace_back(coordinate.GetArray()[0].GetDouble(), coordinate.GetArray()[1].GetDouble());
    }
    
    return lr;
}

Polygon<double> parsePolygon(const rapidjson::Value& coordinates)
{
    Polygon<double> poly(coordinates.GetArray().Size());

    for (size_t i = 0; i < coordinates.GetArray().Size(); i++)
    {
        poly[i] = std::move(parseRing(coordinates.GetArray()[i]));
    }

    return poly;
}


MultiPolygon<double> parseMultiPolygon(const rapidjson::Value& coordinates)
{
    MultiPolygon<double> multipoly(coordinates.GetArray().Size());

    for (const auto& arr : coordinates.GetArray())
    {
        multipoly.push_back(parsePolygon(arr));
    }

    return multipoly;
}


// template<typename T>
// static void show(T vec)
// {
//   std::cout << vec;
// }


// template<typename T>
// static void show(std::vector<T> vec)
// {
//   int size = vec.size();
//   if (size <= 0) {
//     std::cout << "invalid vector";
//     return;
//   }
//   std::cout << '{';
//   for (int l = 0; l < size - 1; l++) {
//     show(vec[l]);
//     std::cout << ',';
//   }
//   show(vec[size - 1]);
//   std::cout << '}';
// }

inline float RandomFloat()
{
    return static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
}


void DataParser::parseGeoUnits(const std::string& filepath)
{
    std::cout << "parsing geo blocks from " << filepath << std::endl;
    std::ifstream ifs(filepath);

    if (!ifs.is_open())
    {
        std::cerr << "Failed to open file" << std::endl;
    }

    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document doc;
    doc.ParseStream(isw);

    const rapidjson::Value& features = doc["features"];
    assert(features.IsArray());

    gl::Canvas* ctx = gl::Canvas::GetInstance();

    for (int i = 0; i < features.Size(); i++)
    {
        const rapidjson::Value& geometry = features[i]["geometry"];
        const rapidjson::Value& coordinates = geometry["coordinates"];
    
        gl::RGBColor c{RandomFloat(), RandomFloat(), RandomFloat()};
        gl::RGBColor cdark{c.r * 0.5, c.g * 0.5, c.b * 0.5};

        if (geometry["type"] == "MultiPolygon") 
        {
            for (const auto& poly : parseMultiPolygon(coordinates))
            {
                ctx->pushGeometry(poly, gl::Style{1, true, cdark, true, c});
            }
        }
        else
        {
            ctx->pushGeometry(parsePolygon(coordinates), gl::Style{1, true, cdark, true, c});
        }
    }

    
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    while (!glfwWindowShouldClose(ctx->getWindow()))
    {
        double currentTime = glfwGetTime();
        nbFrames++;

        if (currentTime - lastTime >= 1.0) {
            std::cout << 1000.0/double(nbFrames) << " ms/frame" << std::endl;
            nbFrames = 0;
            lastTime += 1.0;
        }

        ctx->renderCurrent();
    }
}


void DataParser::parseToState(State& state)
{
    // for (auto it : fileLocations)
    // {
    //     parseFunctionPtrs.at(it.first)(it.second);
    // }
    parseGeoUnits(fileLocations[FileType::BLOCK_GEO]);


    // state.blocks = std::move(blocks);

    // fractionally assign race data

    // distribute votes to blocks

    // create networks from block geometry;
}

}
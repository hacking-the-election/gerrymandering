/*=======================================
 parse.cpp:                     k-vernooy
 last modified:               Thu, Apr 23

 Definitions of state methods for parsing
 from geodata and election data (see data
 specs in root directory for information)
========================================*/

#include <iostream>      // cout and cin
#include <algorithm>     // for std::find and std::distance
#include <numeric>       // include std::iota
#include <iomanip>       // setprecision for debug
#include <iterator>      // for find algorithms

// for the rapidjson parser
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

#include "../include/shape.hpp"       // class definitions
#include "../include/canvas.hpp"      // class definitions
#include "../include/util.hpp"        // array modification functions
#include "../include/geometry.hpp"    // exterior border generation

#define VERBOSE 1        // print progress
#define TEXAS_COORDS 0   // absolute coordinates

using namespace rapidjson;
using namespace std;
using namespace hte;
using namespace Geometry;
using namespace Graphics;

// the number to multiply floating coordinates by
const long int c = pow(2, 18);

// identifications for files
map<ID_TYPE, string> id_headers;
map<POLITICAL_PARTY, string> election_headers;
const vector<string> non_precinct = {"9999", "WV", "ZZZZZ", "LAKE", "WWWWWW", "1808904150", "1812700460", "39095123ZZZ", "39043043ACN", "39123123ZZZ", "39043043ZZZ", "39093093999", "39035007999", "3908500799", "3900700799"};

// parsing functions for tsv files
vector<vector<string > > parse_sv(string, string);
bool check_column(vector<vector<string> >, int);


class NodePair {
    /*
        A class representing a link between two nodes.
        This is used for sorting in the link_islands method.
        @warn: this is not to be used as an edge.
    */

    public:
        array<int, 2> node_ids;
        double distance;
        NodePair() {};

        bool operator<(const NodePair& l2) const {
            return this->distance < l2.distance;
        }
};


vector<vector<string> > parse_sv(string tsv, string delimiter) {
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


bool check_column(vector<vector<string> > data_list, int index) {
    /*
        @desc:
            returns whether or not a given column in a two
            dimensional vector is empty at any given point

        @params:
            `string` data_list: The two dimensional array of data
            `int` index: The column index to check

        @return: `bool` column `index` is not empty
    */


    for (int i = 0; i < data_list.size(); i++)
        if (data_list[i][index].size() == 0)
            return false; // the column is empty at this cell

    return true;
}


map<string, map<POLITICAL_PARTY, int> > parse_voter_data(string voter_data) {
    /*
        @desc:
            from a string in the specified format,
            creates a map with the key of the precinct
            name and vector as `"name": {dem_vote, rep vote}`

        @params: `string` voter_data: tab separated voting file
        @return: `map<string, array<int, 2>>` parsed data
    */

    vector<vector<string> > data_list // two dimensional
        = parse_sv(voter_data, "\t"); // array of voter data

    int precinct_id_col = -1; // the column index that holds precinct id's
    map<POLITICAL_PARTY, int> election_columns;

    for ( int i = 0; i < data_list[0].size(); i++) {
        // val holds header string
        string val = data_list[0][i];

        if (val == id_headers[ID_TYPE::ELECTIONID])
            precinct_id_col = i;

        for (auto& vid : election_headers) {
            if (vid.second == val) {
                election_columns[vid.first] = i;
            }
        }
    }

    cout << "ID COLUMN: " << precinct_id_col << endl;
    cout << "PARTY COLUMNS: ";
    for  (auto& pair : election_columns) {
        cout << pair.second << " ";
    }
    cout << endl;

    map<string, map<POLITICAL_PARTY, int> > parsed_data;

    // iterate over each precinct, skipping header
    for (int x = 1; x < data_list.size(); x++) {
        string id;

        // remove quotes from string
        if (data_list[x][precinct_id_col].substr(0, 1) == "\"")
            id = split(data_list[x][precinct_id_col], "\"")[1];
        else id = data_list[x][precinct_id_col];
                
        map<POLITICAL_PARTY, int> precinct_voter_data;

        // get the right voter columns, add to party total
        for (auto& party_column : election_columns) {
            string d = data_list[x][party_column.second];

            if (is_number(d)) precinct_voter_data[party_column.first] += stoi(d);
        }

        parsed_data[id] = precinct_voter_data;
        // set the voter data of the precinct in the map
    }

    return parsed_data; // return the filled map
}


Polygon string_to_vector(string str, bool texas_coordinates) {
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

    Polygon v;
    Document mp;
    mp.Parse(str.c_str());

    LinearRing hull;
    for (int i = 0; i < mp[0].Size(); i++) {
        double x, double y;
        if (!texas_coordinates) {
            x = mp[0][i][0].GetDouble() * c;
            y = mp[0][i][1].GetDouble() * c;
        }
        else {
            x = mp[0][i][0].GetDouble() * 20;
            y = mp[0][i][1].GetDouble() * 20;
        }

        hull.border.push_back({(long int) x, (long int) y});
    }

    if (mp[0][0][0] != mp[0][mp[0].Size() - 1][0] || 
        mp[0][0][1] != mp[0][mp[0].Size() - 1][1]) {       
        hull.border.push_back(hull.border[0]);
    }

    v.hull = hull;

    for (int i = 1; i < mp.Size(); i++) {
        LinearRing hole;
        for (int j = 0; j < mp[i].Size(); j++) {
            if (!texas_coordinates) {
                hole.border.push_back({(long int) mp[i][j][0].GetDouble() * c, (long int) mp[i][j][1].GetDouble() * c});
            }
            else {
                hole.border.push_back({(long int) mp[i][j][0].GetDouble() * 2, (long int) mp[i][j][1].GetDouble() * 2});
            }
        }
        if (mp[i][0][0] != mp[i][mp[i].Size() - 1][0] || 
            mp[i][0][1] != mp[i][mp[i].Size() - 1][1]) {       
            hole.border.push_back(hole.border[0]);
        }

        v.holes.push_back(hole);
    }

    return v;
}


Multi_Polygon multi_string_to_vector(string str, bool texas_coordinates) {
    /*
        @desc: takes a json array string and returns a parsed multishape
        @params: `string` str: data to be parsed
        @return: `Polygon` parsed multishape
    */

    Multi_Polygon v;

    Document mp;
    mp.Parse(str.c_str());

    for (int i = 0; i < mp.Size(); i++) {
        // for each polygon
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);
        mp[i].Accept(writer);

        Polygon polygon = string_to_vector(buffer.GetString(), texas_coordinates);
        v.border.push_back(polygon);
    }

    return v;
}


vector<Precinct> parse_precinct_data(string geoJSON) {
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
    vector<Precinct> shapes_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        string coords;
        string id = "";
        int pop = 0;

        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember(id_headers[ID_TYPE::GEOID].c_str())) {
            if (shapes["features"][i]["properties"][id_headers[ID_TYPE::GEOID].c_str()].IsInt()) {
                id = std::to_string(shapes["features"][i]["properties"][id_headers[ID_TYPE::GEOID].c_str()].GetInt());
            }
            else if (shapes["features"][i]["properties"][id_headers[ID_TYPE::GEOID].c_str()].IsString()) {
                id = shapes["features"][i]["properties"][id_headers[ID_TYPE::GEOID].c_str()].GetString();
            }
        }
        else {
            cout << "\e[31merror: \e[0mYou have no precinct id." << endl;
            cout << "If future k-vernooy runs into this error, it means that GEOID10 in your geoJSON in your voter data is missing. To fix... maybe try a loose comparison of the names?" << endl;
        }

        // get voter data from geodata
        map<POLITICAL_PARTY, int> voter_data;

        // get all democrat data from JSON, and add it to the total
        for (auto& party_header : election_headers) {
            int vote = -1;
            if (shapes["features"][i]["properties"].HasMember(party_header.second.c_str())) {
                if (shapes["features"][i]["properties"][party_header.second.c_str()].IsInt()) {
                    vote = shapes["features"][i]["properties"][party_header.second.c_str()].GetInt();
                }
                else if (shapes["features"][i]["properties"][party_header.second.c_str()].IsDouble()) {
                    vote = (int) shapes["features"][i]["properties"][party_header.second.c_str()].GetDouble();
                }
                else if (shapes["features"][i]["properties"][party_header.second.c_str()].IsString()) {
                    string str = shapes["features"][i]["properties"][party_header.second.c_str()].GetString();
                    if (str != "NA" && str != "" && str != " ")
                        vote = stoi(shapes["features"][i]["properties"][party_header.second.c_str()].GetString());
                }
                else cout << "VOTER DATA IN UNRECOGNIZED OR UNPARSABLE TYPE." << endl;
            }
            else cout << "\e[31merror: \e[0mNo voter data near parse.cpp:314 " << party_header.second << endl;
            voter_data[party_header.first] = vote;
        }
        

        // get the population data from geodata
        if (shapes["features"][i]["properties"].HasMember(id_headers[ID_TYPE::POPUID].c_str())) {
            if (shapes["features"][i]["properties"][id_headers[ID_TYPE::POPUID].c_str()].IsInt()) {
                pop = shapes["features"][i]["properties"][id_headers[ID_TYPE::POPUID].c_str()].GetInt();
            }
            else if (shapes["features"][i]["properties"][id_headers[ID_TYPE::POPUID].c_str()].IsString()) {
                string test = shapes["features"][i]["properties"][id_headers[ID_TYPE::POPUID].c_str()].GetString();
                if (test != "" && test != "NA") {
                    pop = stoi(test);
                }
            }
        }
        else cout << "\e[31merror: \e[0mNo population data" << endl;

        bool texas_coordinates = false;
        #ifdef TEXAS_COORDS
            texas_coordinates = true;
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
            Polygon geo = string_to_vector(coords, texas_coordinates);
            Precinct precinct(geo.hull, pop, id);
            precinct.shape_id = id;

            if (election_headers.find(POLITICAL_PARTY::OTHER) == election_headers.end()) {
                if (election_headers.find(POLITICAL_PARTY::TOTAL) != election_headers.end()) {
                    int other = voter_data[POLITICAL_PARTY::TOTAL];

                    for (auto& party : election_headers) {
                        if (party.first != POLITICAL_PARTY::TOTAL) {
                            other -= voter_data[party.first];
                        }
                    }

                    voter_data[POLITICAL_PARTY::OTHER] = other;
                }
            }

            precinct.voter_data = voter_data;

            for (int i = 0; i < geo.holes.size(); i++)
                precinct.holes.push_back(geo.holes[i]);

            shapes_vector.push_back(precinct);
        }
        else {
            Multi_Polygon geo = multi_string_to_vector(coords, texas_coordinates);
            geo.shape_id = id;

            // calculate area of multipolygon
            double total_area = geo.get_area();
            int append = 0;

            for (Polygon s : geo.border) {
                double fract = s.get_area() / total_area;
                pop = round((double)pop * (double)fract);

                map<POLITICAL_PARTY, int> adjusted;
                for (auto& party_data : voter_data) {
                    adjusted[party_data.first] = (int)((double)party_data.second * fract);
                }

                if (election_headers.find(POLITICAL_PARTY::OTHER) == election_headers.end()) {
                    if (election_headers.find(POLITICAL_PARTY::TOTAL) != election_headers.end()) {
                        int other = adjusted[POLITICAL_PARTY::TOTAL];

                        for (auto& party : election_headers) {
                            if (party.first != POLITICAL_PARTY::TOTAL) {
                                other -= adjusted[party.first];
                            }
                        }

                        adjusted[POLITICAL_PARTY::OTHER] = other;
                    }
                }

                Precinct precinct(s.hull, pop, (id + "_s" + std::to_string(append)));
                precinct.holes = s.holes;
                precinct.voter_data = adjusted;
                if (precinct.shape_id == "560051901_s1"){
                    for (auto& p : precinct.voter_data) {
                        cout << p.second << ", " << endl;
                    }
                }

                shapes_vector.push_back(precinct);
                append++;
            }
        }
    }

    return shapes_vector;
}


vector<Polygon> parse_precinct_coordinates(string geoJSON) {
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
    vector<Polygon> shapes_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        string coords;
        string id = "";
        int pop = 0;

        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember(id_headers[ID_TYPE::GEOID].c_str())) {
            if (shapes["features"][i]["properties"][id_headers[ID_TYPE::GEOID].c_str()].IsInt()) {
                id = std::to_string(shapes["features"][i]["properties"][id_headers[ID_TYPE::GEOID].c_str()].GetInt());
            }
            else if (shapes["features"][i]["properties"][id_headers[ID_TYPE::GEOID].c_str()].IsString()) {
                id = shapes["features"][i]["properties"][id_headers[ID_TYPE::GEOID].c_str()].GetString();
            }
        }
        else {
            cout << id_headers[ID_TYPE::GEOID] << endl;
            cout << "\e[31merror: \e[0mYou have no precinct id." << endl;
            cout << "If future k-vernooy runs into this error, it means that GEOID10 in your geoJSON in your voter data is missing. To fix... maybe try a loose comparison of the names?" << endl;
        }

        // get the population from geodata
        
        if (shapes["features"][i]["properties"].HasMember(id_headers[ID_TYPE::POPUID].c_str())) {
            if (shapes["features"][i]["properties"][id_headers[ID_TYPE::POPUID].c_str()].IsInt())
                pop = shapes["features"][i]["properties"][id_headers[ID_TYPE::POPUID].c_str()].GetInt();
            else if (shapes["features"][i]["properties"][id_headers[ID_TYPE::POPUID].c_str()].IsString()){
                string tmp = shapes["features"][i]["properties"][id_headers[ID_TYPE::POPUID].c_str()].GetString();
                if (tmp != "" && tmp != "NA") pop = stoi(tmp);
            }
            else {
                cout << "Uncaught typerror - src/parse.cpp, line 421" << endl;
            }
        }
        else {
            cerr << id_headers[ID_TYPE::POPUID] << endl;
            cerr << "\e[31merror: \e[0mNo population data" << endl;
        }

        bool texas_coordinates = false;
        #ifdef TEXAS_COORDS
            texas_coordinates = true;
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
            Polygon geo = string_to_vector(coords, texas_coordinates);
            LinearRing border = geo.hull;

            Polygon shape(border, id);
            shape.pop = pop;
            shape.shape_id = id;

            for (int i = 0; i < geo.holes.size(); i++)
                shape.holes.push_back(geo.holes[i]);

            shapes_vector.push_back(shape);
        }
        else {
            Multi_Polygon geo = multi_string_to_vector(coords, texas_coordinates);
            geo.shape_id = id;
            // calculate area of multipolygon
            double total_area = geo.get_area();

            // create many shapes with the same ID, add them to the array
            
            int append = 0;
            for (Polygon s : geo.border) {
                Polygon shape(s.hull, s.holes, id);
                shape.is_part_of_multi_polygon = append;
                double fract = shape.get_area() / total_area;
                shape.pop = (int) round(pop * fract);
                shapes_vector.push_back(shape);
                append++;
            }
        }
    }

    return shapes_vector;
}


vector<Multi_Polygon> parse_district_coordinates(string geoJSON) {
    /* 
        @desc: Parses a geoJSON file into an array of Multi_Polygon district objects
        @params: `string` geoJSON: geojson precincts to be parsed
        @return: `vector<Multi_Polygon>` district objects with coordinate data
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Multi_Polygon> shapes_vector;

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
            Polygon border = string_to_vector(coords, false);
            Multi_Polygon ms({border});
            shapes_vector.push_back(ms);
        }
        else {
            Multi_Polygon borders = multi_string_to_vector(coords, false);
            shapes_vector.push_back(borders);
        }
    }

    return shapes_vector;
}


vector<Precinct> merge_data(vector<Polygon> precinct_shapes, map<string, map<POLITICAL_PARTY, int> > voter_data) {
    /*
        @desc:
            returns an array of precinct objects given
            geodata (shape objects) and voter data
            in the form of a map for a list of precincts

        @params:
            `vector<Polygon>` precinct_shapes: coordinate data with id's
            `map<string, vector<int>>` voter_data: voting data with id's
    */

    vector<Precinct> precincts;

    int x = 0;

    for (Polygon precinct_shape : precinct_shapes) {
        // iterate over shapes array, get the id of the current shape
        string p_id = precinct_shape.shape_id;
        map<POLITICAL_PARTY, int> p_data = {}; // the voter data to be filled

        if ( voter_data.find(p_id) == voter_data.end() ) {
            // there is no matching id in the voter data
            cout << "error: the id \e[41m" << p_id << "\e[0m, has no matching key in voter_data, will not be filled" << endl;
        }
        else {
            // get the voter data of the precinct
            p_data = voter_data[p_id];
        }

        // create a precinct object and add it to the array
        if (precinct_shape.is_part_of_multi_polygon != -1) {
            double total_area = precinct_shape.get_area();

            for (int i = 0; i < precinct_shapes.size(); i++) {
                if (i != x && precinct_shapes[i].shape_id == p_id) {
                    total_area += precinct_shapes[i].get_area();
                }
            }

            double ratio = precinct_shape.get_area() / total_area;

            Precinct precinct =
                Precinct(
                    precinct_shape.hull, 
                    precinct_shape.holes, 
                    p_id + "_s" + std::to_string(precinct_shape.is_part_of_multi_polygon)
                );
            
            for (auto const& x : p_data) {
                precinct.voter_data[x.first] = (int)((double) x.second * (double) ratio);
            }

            if (election_headers.find(POLITICAL_PARTY::OTHER) == election_headers.end()) {
                if (election_headers.find(POLITICAL_PARTY::TOTAL) != election_headers.end()) {
                    int other = precinct.voter_data[POLITICAL_PARTY::TOTAL];

                    for (auto& party : election_headers) {
                        if (party.first != POLITICAL_PARTY::TOTAL) {
                            other -= precinct.voter_data[party.first];
                        }
                    }

                    precinct.voter_data[POLITICAL_PARTY::OTHER] = other;
                }
            }

            precinct.pop = precinct_shape.pop;
            precincts.push_back(precinct);
        }
        else {
            Precinct precinct = 
                Precinct(precinct_shape.hull, precinct_shape.holes, p_id);
            

            if (election_headers.find(POLITICAL_PARTY::OTHER) == election_headers.end()) {
                if (election_headers.find(POLITICAL_PARTY::TOTAL) != election_headers.end()) {
                    int other = p_data[POLITICAL_PARTY::TOTAL];

                    for (auto& party : election_headers) {
                        if (party.first != POLITICAL_PARTY::TOTAL) {
                            other -= p_data[party.first];
                        }
                    }

                    p_data[POLITICAL_PARTY::OTHER] = other;
                }
            }

            precinct.voter_data = p_data;
            precinct.pop = precinct_shape.pop;
            precincts.push_back(precinct);
        }
        x++;
    }

    return precincts; // return precincts array
}


Precinct_Group combine_holes(Precinct_Group pg) {
    /*
        Takes a precinct group, iterates through precincts
        with holes, and combines internal precinct data to
        eliminate holes from the precinct group
    */

    vector<Precinct> precincts;
    vector<int> precincts_to_ignore;

    vector<bounding_box> bounds;
    for (Precinct p : pg.precincts) {
        bounds.push_back(p.get_bounding_box());
    }


    for (int x = 0; x < pg.precincts.size(); x++) {
        // for each precinct in the pg array
        Precinct p = pg.precincts[x];

        // define starting precinct metadata
        LinearRing precinct_border = p.hull;
        string id = p.shape_id;
        map<POLITICAL_PARTY, int> voter = p.voter_data;
        int pop = p.pop;

        if (p.holes.size() > 0) {
            // need to remove precinct holes
            int interior_pre = 0; // precincts inside the hole

            for (int j = 0; j < pg.precincts.size(); j++) {
                // check all other precincts for if they're inside
                Precinct p_c = pg.precincts[j];

                if (bound_overlap(bounds[x], bounds[j])) {
                    if (j != x && get_inside(p_c.hull, p.hull)) {
                        // precinct j is inside precinct x,
                        // add the appropriate data from j to x
                        for (auto const& x : pg.precincts[j].voter_data) {
                            voter[x.first] += x.second;
                        }

                        // demv += pg.precincts[j].dem;
                        // repv += pg.precincts[j].rep;
                        pop += pg.precincts[j].pop;

                        // this precinct will not be returned
                        precincts_to_ignore.push_back(j);
                        interior_pre++;
                    }
                }
            }
        }

        // create object from updated data
        Precinct np = Precinct(precinct_border, pop, id);
        np.voter_data = voter;
        precincts.push_back(np);
    }

    vector<Precinct> new_pre; // the new precinct array to return

    for (int i = 0; i < precincts.size(); i++) {
        if (!(std::find(precincts_to_ignore.begin(), precincts_to_ignore.end(), i) != precincts_to_ignore.end())) {
            // it is not a hole precinct, so add it
            new_pre.push_back(precincts[i]);
        }
    }

    return Precinct_Group(new_pre);
}


Graph generate_graph(Precinct_Group pg) {
    /*
        @desc: Determines the network graph of a given precinct group
        @param: `Precinct_Group` pg: precinct group to get graph of
        @return: `Graph` connection network
    */

    // assign all precincts to be nodes
    Graph graph;

    for (int i = 0; i < pg.precincts.size(); i++) {
        // create all vertices with no edges
        Node n(&pg.precincts[i]);
        n.id = i;
        // assign precinct to node
        n.precinct = &pg.precincts[i];
        graph.vertices[n.id] = n;
    }


    // get bounding boxes for border-checks
    cout << "determining bounding boxes..." << endl;
    vector<bounding_box> bounding_boxes;
    for (Precinct p : pg.precincts) {
        bounding_boxes.push_back(p.get_bounding_box());
    }

    // add bordering precincts as edges to the graph
    cout << "determining edges..." << endl;
    for (int i = 0; i < pg.precincts.size(); i++) {
        // check all unique combinations of precincts with get_bordering
        for (int j = i + 1; j < pg.precincts.size(); j++) {
            // check bounding box overlapping
            if (bound_overlap(bounding_boxes[i], bounding_boxes[j])) {
                // check clip because bounding boxes overlap
                if (get_bordering(pg.precincts[i], pg.precincts[j])) {
                    graph.add_edge({j, i});
                }
            }
        }
    }

    cout << "linking components" << endl;
    // link components with closest precincts
    if (graph.get_num_components() > 1) {
        // determine all centers of precincts
        cout << graph.get_num_components() << endl;
        std::map<int, coordinate> centers;

        for (int i = 0; i < graph.vertices.size(); i++) {
            // add center of precinct to the map
            int key = (graph.vertices.begin() + i).key();
            Node pre = (graph.vertices[key]);
            coordinate center = (graph.vertices.begin() + i).value().precinct->get_centroid();
            centers.insert({key, center});
        }


        while (graph.get_num_components() > 1) {
            // add edges between two precincts on two islands
            // until `graph` is connected

            vector<Graph> components = graph.get_components();
            vector<NodePair> dists;

            for (size_t i = 0; i < components.size(); i++) {
                for (size_t j = i + 1; j < components.size(); j++) {
                    // for each distinct combination of islands
                    for (size_t p = 0; p < components[i].vertices.size(); p++) {
                        for (size_t q = 0; q < components[j].vertices.size(); q++) {
                            // for each distinct precinct combination of the
                            // two current islands, determine distance between centers
                            int keyi = (components[i].vertices.begin() + p).key(), keyj = (components[j].vertices.begin() + q).key();
                            double distance = get_distance(centers[keyi], centers[keyj]);
                            
                            // add node pairing to list
                            NodePair np;
                            np.distance = distance;
                            np.node_ids = {keyi, keyj};
                            dists.push_back(np);
                        }
                    }
                }
            }

            // find the shortest link between any two precincts on any two islands islands
            array<int, 2> me = std::min_element(dists.begin(), dists.end())->node_ids;
            graph.add_edge({me[0], me[1]});
        }
    } // else the graph is linked already

    return graph;
}


LinearRing bound_to_shape(bounding_box box) {
    return (LinearRing({{box[3], box[0]}, {box[3], box[1]}, {box[2], box[1]}, {box[2], box[0]}}));
}


void scale_precincts_to_district(Geometry::State& state) {
    // determine bounding box of districts
    if (VERBOSE) cout << "scaling precincts to district bounds..." << endl;

    bounding_box district_b {-214748364, 214748364, 214748364, -214748364};
    bounding_box precinct_b {-214748364, 214748364, 214748364, -214748364};

    for (Multi_Polygon p : state.districts) {
        bounding_box t = p.get_bounding_box();
        if (t[0] > district_b[0]) district_b[0] = t[0];
        if (t[1] < district_b[1]) district_b[1] = t[1];
        if (t[2] < district_b[2]) district_b[2] = t[2];
        if (t[3] > district_b[3]) district_b[3] = t[3];
    }

    for (Precinct p : state.precincts) {
        bounding_box t = p.get_bounding_box();
        if (t[0] > precinct_b[0]) precinct_b[0] = t[0];
        if (t[1] < precinct_b[1]) precinct_b[1] = t[1];
        if (t[2] < precinct_b[2]) precinct_b[2] = t[2];
        if (t[3] > precinct_b[3]) precinct_b[3] = t[3];
    }

    cout << "need to translate " << precinct_b[1] << ", " << precinct_b[2] << " to " << district_b[1] << ", " << district_b[2] << endl;
     // translate lower left of bounding box of precinct to district
    int tu = district_b[1] - precinct_b[1]; // how much to translate up
    int tr = district_b[2] - precinct_b[2]; // how much to translate right
    cout << "going up " << tu << ", going right " << tr << endl;

    double scale_top = (double)(district_b[3] - district_b[2]) / (double)(precinct_b[3] - precinct_b[2]);
    double scale_right = (double)(district_b[0] - district_b[1]) / (double)(precinct_b[0] - precinct_b[1]);
    
    for (int i = 0; i < state.precincts.size(); i++) {
        for (int j = 0; j < state.precincts[i].hull.border.size(); j++) {
            state.precincts[i].hull.border[j][0] += tr;
            state.precincts[i].hull.border[j][1] += tu;

            state.precincts[i].hull.border[j][0] *= scale_right;
            state.precincts[i].hull.border[j][1] *= scale_top;
        }
    }

    precinct_b[0] += tu;
    precinct_b[0] *= scale_top;
    precinct_b[1] += tu;
    precinct_b[1] *= scale_top;
    precinct_b[2] += tr;
    precinct_b[2] *= scale_right;
    precinct_b[3] += tr;
    precinct_b[3] *= scale_right;


    cout << precinct_b[0] << ", " << precinct_b[1] << ", " << precinct_b[2] << precinct_b[3] << endl;
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(state));
    canvas.add_outline(to_outline(bound_to_shape(district_b)));
    canvas.add_outline(to_outline(bound_to_shape(precinct_b)));
    for (Multi_Polygon p : state.districts) {
        Outline o = to_outline(p.border[0].hull);
        o.style().thickness(1).outline(RGB_Color(0,0,0)).fill(RGB_Color(0,0,0));
        canvas.add_outline(o);
    }
    canvas.draw_to_window();
}


State State::generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON, map<POLITICAL_PARTY, string> pid, map<ID_TYPE, string> tid) {
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

    election_headers = pid;
    id_headers = tid;


    // generate shapes from coordinates
    if (VERBOSE) cout << "generating coordinate arrays..." << endl;
    vector<Polygon> precinct_shapes = parse_precinct_coordinates(precinct_geoJSON);
    vector<Multi_Polygon> district_shapes = parse_district_coordinates(district_geoJSON);
    
    // get voter data from election data file
    if (VERBOSE) cout << "parsing voter data from tsv..." << endl;
    map<string, map<POLITICAL_PARTY, int> > precinct_voter_data = parse_voter_data(voter_data);

    // create a vector of precinct objects from border and voter data
    if (VERBOSE) cout << "merging geodata with voter data into precincts..." << endl;
    vector<Precinct> precincts = merge_data(precinct_shapes, precinct_voter_data);
    
    // remove water precincts from data
    cout << "removing water precincts... ";

    int n_removed = 0;
    for (int i = 0; i < precincts.size(); i++) {
        string id = precincts[i].shape_id;
        for (string str : non_precinct) {
            if (id.find(str) != string::npos) {
                precincts.erase(precincts.begin() + i);
                i--;
                n_removed++;
            }
        }
    }

    cout << n_removed << endl;
    Precinct_Group pre_group(precincts);

    if (VERBOSE) cout << "removing holes..." << endl;
    // remove holes from precinct data
    pre_group = combine_holes(pre_group);
    vector<Polygon> state_shape_v; // dummy exterior border

    // generate state data from files
    if (VERBOSE) cout << "generating state with precinct and district arrays..." << endl;
    State state = State(district_shapes, pre_group.precincts, state_shape_v);

    scale_precincts_to_district(state);
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(state));
    for (Multi_Polygon p : state.districts) {
        Outline o = to_outline(p.border[0].hull);
        o.style().thickness(1).outline(RGB_Color(0,0,0)).fill(RGB_Color(0,0,0));
        canvas.add_outline(o);
    }
    canvas.draw_to_window();

    cout << "getting precinct centroids with boost" << endl;
    for (int i = 0; i < state.precincts.size(); i++) {
        boost_point center;
        boost::geometry::centroid(ring_to_boost_poly(state.precincts[i].hull), center);
        state.precincts[i].hull.centroid = {center.x(), center.y()};
    }


    state.network = generate_graph(pre_group);
    cout << "complete!" << endl;
    return state; // return the state object
}


State State::generate_from_file(string precinct_geoJSON, string district_geoJSON, map<POLITICAL_PARTY, string> pid, map<ID_TYPE, string> tid) {

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

    election_headers = pid;
    id_headers = tid;

    // generate shapes from coordinates
    if (VERBOSE) cout << "generating coordinate array from precinct file..." << endl;
    vector<Precinct> precinct_shapes = parse_precinct_data(precinct_geoJSON);
    if (VERBOSE) cout << "generating coordinate array from district file..." << endl;
    vector<Multi_Polygon> district_shapes = parse_district_coordinates(district_geoJSON);


    if (VERBOSE) cout << "removing water precincts... ";
    int n_removed = 0;
    for (int i = 0; i < precinct_shapes.size(); i++) {
        string id = precinct_shapes[i].shape_id;
        for (string str : non_precinct) {
            if (id.find(str) != string::npos) {
                precinct_shapes.erase(precinct_shapes.begin() + i);
                i--;
                n_removed++;
            }
        }
    }

    cout << n_removed << endl;


    // create a vector of precinct objects from border and voter data
    Precinct_Group pre_group(precinct_shapes);

    // remove holes from precinct data
    if (VERBOSE) cout << "combining holes in precinct geodata..." << endl;
    pre_group = combine_holes(pre_group);

    vector<Polygon> state_shape_v;  // dummy exterior border

    // generate state data from files
    if (VERBOSE) cout << "generating state with precinct and district arrays..." << endl;
    State state = State(district_shapes, pre_group.precincts, state_shape_v);
    
    Canvas canvas(900, 900);
    canvas.add_outlines(to_outline(state));
    for (Multi_Polygon p : state.districts) {
        Outline o = to_outline(p.border[0].hull);
        o.style().thickness(1).outline(RGB_Color(0,0,0)).fill(RGB_Color(0,0,0));
        canvas.add_outline(o);
    }
    canvas.draw_to_window();

    cout << "getting centroids of precincst" << endl; 

    for (int i = 0; i < state.precincts.size(); i++) {
        boost_point center;
        boost::geometry::centroid(ring_to_boost_poly(state.precincts[i].hull), center);
        state.precincts[i].hull.centroid = {center.x(), center.y()};
    }

    state.network = generate_graph(pre_group);
    if (VERBOSE) cout << "state serialized!" << endl;
    return state; // return the state object
}

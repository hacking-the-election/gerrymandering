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

// for the filesystem manip
#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>

#include "../include/shape.hpp"       // class definitions
#include "../include/canvas.hpp"      // class definitions
#include "../include/util.hpp"        // array modification functions
#include "../include/geometry.hpp"    // exterior border generation
#define VERBOSE 1  // print progress


namespace fs = boost::filesystem;
using namespace rapidjson;

using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::vector;
using std::array;
using std::stringstream;
using std::map;

// the number to multiply floating coordinates by
const long int c = pow(2, 18);

// identifications for files
string election_id_header = "";
vector<string> d_head = {""};
vector<string> r_head = {""};
string geodata_id = "";
string population_id = "";

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
        @desc: takes a tsv file as string, returns two dimensional array of cells and rows
        @params:
            `string` tsv: A delimiter separated file
            `string` delimiter: A delimiter to split by
        
        @return: `vector<vector<string> >` parsed array of data
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


map<string, vector<int> > parse_voter_data(string voter_data) {
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
    // indices of usable data
    vector<int> d_index;
    vector<int> r_index;

    for ( int i = 0; i < data_list[0].size(); i++) {
        string val = data_list[0][i];

        if (val == election_id_header)
            precinct_id_col = i;

        for (string head : d_head) {
            if (head == val) {
                d_index.push_back(i);
            }
        }

        for (string head : r_head) {
            if (head == val) {
                r_index.push_back(i);
            }
        }
    }

    map<string, vector<int> > parsed_data;

    // iterate over each precinct
    for (int x = 1; x < data_list.size(); x++) {
        string id;

        // remove quotes from string
        if (data_list[x][precinct_id_col].substr(0, 1) == "\"")
            id = split(data_list[x][precinct_id_col], "\"")[1];
        else
            id = data_list[x][precinct_id_col];
                
        int demT = 0, repT = 0;

        // get the right voter columns, add to party total
        for (int i = 0; i < d_index.size(); i++) {
            string d = data_list[x][d_index[i]];
            string r = data_list[x][r_index[i]];

            if (is_number(d)) 
                demT += stoi(d);
            if (is_number(r))
                repT += stoi(r);
        }

        parsed_data[id] = {demT, repT};
        // set the voter data of the precinct in the map
    }

    return parsed_data; // return the filled map
}


Geometry::Polygon string_to_vector(string str) {
    /*
        @desc: takes a json array string and returns a parsed shape object
        @params: `string` str: data to be parsed
        @return: `Geometry::Polygon` parsed shape
    */

    Geometry::Polygon v;
    Document mp;
    mp.Parse(str.c_str());

    Geometry::LinearRing hull;
    for (int i = 0; i < mp[0].Size(); i++) {
        double x = mp[0][i][0].GetDouble() * c;
        double y = mp[0][i][1].GetDouble() * c;

        hull.border.push_back({(long int) x, (long int) y});
    }

    if (mp[0][0][0] != mp[0][mp[0].Size() - 1][0] || 
        mp[0][0][1] != mp[0][mp[0].Size() - 1][1]) {       
        hull.border.push_back(hull.border[0]);
    }

    v.hull = hull;

    for (int i = 1; i < mp.Size(); i++) {
        Geometry::LinearRing hole;
        for (int j = 0; j < mp[i].Size(); j++) {
            hole.border.push_back({(long int) mp[i][j][0].GetDouble() * c, (long int) mp[i][j][1].GetDouble() * c});
        }
        if (mp[i][0][0] != mp[i][mp[i].Size() - 1][0] || 
            mp[i][0][1] != mp[i][mp[i].Size() - 1][1]) {       
            hole.border.push_back(hole.border[0]);
        }

        v.holes.push_back(hole);
    }

    return v;
}


Geometry::Multi_Polygon multi_string_to_vector(string str) {
    /*
        @desc: takes a json array string and returns a parsed multishape
        @params: `string` str: data to be parsed
        @return: `Geometry::Polygon` parsed multishape
    */

    Geometry::Multi_Polygon v;

    Document mp;
    mp.Parse(str.c_str());

    for (int i = 0; i < mp.Size(); i++) {
        // for each polygon
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);
        mp[i].Accept(writer);

        Geometry::Polygon polygon = string_to_vector(buffer.GetString());
        v.border.push_back(polygon);
    }

    return v;
}


vector<Geometry::Precinct> parse_precinct_data(string geoJSON) {
    /* 
        @desc:
            Parses a geoJSON file into an array of Polygon
            objects - finds ID using top-level defined constants,
            and splits multipolygons into separate shapes (of the same id)
    
        @params: `string` geoJSON: geojson precincts to be parsed
        @return: `vector<Geometry::Polygon>` precinct objects with all data
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Geometry::Precinct> shapes_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        string coords;
        string id = "";
        int pop = 0;

        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember(geodata_id.c_str())) {
            id = shapes["features"][i]["properties"][geodata_id.c_str()].GetString();
        }
        else {
            cout << "\e[31merror: \e[0mYou have no precinct id." << endl;
            cout << "If future k-vernooy runs into this error, it means that GEOID10 in your geoJSON in your voter data is missing. To fix... maybe try a loose comparison of the names?" << endl;
        }

        // get voter data from geodata
        int demv = 0;
        int repv = 0;

        for (string dem_head : d_head) {
            if (shapes["features"][i]["properties"].HasMember(dem_head.c_str())) {
                if (shapes["features"][i]["properties"][dem_head.c_str()].IsInt()) {
                    demv += shapes["features"][i]["properties"][dem_head.c_str()].GetInt();
                }
                else if (shapes["features"][i]["properties"][dem_head.c_str()].IsDouble()) {
                    demv += (int) shapes["features"][i]["properties"][dem_head.c_str()].GetDouble();
                }
            }
            else cout << "\e[31merror: \e[0mNo democratic voter data" << endl;
        }

        for (string rep_head : r_head) {
            if (shapes["features"][i]["properties"].HasMember(rep_head.c_str())) {
                if (shapes["features"][i]["properties"][rep_head.c_str()].IsInt()) {
                    repv += shapes["features"][i]["properties"][rep_head.c_str()].GetInt();
                }
                else if (shapes["features"][i]["properties"][rep_head.c_str()].IsDouble()) {
                    repv += (int) shapes["features"][i]["properties"][rep_head.c_str()].GetDouble();
                }
            }
            else cout << "\e[31merror: \e[0mNo republican voter data" << endl;
        }

        // get the population data from geodata
        if (shapes["features"][i]["properties"].HasMember(population_id.c_str()))
            pop = shapes["features"][i]["properties"][population_id.c_str()].GetInt();
        else cout << "\e[31merror: \e[0mNo population data" << endl;
        

        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();

        if (shapes["features"][i]["geometry"]["type"] == "Polygon") {
            // vector parsed from coordinate string
            Geometry::Polygon geo = string_to_vector(coords);
            Geometry::Precinct precinct(geo.hull, demv, repv, pop, id);
            precinct.shape_id = id;

            for (int i = 0; i < geo.holes.size(); i++)
                precinct.holes.push_back(geo.holes[i]);

            shapes_vector.push_back(precinct);
        }
        else {
            Geometry::Multi_Polygon geo = multi_string_to_vector(coords);
            geo.shape_id = id;
            // calculate area of multipolygon
            double total_area = geo.get_area();
            int append = 0;

            for (Geometry::Polygon s : geo.border) {
                double fract = s.get_area() / total_area;
                pop = round((double)pop * (double)fract);
                demv = round((double) demv * (double) fract);
                repv = round((double) repv * (double) fract);

                Geometry::Precinct precinct(s.hull, demv, repv, pop, (id + "_s" + std::to_string(append)));
                precinct.holes = s.holes;

                shapes_vector.push_back(precinct);
                append++;
            }
        }
    }

    return shapes_vector;
}


vector<Geometry::Polygon> parse_precinct_coordinates(string geoJSON) {
    /* 
        @desc:
            Parses a geoJSON file into an array of Polygon
            objects - finds ID using top-level defined constants,
            and splits multipolygons into separate shapes (of the same id)
    
        @params: `string` geoJSON: geojson precincts to be parsed
        @return: `vector<Geometry::Polygon>` precinct objects with coordinate data
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Geometry::Polygon> shapes_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        string coords;
        string id = "";
        int pop = 0;

        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember(geodata_id.c_str())) {
            id = shapes["features"][i]["properties"][geodata_id.c_str()].GetString();
        }
        else {
            cout << "\e[31merror: \e[0mYou have no precinct id." << endl;
            cout << "If future k-vernooy runs into this error, it means that GEOID10 in your geoJSON in your voter data is missing. To fix... maybe try a loose comparison of the names?" << endl;
        }

        // get the population from geodata
        if (shapes["features"][i]["properties"].HasMember(population_id.c_str())) {
            if (shapes["features"][i]["properties"][population_id.c_str()].IsInt())
                pop = shapes["features"][i]["properties"][population_id.c_str()].GetInt();
            else if (shapes["features"][i]["properties"][population_id.c_str()].IsString()){
                // cout << shapes["features"][i]["properties"][population_id.c_str()].GetString() << endl;
                string tmp = shapes["features"][i]["properties"][population_id.c_str()].GetString();
                if (tmp != "" && tmp != "NA") pop = stoi(tmp);
            }
            else {
                cout << "Unrecognized typerror - src/parse.cpp, line 379" << endl;
            }
        }
        else cerr << "\e[31merror: \e[0mNo population data" << endl;
        
        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();

        if (shapes["features"][i]["geometry"]["type"] == "Polygon") {
            // vector parsed from coordinate string
            Geometry::Polygon geo = string_to_vector(coords);
            Geometry::LinearRing border = geo.hull;

            Geometry::Polygon shape(border, id);
            shape.pop = pop;
            shape.shape_id = id;

            for (int i = 0; i < geo.holes.size(); i++)
                shape.holes.push_back(geo.holes[i]);

            shapes_vector.push_back(shape);
        }
        else {
            Geometry::Multi_Polygon geo = multi_string_to_vector(coords);
            geo.shape_id = id;
            // calculate area of multipolygon
            double total_area = geo.get_area();

            // create many shapes with the same ID, add them to the array
            int append = 0;
            for (Geometry::Polygon s : geo.border) {
                Geometry::Polygon shape(s.hull, s.holes, id);
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


vector<Geometry::Multi_Polygon> parse_district_coordinates(string geoJSON) {
    /* 
        @desc: Parses a geoJSON file into an array of Multi_Polygon district objects
        @params: `string` geoJSON: geojson precincts to be parsed
        @return: `vector<Geometry::Multi_Polygon>` district objects with coordinate data
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Geometry::Multi_Polygon> shapes_vector;

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
            Geometry::Polygon border = string_to_vector(coords);
            Geometry::Multi_Polygon ms({border});
            shapes_vector.push_back(ms);
        }
        else {
            Geometry::Multi_Polygon borders = multi_string_to_vector(coords);
            shapes_vector.push_back(borders);
        }
    }

    return shapes_vector;
}


vector<Geometry::Precinct> merge_data( vector<Geometry::Polygon> precinct_shapes, map<string, vector<int> > voter_data) {
    /*
        @desc:
            returns an array of precinct objects given
            geodata (shape objects) and voter data
            in the form of a map for a list of precincts

        @params:
            `vector<Polygon>` precinct_shapes: coordinate data with id's
            `map<string, vector<int>>` voter_data: voting data with id's
    */

    vector<Geometry::Precinct> precincts;

    int x = 0;

    for (Geometry::Polygon precinct_shape : precinct_shapes) {
        // iterate over shapes array, get the id of the current shape
        string p_id = precinct_shape.shape_id;
        vector<int> p_data = {-1, -1}; // the voter data to be filled

        if ( voter_data.find(p_id) == voter_data.end() ) {
            // there is no matching id in the voter data
            cout << "error: the id in the geodata, \e[41m" << p_id << "\e[0m, has no matching key in voter_data" << endl;
            cout << "the program will continue, but the voter_data for the precinct will be filled with 0,0." << endl;
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

            int ratio = precinct_shape.get_area() / total_area;
            
            Geometry::Precinct precinct =
                Geometry::Precinct(precinct_shape.hull, precinct_shape.holes, p_data[0] * ratio, p_data[1] * ratio,
                                   p_id + "_s" + std::to_string(precinct_shape.is_part_of_multi_polygon));
            
            precinct.pop = precinct_shape.pop;
            precincts.push_back(precinct);
        }
        else {
            Geometry::Precinct precinct = 
                Geometry::Precinct(precinct_shape.hull, precinct_shape.holes, p_data[0], p_data[1], p_id);
            
            precinct.pop = precinct_shape.pop;
            precincts.push_back(precinct);
        }
        x++;
    }

    return precincts; // return precincts array
}


Geometry::Precinct_Group combine_holes(Geometry::Precinct_Group pg) {
    /*
        Takes a precinct group, iterates through precincts
        with holes, and combines internal precinct data to
        eliminate holes from the precinct group
    */

    vector<Geometry::Precinct> precincts;
    vector<Geometry::p_index> precincts_to_ignore;

    for (int x = 0; x < pg.precincts.size(); x++) {
        // for each precinct in the pg array
        Geometry::Precinct p = pg.precincts[x];

        // define starting precinct metadata
        Geometry::LinearRing precinct_border = p.hull;
        string id = p.shape_id;
        int demv = p.dem;
        int repv = p.rep;
        int pop = p.pop;

        if (p.holes.size() > 0) {
            // need to remove precinct holes
            int interior_pre = 0; // precincts inside the hole

            for (int j = 0; j < pg.precincts.size(); j++) {
                // check all other precincts for if they're inside
                Geometry::Precinct p_c = pg.precincts[j];
                if (j != x && get_inside(p_c.hull, p.hull)) {
                    // precinct j is inside precinct x,
                    // add the appropriate data from j to x

                    // Graphics::Canvas canvas(600,600);
                    // canvas.add_shape(p_c, false, Graphics::Color(255, 0, 0), 2);
                    // canvas.add_shape(p, false, Graphics::Color(0, 255, 0), 2);
                    // canvas.draw();

                    demv += pg.precincts[j].dem;
                    repv += pg.precincts[j].rep;
                    pop += pg.precincts[j].pop;

                    // this precinct will not be returned
                    precincts_to_ignore.push_back(j);
                    interior_pre++;
                }
            }
        }

        // create object from updated data
        Geometry::Precinct np = 
            Geometry::Precinct(precinct_border, demv, repv, pop, id);
        
        precincts.push_back(np);
    }

    vector<Geometry::Precinct> new_pre; // the new precinct array to return

    for (int i = 0; i < precincts.size(); i++) {
        if (!(std::find(precincts_to_ignore.begin(), precincts_to_ignore.end(), i) != precincts_to_ignore.end())) {
            // it is not a hole precinct, so add it
            new_pre.push_back(precincts[i]);
        }
    }

    return Geometry::Precinct_Group(new_pre);
}


vector<Geometry::p_index_set> sort_precincts(Geometry::Multi_Polygon shape, Geometry::Precinct_Group pg) {
    /*
        @desc:
            Takes an array of precincts and an exterior border array (islands), and
            determines which precincts go in which island. Returns array of precinct_index_list
            that correspond to precinct indices in the pg.precincts array

        @params:
            `Multi_Polygon` shape: The island shapes that will contain sorted precincts
            `Precinct_Group` pg: The precincts to be sorted

        @return: `p_index_set` array of precinct index sets that correspond to individual islands
    */

    vector<Geometry::p_index_set> islands;

    if (shape.border.size() > 1) {
        // has islands, need actually to sort
        vector<Geometry::Precinct> tmp_precincts = pg.precincts;
        Geometry::p_index_set ignore; // precincts that have already been sorted

        for (int i = 0; i < shape.border.size(); i++) {
            // for each island
            Geometry::p_index_set island;
            for (int j = 0; j < tmp_precincts.size(); j++) {
                // check whether each precinct is within the island
                if (!(std::find(ignore.begin(), ignore.end(), j) != ignore.end())) {
                    // not in the ignore list, must check;
                    if (get_inside_first(tmp_precincts[j].hull, shape.border[i].hull)) {
                        // the precinct is inside the island, add to the array
                        island.push_back(j);
                        ignore.push_back(j);
                    }
                }
            }
            
            // island is completely sorted
            islands.push_back(island);
        }
    }
    else {
        // only has one island, fill array with range(0, size_of_precincts)
        Geometry::p_index_set p(pg.precincts.size());
        std::iota(p.begin(), p.end(), 0);
        islands.push_back(p);
    }

    return islands;
}


Geometry::Graph generate_graph(Geometry::Precinct_Group pg) {
    /*
        @desc: Determines the network graph of a given precinct group
        @param: `Geometry::Precinct_Group` pg: precinct group to get graph of
        @return: `Graph` connection network
    */

    // assign all precincts to be nodes
    Geometry::Graph graph;

    for (int i = 0; i < pg.precincts.size(); i++) {
        Geometry::Node n(&pg.precincts[i]);
        n.id = i;
        n.precinct = &pg.precincts[i];
        graph.vertices.insert({n.id, n});
    }

    cout << "determining bounding boxes..." << endl;
    vector<Geometry::bounding_box> bounding_boxes;
    for (Geometry::Precinct p : pg.precincts) {
        bounding_boxes.push_back(p.get_bounding_box());
    }

    cout << "determining edges..." << endl;
    int edge = 0;
    for (int i = 0; i < pg.precincts.size(); i++) {
        Geometry::p_index_set precincts = {};
        cout << "on precinct " << i << endl;
        // check all unique combinations of
        // precincts with get_bordering

        for (int j = i + 1; j < pg.precincts.size(); j++) {
            if (bound_overlap(bounding_boxes[i], bounding_boxes[j])) {
                // check clip because bounding boxes overlap
                if (get_bordering(pg.precincts[i], pg.precincts[j])) {
                    graph.add_edge({j, i});
                    edge++;
                }
            }
        }

    //     for (Geometry::p_index border : precincts) {
    //         graph.add_edge({border, i});
    //     }
    }
    
    cout << "edge: " << edge << endl;
    cout << graph.edges.size() << ", " << graph.vertices.size() << endl;


    if (graph.get_num_components() > 1) {
        // the graph needs to be linked

        std::map<int, Geometry::coordinate> centers;        
        // determine all centers of precincts
        for (int i = 0; i < graph.vertices.size(); i++) {
            // add center of precinct to the map
            int key = (graph.vertices.begin() + i).key();
            Geometry::Node pre = (graph.vertices[key]);
            Geometry::coordinate center = (graph.vertices.begin() + i).value().precinct->get_center();
            centers.insert({key, center});
        }


        while (graph.get_num_components() > 1) {
            // add edges between two precincts on two islands
            // until `graph` is connected
            
            vector<Geometry::Graph> components = graph.get_components();
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
            cout << graph.vertices[me[0]].precinct->shape_id << ", " << graph.vertices[me[1]].precinct->shape_id << endl;
            // add link as edge to the graph
            graph.add_edge({me[0], me[1]});
        }
    } // else the graph is linked already

    return graph;
}



Geometry::State Geometry::State::generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON, vector<vector<string> > opts) {
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

    //! Should probably allocate memory with malloc
    //! Will be some outrageously large vectors here

    election_id_header = opts[0][0];
    geodata_id = opts[1][0];
    d_head = opts[2];
    r_head = opts[3];
    population_id = opts[4][0];

    // generate shapes from coordinates
    if (VERBOSE) cout << "generating coordinate arrays..." << endl;
    vector<Polygon> precinct_shapes = parse_precinct_coordinates(precinct_geoJSON);
    vector<Multi_Polygon> district_shapes = parse_district_coordinates(district_geoJSON);
    
    // get voter data from election data file
    if (VERBOSE) cout << "parsing voter data from tsv..." << endl;
    map<string, vector<int> > precinct_voter_data = parse_voter_data(voter_data);

    // create a vector of precinct objects from border and voter data
    if (VERBOSE) cout << "merging geodata with voter data into precincts..." << endl;
    vector<Precinct> precincts = merge_data(precinct_shapes, precinct_voter_data);
    
    // vector<
    // remove water precincts from data
    cout << "removing water precincts" << endl;
    for (int i = 0; i < precincts.size(); i++) {
        string id = precincts[i].shape_id;
        if (id.size() >= 6) {
            if (id.find("ZZZZZ") != string::npos) {
                cout << "removing precinct!" << endl;
                cout << id << endl;
                precincts.erase(precincts.begin() + i);
                i--;
            }
        }
    }

    Geometry::Precinct_Group pre_group(precincts);

    // remove holes from precinct data
    pre_group = combine_holes(pre_group);
    if (VERBOSE) cout << "removed hole precincts from precinct geodata" << endl;
    vector<Polygon> state_shape_v; // dummy exterior border

    // generate state data from files
    if (VERBOSE) cout << "generating state with precinct and district arrays..." << endl;
    State state = State(district_shapes, pre_group.precincts, state_shape_v);
    Multi_Polygon sborder = generate_exterior_border(state);
    state.border = sborder.border;

    // sort files into
    if (VERBOSE) cout << "sorting precincts into islands..." << endl << endl;
    // state.islands = sort_precincts(sborder, pre_group);
    state.network = generate_graph(pre_group);
    return state; // return the state object
}


Geometry::State Geometry::State::generate_from_file(string precinct_geoJSON, string district_geoJSON, vector<vector<string> > opts) {
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

    geodata_id = opts[0][0];
    d_head = opts[1];
    r_head = opts[2];
    population_id = opts[3][0];

    // generate shapes from coordinates
    if (VERBOSE) cout << "generating coordinate array from precinct file..." << endl;
    vector<Precinct> precinct_shapes = parse_precinct_data(precinct_geoJSON);
    if (VERBOSE) cout << "generating coordinate array from district file..." << endl;
    vector<Multi_Polygon> district_shapes = parse_district_coordinates(district_geoJSON);

    for (int i = 0; i < precinct_shapes.size(); i++) {
        string id = precinct_shapes[i].shape_id;
        if (id.size() >= 6) {
            if (id.find("ZZZZZ") != string::npos) {
                cout << "removing precinct!" << endl;
                precinct_shapes.erase(precinct_shapes.begin() + i);
                i--;
            }
        }
    }

    // create a vector of precinct objects from border and voter data
    Precinct_Group pre_group(precinct_shapes);

    // remove holes from precinct data
    if (VERBOSE) cout << "combining holes in precinct geodata..." << endl;
    pre_group = combine_holes(pre_group);

    vector<Polygon> state_shape_v;  // dummy exterior border

    // generate state data from files
    if (VERBOSE) cout << "generating state with precinct and district arrays..." << endl;
    State state = State(district_shapes, pre_group.precincts, state_shape_v);
    Multi_Polygon sborder = generate_exterior_border(state);
    state.border = sborder.border;

    state.network = generate_graph(pre_group);
    if (VERBOSE) cout << "state serialized!" << endl;

    return state; // return the state object
}
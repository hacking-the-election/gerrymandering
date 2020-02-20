/*=======================================
 parse.cpp:                     k-vernooy
 last modified:               Web, Feb 19

 Definitions of state methods for parsing
 from geodata and election data (see data
 specs in root directory for information)
========================================*/

#include "../include/shape.hpp"       // class definitions
#include "../include/util.hpp"        // array modification functions
#include "../include/geometry.hpp"    // exterior border generation
#include <algorithm>                  // for std::find and std::distance

#define VERBOSE 1  // print progress messages
using namespace rapidjson;

// constant id strings
//ndv	nrv	geoid10	GEOID10	POP100
const std::string election_id_header = "geoid10";
const std::vector<std::string> d_head = {"ndv"};
const std::vector<std::string> r_head = {"nrv"};
const std::string geodata_id = "GEOID10";
const std::string population_id = "POP100";

std::vector<std::vector<std::string > > parse_sv(std::string, std::string);
bool check_column(std::vector<std::vector<std::string> >, int);

std::vector<std::vector<std::string> > parse_sv(std::string tsv, std::string delimiter) {
    
    /*
        takes a tsv file as string, returns
        two dimensional array of cells and rows
    */

    std::stringstream file(tsv);
    std::string line;
    std::vector<std::vector<std::string> > data;

    while (getline(file, line)) {
        std::vector<std::string> row;
        size_t pos = 0;

        while ((pos = line.find(delimiter)) != std::string::npos) {
            row.push_back(line.substr(0, pos));
            line.erase(0, pos + delimiter.length());
        }

        row.push_back(line);
        data.push_back(row);
    }

    return data;
}

bool check_column(std::vector<std::vector<std::string> > data_list, int index) {
   
    /*
        returns whether or not a given column in a two
        dimensional vector is empty at any given point
    */

    for (int i = 0; i < data_list.size(); i++)
        if (data_list[i][index].size() == 0)
            return false; // the column is empty at this cell

    return true;
}

std::map<std::string, std::vector<int> > parse_voter_data(std::string voter_data) {

    /*
        from a string in the specified format,
        creates a map with the key of the precinct
        name and vector as `"name": {dem_vote, rep vote}`
    */

    std::vector<std::vector<std::string> > data_list // two dimensional
        = parse_sv(voter_data, "\t"); // array of voter data

    int precinct_id_col = -1; // the column index that holds precinct id's
    // indices of usable data
    std::vector<int> d_index;
    std::vector<int> r_index;

    for ( int i = 0; i < data_list[0].size(); i++) {
        std::string val = data_list[0][i];

        if (val == election_id_header)
            precinct_id_col = i;

        for (std::string head : d_head) {
            if (head == val) {
                d_index.push_back(i);
            }
        }

        for (std::string head : r_head) {
            if (head == val) {
                r_index.push_back(i);
            }
        }
    }

    std::map<std::string, std::vector<int> > parsed_data;

    // iterate over each precinct
    for (int x = 1; x < data_list.size(); x++) {
        std::string id;

        // remove quotes from string
        if (data_list[x][precinct_id_col].substr(0, 1) == "\"")
            id = split(data_list[x][precinct_id_col], "\"")[1];
        else
            id = data_list[x][precinct_id_col];
                
        int demT = 0, repT = 0;

        // get the right voter columns, add to party total
        for (int i = 0; i < d_index.size(); i++) {
            std::string d = data_list[x][d_index[i]];
            std::string r = data_list[x][r_index[i]];

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

GeoGerry::Shape string_to_vector(std::string str) {
    // takes a json array string and returns a parsed vector

    GeoGerry::Shape v;
    Document mp;
    mp.Parse(str.c_str());

    GeoGerry::LinearRing hull;
    for (int i = 0; i < mp[0].Size(); i++)
        hull.border.push_back({mp[0][i][0].GetDouble(), mp[0][i][1].GetDouble()});

    v.hull = hull;

    for (int i = 1; i < mp.Size(); i++) {
        std::cout << "hole" << std::endl;
        GeoGerry::LinearRing hole;
        for (int j = 0; j < mp[i].Size(); j++)
            hole.border.push_back({mp[i][j][0].GetDouble(), mp[i][j][0].GetDouble()});

        v.holes.push_back(hole);
    }

    return v;
}

GeoGerry::Multi_Shape multi_string_to_vector(std::string str) {
    // takes a json array string and returns a parsed vector
    
    GeoGerry::Multi_Shape v;

    Document mp;
    mp.Parse(str.c_str());

    for (int i = 0; i < mp.Size(); i++) {
        // for each polygon
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);
        mp[i].Accept(writer);

        GeoGerry::Shape polygon = string_to_vector(buffer.GetString());
        v.border.push_back(polygon);
    }

    return v;
}

std::vector<GeoGerry::Shape> parse_precinct_coordinates(std::string geoJSON) {
    /* 
        Parses a geoJSON file into an array of Shape
        objects - finds ID using top-level defined constants,
        and splits multipolygons into separate shapes (of the same id)
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    std::vector<GeoGerry::Shape> shapes_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        std::string coords;
        std::string id = "";
        int pop = 0;

        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember(geodata_id.c_str())) {
            id = shapes["features"][i]["properties"][geodata_id.c_str()].GetString();
        }
        else {
            std::cout << "\e[31merror: \e[0mYou have no precinct id." << std::endl;
            std::cout << "If future k-vernooy runs into this error, it means that GEOID10 in your geoJSON in your voter data is missing. To fix... maybe try a loose comparison of the names?" << std::endl;
        }

        // get the population from geodata
        if (shapes["features"][i]["properties"].HasMember(population_id.c_str()))
            pop = shapes["features"][i]["properties"][population_id.c_str()].GetInt();
        else
            std::cout << "\e[31merror: \e[0mNo population data" << std::endl;
        
        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();

        if (shapes["features"][i]["geometry"]["type"] == "Polygon") {
            // vector parsed from coordinate string
            GeoGerry::Shape geo = string_to_vector(coords);
            GeoGerry::LinearRing border = geo.hull;

            GeoGerry::Shape shape(border, id);
            shape.pop = pop;

            for (int i = 0; i < geo.holes.size(); i++) {
                shape.holes.push_back(geo.holes[i]);
            }

            shapes_vector.push_back(shape);
        }
        else {
            GeoGerry::Multi_Shape geo = multi_string_to_vector(coords);

            // calculate area of multipolygon
            float total_area = geo.get_area();

            // create many shapes with the same ID, add them to the array
            for (GeoGerry::Shape s : geo.border) {
                GeoGerry::Shape shape(s.hull, s.holes, id);
                shape.is_part_of_multi_polygon = true;
                float fract = shape.get_area() / total_area;
                shape.pop = (int) round(pop * fract);
                shapes_vector.push_back(shape);
            }
        }
    }

    return shapes_vector;
}


std::vector<GeoGerry::Multi_Shape> parse_district_coordinates(std::string geoJSON) {
    /* 
        Parses a geoJSON file into an array of Multi_Shape
        objects - finds ID using top-level defined constants
    */

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    std::vector<GeoGerry::Multi_Shape> shapes_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        std::string coords;
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
            GeoGerry::Shape border = string_to_vector(coords);
            GeoGerry::Multi_Shape ms({border});
            shapes_vector.push_back(ms);
        }
        else {
            GeoGerry::Multi_Shape borders = multi_string_to_vector(coords);
            shapes_vector.push_back(borders);
        }
    }

    // std::cout << mptotal << " multipolygons found!" << std::endl;
    return shapes_vector;
}

std::vector<GeoGerry::Precinct> merge_data( std::vector<GeoGerry::Shape> precinct_shapes, std::map<std::string, std::vector<int> > voter_data) {

    /*
        returns an array of precinct objects given
        geodata (shape objects) and voter data
        in the form of a map for a list of precincts
    */

    std::vector<GeoGerry::Precinct> precincts;

    int x = 0;

    for (GeoGerry::Shape precinct_shape : precinct_shapes) {
        // iterate over shapes array, get the id of the current shape
        std::string p_id = precinct_shape.shape_id;
        std::vector<int> p_data = {-1, -1}; // the voter data to be filled

        if ( voter_data.find(p_id) == voter_data.end() ) {
            // there is no matching id in the voter data
            std::cout << "error: the id in the geodata, \e[41m" << p_id << "\e[0m, has no matching key in voter_data" << std::endl;
            std::cout << "the program will continue, but the voter_data for the precinct will be filled with 0,0." << std::endl;
        }
        else {
            // get the voter data of the precinct
            p_data = voter_data[p_id];
        }

        // create a precinct object and add it to the array
        if (precinct_shape.is_part_of_multi_polygon) {
            float total_area = precinct_shape.get_area();

            for (int i = 0; i < precinct_shapes.size(); i++) {
                if (i != x && precinct_shapes[i].shape_id == p_id) {
                    total_area += precinct_shapes[i].get_area();
                }
            }

            int ratio = precinct_shape.get_area() / total_area;
            
            GeoGerry::Precinct precinct =
                GeoGerry::Precinct(precinct_shape.hull, precinct_shape.holes, p_data[0] * ratio, p_data[1] * ratio, p_id);
            
            precinct.pop = precinct_shape.pop;
            precincts.push_back(precinct);
        }
        else {
            GeoGerry::Precinct precinct = 
                GeoGerry::Precinct(precinct_shape.hull, precinct_shape.holes, p_data[0], p_data[1], p_id);
            
            precinct.pop = precinct_shape.pop;
            precincts.push_back(precinct);
        }
        x++;
    }

    return precincts; // return precincts array
}

GeoGerry::Precinct_Group combine_holes(GeoGerry::Precinct_Group pg) {
    /*
        Takes a precinct group, iterates through precincts
        with holes, and combines internal precinct data to
        eliminate holes from the precinct group
    */

    std::vector<GeoGerry::Precinct> precincts;

    int x = 0;

    while (x < pg.precincts.size()) {
        // get precinct object by index
        GeoGerry::Precinct p = pg.precincts[x];
        // define or declare precinct metadata
        GeoGerry::LinearRing precinct_border = p.hull;
        int demv = 0, repv = 0, pop = 0;
        std::string id = p.shape_id;

        if (p.holes.size() > 0) {
            std::vector<GeoGerry::p_index> precincts_to_combine;

            int i = 0; // index of precinct to check

            for (GeoGerry::Precinct p_c : pg.precincts) {
                if (p_c != p) { // avoid checking same precinct
                    for (GeoGerry::LinearRing hole : p.holes){
                        if (get_inside(p_c.hull, hole)) {
                            std::cout << "precinct inside" << std::endl;
                            precincts_to_combine.push_back(i);
                        }
                    }
                }

                i++;
            }

            for (GeoGerry::p_index pi : precincts_to_combine) {
                demv += pg.precincts[pi].dem;
                repv += pg.precincts[pi].rep;
                pop += pg.precincts[pi].pop;
                pg.precincts.erase(pg.precincts.begin() + pi);
            }
        }
        else {
            demv = p.dem;
            repv = p.rep;
            pop = p.pop;
        }

        GeoGerry::Precinct np = GeoGerry::Precinct(precinct_border, demv, repv, pop, id);
        precincts.push_back(np);

        x++;
    }

    return GeoGerry::Precinct_Group(precincts);
}

GeoGerry::State GeoGerry::State::generate_from_file(std::string precinct_geoJSON, std::string voter_data, std::string district_geoJSON) {
    /*
        Parse precinct and district geojson, along with
        precinct voter data, into a State object.
    */

    //! Should probably allocate memory with malloc
    //! Will be some outrageously large vectors here

    // generate shapes from coordinates
    if (VERBOSE) std::cout << "generating coordinate array from precinct file..." << std::endl;
    std::vector<Shape> precinct_shapes = parse_precinct_coordinates(precinct_geoJSON);

    if (VERBOSE) std::cout << "generating coordinate array from district file..." << std::endl;
    std::vector<Multi_Shape> district_shapes = parse_district_coordinates(district_geoJSON);
    
    // create a vector of precinct objects from border and voter data
    if (VERBOSE) std::cout << "parsing voter data from tsv..." << std::endl;
    std::map<std::string, std::vector<int> > precinct_voter_data = parse_voter_data(voter_data);

    if (VERBOSE) std::cout << "merging parsed geodata with parsed voter data into precinct array..." << std::endl;
    std::vector<Precinct> precincts = merge_data(precinct_shapes, precinct_voter_data);
    
    Precinct_Group pre_group(precincts);
    std::cout << pre_group.precincts.size() << std::endl;
    pre_group = combine_holes(pre_group);

    std::vector<Shape> state_shape_v; // dummy exterior border
    std::cout << pre_group.precincts.size() << std::endl;
    // generate state data from files
    if (VERBOSE) std::cout << "generating state with shape arrays..." << std::endl;
    State state = State(district_shapes, pre_group.precincts, state_shape_v);
    
    // state.draw();

    Multi_Shape border = generate_exterior_border(state);
    // border.draw();

    return state; // return the state object
}
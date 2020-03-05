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
#include <numeric>                    // include std::iota
#include <iomanip>

#define VERBOSE 1  // print progress messages
using namespace rapidjson;

const long int c = pow(2, 18);

// constant id strings
//ndv	nrv	geoid10	GEOID10	POP100
const std::string election_id_header = "geoid10";
const std::vector<std::string> d_head = {"PRS08DEM"};
const std::vector<std::string> r_head = {"PRS08REP"};
const std::string geodata_id = "GEOID10";
const std::string population_id = "TOTPOP";

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
        GeoGerry::LinearRing hole;
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

            for (int i = 0; i < geo.holes.size(); i++)
                shape.holes.push_back(geo.holes[i]);

            shapes_vector.push_back(shape);
        }
        else {
            GeoGerry::Multi_Shape geo = multi_string_to_vector(coords);

            // calculate area of multipolygon
            double total_area = geo.get_area();

            // create many shapes with the same ID, add them to the array
            int append = 0;
            for (GeoGerry::Shape s : geo.border) {
                GeoGerry::Shape shape(s.hull, s.holes, id + "_s" + std::to_string(append));
                shape.is_part_of_multi_polygon = true;
                double fract = shape.get_area() / total_area;
                shape.pop = (int) round(pop * fract);
                shapes_vector.push_back(shape);
                append++;
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
            double total_area = precinct_shape.get_area();

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
    std::vector<GeoGerry::p_index> precincts_to_ignore;

    for (int x = 0; x < pg.precincts.size(); x++) {
        // for each precinct in the pg array
        GeoGerry::Precinct p = pg.precincts[x];

        // define starting precinct metadata
        GeoGerry::LinearRing precinct_border = p.hull;
        std::string id = p.shape_id;
        int demv = p.dem;
        int repv = p.rep;
        int pop = p.pop;

        if (p.holes.size() > 0) {
            // need to remove precinct holes
            int interior_pre = 0; // precincts inside the hole

            for (int j = 0; j < pg.precincts.size(); j++) {
                // check all other precincts for if they're inside
                GeoGerry::Precinct p_c = pg.precincts[j];
                if (j != x && get_inside(p_c.hull, p.hull)) {
                    // precinct j is inside precinct x,
                    // add the appropriate data from j to x
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
        GeoGerry::Precinct np = 
            GeoGerry::Precinct(precinct_border, demv, repv, pop, id);
        
        precincts.push_back(np);
    }

    std::vector<GeoGerry::Precinct> new_pre; // the new precinct array to return

    for (int i = 0; i < precincts.size(); i++) {
        if (!(std::find(precincts_to_ignore.begin(), precincts_to_ignore.end(), i) != precincts_to_ignore.end())) {
            // it is not a hole precinct, so add it
            new_pre.push_back(precincts[i]);
        }
    }

    return GeoGerry::Precinct_Group(new_pre);
}

std::vector<GeoGerry::p_index_set> sort_precincts(GeoGerry::Multi_Shape shape, GeoGerry::Precinct_Group pg) {
    /*
        @desc:
            Takes an array of precincts and an exterior border array (islands), and
            determines which precincts go in which island. Returns array of precinct_index_list
            that correspond to precinct indices in the pg.precincts array

        @params:
            `Multi_Shape` shape: The island shapes that will contain sorted precincts
            `Precinct_Group` pg: The precincts to be sorted

        @return: `p_index_set` array of precinct index sets that correspond to individual islands
    */

    std::vector<GeoGerry::p_index_set> islands;

    if (shape.border.size() > 1) {
        // has islands, need actually to sort
        std::vector<GeoGerry::Precinct> tmp_precincts = pg.precincts;
        GeoGerry::p_index_set ignore; // precincts that have already been sorted

        for (int i = 0; i < shape.border.size(); i++) {
            // for each island
            GeoGerry::p_index_set island;
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
        GeoGerry::p_index_set p(pg.precincts.size());
        std::iota(p.begin(), p.end(), 0);
        islands.push_back(p);
    }

    return islands;
}

int hole_count(GeoGerry::Precinct_Group pg) {
    /*
        @desc: counts sum of holes in a given precinct group
        @params: `Precinct_Group` pg: Precincts to count holes of
        @return: `int` number of holes in precincts
    */

    int sum = 0;
    for (GeoGerry::Precinct p : pg.precincts )
        // add number of holes for each precinct
        sum += p.holes.size();
    
    return sum;
}

GeoGerry::State GeoGerry::State::generate_from_file(std::string precinct_geoJSON, std::string voter_data, std::string district_geoJSON) {
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

    // generate shapes from coordinates
    if (VERBOSE) std::cout << "generating coordinate array from precinct file..." << std::endl;
    std::vector<Shape> precinct_shapes = parse_precinct_coordinates(precinct_geoJSON);
    if (VERBOSE) std::cout << "generating coordinate array from district file..." << std::endl;
    std::vector<Multi_Shape> district_shapes = parse_district_coordinates(district_geoJSON);
    
    // get voter data from election data file
    if (VERBOSE) std::cout << "parsing voter data from tsv..." << std::endl;
    std::map<std::string, std::vector<int> > precinct_voter_data = parse_voter_data(voter_data);

    // create a vector of precinct objects from border and voter data
    if (VERBOSE) std::cout << "merging parsed geodata with parsed voter data into precinct array..." << std::endl;
    std::vector<Precinct> precincts = merge_data(precinct_shapes, precinct_voter_data);
    Precinct_Group pre_group(precincts);

    // remove holes from precinct data
    int before = pre_group.precincts.size();
    if (VERBOSE) std::cout << "combining holes in precinct geodata..." << std::endl;
    pre_group = combine_holes(pre_group);
    int removed = before - pre_group.precincts.size();
    if (VERBOSE) std::cout << "removed " << removed << " hole precincts from precinct geodata" << std::endl;

    std::vector<Shape> state_shape_v; // dummy exterior border

    // generate state data from files
    if (VERBOSE) std::cout << "generating state with precinct and district arrays..." << std::endl;
    State state = State(district_shapes, pre_group.precincts, state_shape_v);
    Multi_Shape border = generate_exterior_border(state);
    state.border = border.border;

    // sort files into
    if (VERBOSE) std::cout << "sorting precincts into islands from exterior state border..." << std::endl;
    state.islands = sort_precincts(border, pre_group);
    if (VERBOSE) std::cout << "state serialized!" << std::endl;
    writef(state.to_json(), "test.json");
    return state; // return the state object
}
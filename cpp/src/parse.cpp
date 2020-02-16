/*=======================================
 parse.cpp:                     k-vernooy
 last modified:               Sun, Jan 20

 Definitions of state methods for parsing
 from geodata and election data (see data
 specs in root directory for information)
========================================*/

#include "../include/shape.hpp"    // class definitions
#include "../include/util.hpp"     // array modification functions
#include "../include/geometry.hpp" // exterior border generation

#include <algorithm>             // for std::find and std::distance

#define VERBOSE 1  // print progress messages

// constant id strings
//ndv	nrv	geoid10	GEOID10	POP100
const string election_id_header = "geoid10";
const vector<string> d_head = {"ndv"};
const vector<string> r_head = {"nrv"};
const string geodata_id = "GEOID10";
const string population_id = "POP100";


vector<vector<string> > parse_sv(string tsv, string delimiter) {
    
    /*
        takes a tsv file as string, returns
        two dimensional array of cells and rows
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
        returns whether or not a given column in a two
        dimensional vector is empty at any given point
    */

    for (int i = 0; i < data_list.size(); i++)
        if (data_list[i][index].size() == 0)
            return false; // the column is empty at this cell

    return true;
}

map<string, vector<int> > parse_voter_data(string voter_data) {

    /*
        from a string in the specified format,
        creates a map with the key of the precinct
        name and vector as `"name": {dem_vote, rep vote}`
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
        string id = split(data_list[x][precinct_id_col], "\"")[1]; // remove quotes
        // string id = data_list[x][precinct_id_col];
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

vector<vector<float> > string_to_vector(string str) {
    // takes a json array string and returns a parsed vector

    // remove instances of "[" or "]" from the string
    str.erase(remove(str.begin(), str.end(), '['), str.end());
    str.erase(remove(str.begin(), str.end(), ']'), str.end());

    vector<string> list = split(str, ","); // split string by commas
    vector<vector<float> > v;

    // loop through comma split vector
    int i = 1;
    for (int i = 0; i < list.size(); i += 2) {
        // add as pair of floating point coordinates
        v.push_back( { stof(list[i]), stof(list[i + 1]) } );
    }

    return v;
}

vector<Shape> parse_coordinates(string geoJSON) {

    // parses a geoJSON state into an array of shapes

    Document shapes;
    shapes.Parse(geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Shape> shapes_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        string coords;
        string id = "";
        int pop = 0;
        // see if the geoJSON contains the shape id
        if (shapes["features"][i]["properties"].HasMember(geodata_id.c_str())) {
            id = shapes["features"][i]["properties"][geodata_id.c_str()].GetString();
            // id = id.substr(1, id.size());
        }
        else {
            cout << "If this is parsing precincts, you have no precinct id." << endl;
            cout << "If future k-vernooy runs into this error, it means that GEOID10 in your geoJSON in your voter data is missing. To fix... maybe try a loose comparison of the names?" << endl;
        }

        // get the population from geodata
        if (shapes["features"][i]["properties"].HasMember(population_id.c_str())) {
            pop = shapes["features"][i]["properties"][population_id.c_str()].GetInt();
            // if (tmp != "" && tmp != "NA") {
            //     pop = stoi(tmp);
            // }
        }
        else
            cout << "\e[31merror: \e[0mNo population data" << endl;
        
        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();

        // vector parsed from coordinate string
        vector<vector<float> > border = string_to_vector(coords);

        // create shape from border, add to array
        Shape shape(border, id);
        shape.pop = pop;
        shapes_vector.push_back(shape);
    }

    return shapes_vector;
}


vector<Precinct> merge_data( vector<Shape> precinct_shapes, map<string, vector<int> > voter_data) {

    /*
        returns an array of precinct objects given
        geodata (shape objects) and voter data
        in the form of a map for a list of precincts
    */

    vector<Precinct> precincts;

    for (Shape precinct_shape : precinct_shapes) {
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
        Precinct precinct = Precinct(precinct_shape.border, p_data[0], p_data[1], p_id);
        precinct.pop = precinct_shape.pop;
        precincts.push_back(precinct);
    }

    return precincts; // return precincts array
}

vector<Precinct> parse_voter_and_geodata(string precinct_geoJSON) {

    // parses a geoJSON state into an array of precincts with election data

    Document shapes;
    shapes.Parse(precinct_geoJSON.c_str()); // parse json

    // vector of shapes to be returned
    vector<Precinct> p_vector;

    for ( int i = 0; i < shapes["features"].Size(); i++ ) {
        
        string coords;
        int pop = 0, demT = 0, repT = 0;

        if (shapes["features"][i]["properties"].HasMember(population_id.c_str()))
            pop = shapes["features"][i]["properties"][population_id.c_str()].GetFloat();
        else
            cout << "\e[31merror: \e[0mNo population data" << endl;
        

        for (string head : d_head) {
            if (shapes["features"][i]["properties"].HasMember(head.c_str())) {
                if (shapes["features"][i]["properties"][head.c_str()].IsInt())
                    demT += shapes["features"][i]["properties"][head.c_str()].GetInt();
                else if (shapes["features"][i]["properties"][head.c_str()].IsFloat())
                    demT += shapes["features"][i]["properties"][head.c_str()].GetFloat();
                else if (shapes["features"][i]["properties"][head.c_str()].IsString())
                    demT += stoi(shapes["features"][i]["properties"][head.c_str()].GetString());
            }
            else 
                cout << "FAIL" << endl;
        }


        for (string head : r_head) {
            if (shapes["features"][i]["properties"].HasMember(head.c_str())) {
                if (shapes["features"][i]["properties"][head.c_str()].IsInt())
                    repT += shapes["features"][i]["properties"][head.c_str()].GetInt();
                else if (shapes["features"][i]["properties"][head.c_str()].IsFloat())
                    repT += shapes["features"][i]["properties"][head.c_str()].GetFloat();
                else if (shapes["features"][i]["properties"][head.c_str()].IsString())
                    repT += stoi(shapes["features"][i]["properties"][head.c_str()].GetString());
            }
            else 
                cout << "FAIL" << endl;
        }


        // create empty string buffer
        StringBuffer buffer;
        buffer.Clear();
        Writer<rapidjson::StringBuffer> writer(buffer);

        // write the coordinate array to a string
        shapes["features"][i]["geometry"]["coordinates"].Accept(writer);
        coords = buffer.GetString();

        // vector parsed from coordinate string
        vector<vector<float> > border = string_to_vector(coords);

        // create shape from border, add to array
        Precinct p(border, demT, repT);
        p.pop = pop;
        p_vector.push_back(p);
    }

    return p_vector;
};

State State::generate_from_file(string precinct_geoJSON, string voter_data, string district_geoJSON) {

    /*
        Parse precinct and district geojson, along with
        precinct voter data, into a State object.
    */

    //! Should probably allocate memory with malloc
    //! Will be some outrageously large vectors here

    // generate shapes from coordinates
    if (VERBOSE) cout << "generating coordinate array from precinct file..." << endl;
    vector<Shape> precinct_shapes = parse_coordinates(precinct_geoJSON);

    if (VERBOSE) cout << "generating coordinate array from district file..." << endl;
    vector<Shape> district_shapes = parse_coordinates(district_geoJSON);
    
    // create a vector of precinct objects from border and voter data
    if (VERBOSE) cout << "parsing voter data from tsv..." << endl;
    map<string, vector<int> > precinct_voter_data = parse_voter_data(voter_data);

    if (VERBOSE) cout << "merging parsed geodata with parsed voter data into precinct array..." << endl;
    vector<Precinct> precincts = merge_data(precinct_shapes, precinct_voter_data);
    Precinct_Group pre_group(precincts);
    // Multi_Shape state_shape = generate_exterior_border(pre_group);

    vector<Shape> state_shape_v; // dummy exterior border
    
    // for (Shape s : state_shape.border) {
    //     state_shape_v.push_back(s);
    // }

    // generate state data from files
    if (VERBOSE) cout << "generating state with shape arrays..." << endl;
    State state = State(district_shapes, precincts, state_shape_v);
    return state; // return the state object
}
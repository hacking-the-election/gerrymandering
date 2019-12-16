#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "../../lib/rapidjson/include/rapidjson/document.h"
#include "../../lib/rapidjson/include/rapidjson/writer.h"
#include "../../lib/rapidjson/include/rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

class Precinct {

    Precinct(vector<int> coordinates, int demV, int repV);

    int dem;
    int rep;
    double dratio;
    vector<int> precinct_border;

    public: 
        double getRatio();
};

class District {
    District(vector<Precinct> pre);

    vector<int> district_border;
    int id;
};

class State {
    State(vector<District> dists, vector<Precinct> pres, vector<int> boundary);

    vector<int> state_border;
    vector<District> state_districts;
    vector<Precinct> state_precincts;

    void serialize_obj();
    void deserialize_obj();

    int id;
    string name;
};
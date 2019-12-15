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

    Precinct(string coordinates);

    int dem;
    int rep;
    double ratio;
    vector<int> border;

    void write_to_file();
    void read_from_file();

    public: 
        double getRatio();
};

class District {
    District(vector<Precinct> pre);

    vector<int> border;
    vector<Precinct> precincts;
    int id;
};

class State {
    State(vector<District> dists );

    vector<int> border;
    vector<District> districts;
    int id;
    string name;
};
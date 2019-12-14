#include "../include/gerry.hpp"

using namespace rapidjson;
using namespace std;

int main(int argc, char* argv[]) {

    ifstream t(argv[1]);
    stringstream buffer;
    buffer << t.rdbuf();

    Document json;
    json.Parse(buffer.str().c_str());

    Value& s = json["features"][0]["geometry"]["coords"];
    s.SetInt(s.GetInt() + 1);

    StringBuffer jsbuffer;
    Writer<StringBuffer> writer(jsbuffer);
    json.Accept(writer);

    cout << jsbuffer.GetString() << endl;


//     // 2. Modify it by DOM.
//     Value& s = d["features"];
//     s.SetInt(s.GetInt() + 1);

// // [features][geometry][coords]

    return 0;
}
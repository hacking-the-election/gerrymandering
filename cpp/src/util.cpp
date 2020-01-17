// functiions for simple utilities
#include "../include/util.hpp"
using namespace std;

string readf(string path) {
    ifstream f(path);
    stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
};

vector<string> split(string str, string del) {
    vector<string> array;

    int pos = 0;
    string token;

    while ((pos = str.find(del)) != string::npos) {
        token = str.substr(0, pos);
        array.push_back(token);
        str.erase(0, pos + del.length());
    }

    array.push_back(str);
    return array;
}


string join(vector<string> str, string del) {
    string ret;

    for (int i = 0; i < str.size() - 1; i++) {
        ret += str[i] + del;
    }

    ret += str[str.size() - 1];
    return ret;
}
// functiions for simple utilities
#include "../include/util.hpp"
using namespace std;

string readf(string path) {
    ifstream f(path);
    stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
};
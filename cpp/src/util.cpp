// functiions for simple utilities
#include <sstream>
#include <string>
#include <fstream>

using namespace std;

string readf(string path) {
    ifstream f(path);
    stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
};
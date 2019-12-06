#include "../include/gerry.hpp"
using namespace std;

int main(int argc, char* argv[]) {

    ifstream t(argv[1]);
    stringstream buffer;
    buffer << t.rdbuf();

    json data = json::parse(buffer.str());

    std::cout << data.dump(2) << std::endl;

    return 0;
}
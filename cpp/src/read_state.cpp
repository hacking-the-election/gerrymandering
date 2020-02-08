#include <string>
#include "../include/shape.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    string path = string(argv[1]);
    cout << "Reading state..." << endl;
    State state = State::read_binary(path);
    cout << "Read state from " << path << endl;
    state.draw();
}
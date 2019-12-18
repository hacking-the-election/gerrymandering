#include "../include/shape.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

int main(int argc, char* argv[]) {
    vector<vector<int> > shape = { {0, 0}, {0,1}, {1,0} };

    cout << area(shape);
    cout << center(shape)[0] << ", " << center(shape)[0];
    
    // vector<int> test = { {1,3}, {3,6} };
}
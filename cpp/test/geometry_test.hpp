/*=======================================
 geometry_test.hpp:             k-vernooy
 last modified:                Sun, Feb 9
 
 Class declaration and method definitions
 for a Geometry_Test class. For checking
 methods defined in ../src/geometry.cpp
========================================*/

#include <iostream>

using namespace std;

class Geometry_Test {
    public:
        static void main(); // run main tests
        void assert_equal(); //! Prime case for templating
        // void test_point_in_poly();
};

void Geometry_Test::main() {
    cout << "Testing..." << endl;
}
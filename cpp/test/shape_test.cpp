/*=======================================
 shape_test.cpp:                k-vernooy
 last modified:                Sun, Feb 9
 
 A driver program for testing methods in
 the shape classes (state, precinct, etc)
========================================*/

#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/term_disp.hpp"

using namespace std;
using namespace GeoGerry;

void assert_equal(string context, float n1, float n2) {
    if ( n1 != n2 )
        cout << RED << "error " << RESET << "in " << context << ", " << n1 << " != " << n2;
    else
        cout << GREEN << "passed " << RESET << context << ", " << n1 << " = " << n2;
    cout << endl;
}

int main(int argc, char* argv[]) {
    coordinate_set border = {{70.4883, 70.4883}, {57.6562, 57.6562}, {77.168, 77.168}, {84.1992, 84.1992}, {73.125, 73.125}, {70.4883, 70.4883}};
    Shape shape = Shape(LinearRing(border));
    shape.draw();
    cout << point_in_ring({70.4883, 59.5343}, shape.hull) << endl;

    shape.draw();

    // assert_equal("checking area function", area(shape), 0.5);
    // assert_equal("testing center coord 1", center(shape)[0], 0.333333);
    // assert_equal("testing center coord 2", center(shape)[1], 0.333333);
}
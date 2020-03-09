/*=======================================
 shape_test.cpp:                k-vernooy
 last modified:                Sun, Feb 9
 
 A driver program for testing methods in
 the shape classes (state, precinct, etc)
========================================*/

// #include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
// #include "../include/term_disp.hpp"

using namespace std;
// using namespace GeoGerry;

// void assert_equal(string context, float n1, float n2) {
//     if ( n1 != n2 )
//         cout << RED << "error " << RESET << "in " << context << ", " << n1 << " != " << n2;
//     else
//         cout << GREEN << "passed " << RESET << context << ", " << n1 << " = " << n2;
//     cout << endl;
// }

int main(int argc, char* argv[]) {
    GeoDraw::Canvas c(640, 900);
    
    GeoGerry::coordinate_set border = {{0,0}, {1,2}, {2,1}, {3,3}};
    GeoGerry::LinearRing ring(border);
    GeoGerry::Shape shape(ring);
    GeoDraw::Color co(0, 108, 180);  
    c.add_shape(shape, true, co, 1);
    c.draw();
    // assert_equal("checking area function", area(shape), 0.5);
    // assert_equal("testing center coord 1", center(shape)[0], 0.333333);
    // assert_equal("testing center coord 2", center(shape)[1], 0.333333);
}
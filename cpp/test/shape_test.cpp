/*=======================================
 shape_test.cpp:                k-vernooy
 last modified:                Sun, Feb 9
 
 A driver program for testing methods in
 the shape classes (state, precinct, etc)
========================================*/

// #include "../include/shape.hpp"
#include "../include/geometry.hpp"
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
    GeoGerry::coordinate_set border = {{-172.759572, 60.257714},{-172.773586, 60.255490},{-172.789645, 60.250904},{-172.796941, 60.248821},{-172.815625, 60.239928},{-172.834310, 60.233258},{-172.867009, 60.222142},{-172.885693, 60.208803},{-172.890361, 60.182124},{-172.881018, 60.168785},{-172.856434, 60.153742},{-172.848320, 60.148778},{-172.829635, 60.142109},{-172.799861, 60.139197},{-172.799797, 60.139118},{-172.799605, 60.138882},{-172.799541, 60.138804},{-172.761788, 60.140677},{-172.738268, 60.145225},{-172.720868, 60.148590},{-172.694193, 60.154610},{-172.680662, 60.159910},{-172.655702, 60.178484},{-172.638121, 60.195838},{-172.634324, 60.200743},{-172.637485, 60.219294},{-172.649219, 60.236895},{-172.670111, 60.248026},{-172.673410, 60.249784},{-172.698574, 60.258374},{-172.729822, 60.261733},{-172.730166, 60.261818},{-172.731200, 60.262075},{-172.731545, 60.262161}, {-172.759572, 60.257714}};
    GeoGerry::LinearRing ring(border);
    cout << point_in_ring({-159.458, 55.6289}, ring) << endl;

    GeoGerry::Shape shape(ring);
    shape.draw();
    // assert_equal("checking area function", area(shape), 0.5);
    // assert_equal("testing center coord 1", center(shape)[0], 0.333333);
    // assert_equal("testing center coord 2", center(shape)[1], 0.333333);
}
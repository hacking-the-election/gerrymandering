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

// int main(int argc, char* argv[]) {
//     GeoDraw::Canvas canvas(640, 900);
    
//     GeoGerry::coordinate_set border = {{0,0}, {0,1}, {1,1}, {1,0}, {0,0}};
//     GeoGerry::coordinate_set border2 = {{0,1}, {0,2}, {1,2}, {1,1}, {0,1}};
//     GeoGerry::LinearRing ring(border);
//     GeoGerry::Shape shape(ring);
//     GeoGerry::LinearRing ring2(border2);
//     GeoGerry::Shape shape2(ring2);
    
// 	ClipperLib::Paths subj;
//     subj.push_back(ring_to_path(shape.hull));

//     ClipperLib::Paths clip;
//     clip.push_back(ring_to_path(shape2.hull));

//     ClipperLib::PolyTree solutions;
//     ClipperLib::Clipper c; // the executor

//     // execute union on paths array
//     c.AddPaths(subj, ClipperLib::ptSubject, false);
//     c.AddPaths(clip, ClipperLib::ptClip, true);
//     c.Execute(ClipperLib::ctIntersection, solutions, ClipperLib::pftPositive);

//     cout << solutions.ChildCount() << endl;
//     // GeoGerry::Multi_Shape ms = paths_to_multi_shape(solutions);
//     // return (ms.border.size() == 1);

//     GeoDraw::Color green(17, 255, 0);
//     GeoDraw::Color blue(0, 17, 255);

//     canvas.add_shape(shape, true, green, 1);
//     canvas.add_shape(shape2, true, blue, 1);
//     canvas.draw();
//     // assert_equal("checking area function", area(shape), 0.5);
//     // assert_equal("testing center coord 1", center(shape)[0], 0.333333);
//     // assert_equal("testing center coord 2", center(shape)[1], 0.333333);
// }

bool is_bordering(ClipperLib::Path p1, ClipperLib::Path p2) {

    ClipperLib::Paths solutions;
    ClipperLib::Clipper c;
    ClipperLib::Paths subj = {p1};
    ClipperLib::Paths clip = {p2};
    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctIntersection, solutions, ClipperLib::pftNonZero);

    return (solutions.size() > 0);
}

int main() {
    ClipperLib::Path p1, p2;
    p1 << ClipperLib::IntPoint(0,0) << ClipperLib::IntPoint(1,0) << ClipperLib::IntPoint(0,1) << ClipperLib::IntPoint(0,0);
    p2 << ClipperLib::IntPoint(1,0) << ClipperLib::IntPoint(1,1) << ClipperLib::IntPoint(0,1) << ClipperLib::IntPoint(1,0);
    // GeoDraw::Canvas c(900, 900);
    // c.add_shape(path_to_ring(p1));
    // c.add_shape(path_to_ring(p2));
    // c.draw();

    cout << is_bordering(p1, p2) << endl;
}
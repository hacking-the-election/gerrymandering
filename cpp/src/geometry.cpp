/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:              Fri, Feb 28
 
 Definition of useful functions for
 computational geometry. Basic 
 calculation, area, bordering - no
 algorithmic specific methods.
========================================*/

#include "../include/geometry.hpp"
#include "../include/canvas.hpp"
#include "../include/shape.hpp"   // class definitions
#include "../include/util.hpp"
#include <iomanip>
#include <math.h>

// define geometric constants
const long int c = pow(10, 18);
const long int d = pow(10, 9);
const long int l = pow(2, 6);

using namespace GeoGerry;
using namespace std;

#include <chrono>

/*
    Following methods utilize the segment of segments typedefs.
    These are useful for summing distances, or checking colinearity,
    which is used for below border functions.
*/

double round_d(double value) {
    return round(value * d) / d;
}


segment coords_to_seg(coordinate c1, coordinate c2) {
    /*
        @desc: combines coordinates into a segment array
        @params: `c1`, `c2`: coordinates 1 and 2 in segment
        @return: `GeoGerry::segment` a segment with the coordinates provided
    */

    segment s = {{ c1[0], c1[1], c2[0], c2[1] }};
    return s;
}


double get_distance(segment s) {
    /* 
        @desc: Distance formula on a segment array
        @params: `s`: a segment to get the distance of
        @return: `double` the distance of the segment
    */
   
    return sqrt(pow((s[2] - s[0]), 2) + pow((s[3] - s[1]), 2));
}


double get_distance(std::array<long long int, 2> c1, std::array<long long int, 2> c2) {
    /* 
        @desc: Distance formula on a segment array
        @params: `c1`, `c2`: coordinates to get the distance of
        @return: `double` the distance of the segment
    */

    return sqrt(pow((c2[0] - c1[0]), 2) + pow((c2[1] - c1[1]), 2));
}


double get_distance(coordinate c0, coordinate c1) {
    /*
        @desc: Distance formula on two separate points
        @params: `c1`, `c2`: coordinates 1 and 2 in segment
        @return: `double` the distance between the coordinates
    */

    return get_distance(coords_to_seg(c0, c1));
}


vector<long int> get_equation(segment s) {
    /*
        @desc:
            Use slope/intercept form and substituting coordinates
            in order to determine equation of a line segment

        @params: `s`: the segment to calculate
        @return: `vector` slope and intercept
        @warn: Need handlers for div by 0
    */

    long int dy = s[3] - s[1];
    long int dx = s[2] - s[0];

    long int m;
    
    if (dx != 0)
        m = (dy * l) / dx;
    else
        m = l;

    long int b = -1 * ((m * s[0]) - s[1]); // @warn - multiply by -1 for value

    return {m, b};
}


bool get_colinear(segment s0, segment s1) {
    /*
        @desc: gets whether or not two lines have the same equation
        @params: `s0`, `s1`: two segments to check colinearity
        @return: `bool` are colinear
    */

    return (get_equation(s0) == get_equation(s1));
}


bool point_on_segment(coordinate c, segment s) {
    /*
        @desc: checks if a coordinate lines within a line segment
        @params:
            `coordinate` c: coordinate to check
            `segment` s: segment to contain point

        @return: `bool` segment contains line
    */

    if ((c[0] == s[0] && c[1] == s[1]) || (c[0] == s[2] && c[1] == s[3]))
        // point on endpoints of segment
        return true;

    int crossproduct = (c[1] - s[1]) * (s[2] - s[0]) - (c[0] - s[0]) * (s[3] - s[1]);
    if (abs(crossproduct) != 0)
        return false;

    int dotproduct = (c[0] - s[0]) * (s[2] - s[0]) + (c[1] - s[1]) * (s[3] - s[1]);
    if (dotproduct < 0)
        return false;

    int squaredlengthba = (s[2] - s[0]) * (s[2] - s[0]) + (s[3] - s[1]) * (s[3] - s[1]);

    if (dotproduct > squaredlengthba)
        return false;

    return true;
}


bool get_overlap(segment s0, segment s1) {
    /*
        @desc:
            Returns whether or not two segments' bounding boxes
            overlap, meaning one of the extremes of a segment are
            within the range of the other's. One shared point does
            not count to overlap.
        
        @params: `s0`, `s1`: two segments to check overlap
        @return: `bool` segments overlap
        @warn: This function is untested!
    */
    
    long int s0min = s0[0];
    long int s0max = s0[2];

    if (s0[2] < s0min) {
        s0min = s0[2];
        s0max = s0[0];
    }

    long int s1min = s1[0];
    long int s1max = s1[2];

    if (s1[2] < s1min) {
        s1min = s1[2];
        s1max = s1[0];
    }

    return (s0min <= s1max && s1min <= s0max);
}


segments GeoGerry::LinearRing::get_segments() {
    /*
        @desc:
            returns a vector of segments from the
            coordinate array of a LinearRing.border property

        @params: none
        @return: segments of a ring
    */

    segments segs;
    vector<coordinate> border_x = border;

    // loop through segments
    // while (border_x[border_x.size() - 1] == border_x[0])
    //     border_x.pop_back();

    for (int i = 0; i < border_x.size(); i++) {
        coordinate c1 = border_x[i];  // starting coord
        coordinate c2;                // ending coord

        // find position of ending coordinate
        if (i == border_x.size() - 1) c2 = border_x[0];
        else c2 = border_x[i + 1];

        segs.push_back(coords_to_seg(c1, c2)); // add to list
    }

    return segs;
}


segments GeoGerry::Shape::get_segments() {
    /*
        @desc: get list of all segments in a shape, including hole LinearRings array
        @params: none
        @return: segments of a shape
    */

    segments segs = this->hull.get_segments();
    
    for (LinearRing hole : this->holes)
        for (segment seg : hole.get_segments())
            segs.push_back(seg);

    return segs;
}


segments GeoGerry::Multi_Shape::get_segments() {
    /*
        @desc: get a list of all segments in a multi_shape, for each shape, including holes
        @params: none
        @return: segments array of each shape
    */

    segments segs;
    for (Shape s : this->border)
        for (segment seg : s.get_segments())
            segs.push_back(seg);

    return segs;
}


coordinate GeoGerry::LinearRing::get_center() {
    /* 
        @desc: Gets the centroid of a polygon with coords
        @ref: https://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
        @params: none
        @return: coordinate of centroid
    */
    float centroidX = 0, centroidY = 0;
    float det = 0, tempDet = 0;
    unsigned int j = 0;
    int nVertices = border.size();

    for (int i = 0; i < nVertices; i++) {
        // closed polygon
        if (i + 1 == nVertices)
        	j = 0;
		else
			j = i + 1;

		// compute the determinant
		tempDet = border[i][0] * border[j][1] - border[j][0] * border[i][1];
		det += tempDet;

		centroidX += (border[i][0] + border[j][0]) * tempDet;
		centroidY += (border[i][1] + border[j][1]) * tempDet;
	}

	// divide by the total mass of the polygon
	centroidX /= 3*det;
	centroidY /= 3*det;

    coordinate centroid = {(long int)centroidX, (long int)centroidY};
    return centroid;
}


double GeoGerry::LinearRing::get_area() {
    /*
        @desc:
            returns the area of a linear ring, using latitude * long
            area - an implementation of the shoelace theorem

        @params: none
        @ref: https://www.mathopenref.com/coordpolygonarea.html
        @return: area of linear ring as a double
    */

    double area = 0;
    int points = border.size() - 1; // last index of border

    for ( int i = 0; i < border.size(); i++ ) {
        area += (border[points][0] + border[i][0]) * (border[points][1] - border[i][1]);
        points = i;
    }

    return (area / 2);
}


double GeoGerry::LinearRing::get_perimeter() {
    /*
        @desc: returns the perimeter of a LinearRing object by summing distance
        @params: none
        @return: `double` perimeter
    */

    double t = 0;
    for (segment s : get_segments())
        t += get_distance(s);    

    return t;
}


coordinate GeoGerry::Shape::get_center() {
    /*
        @desc:
            returns average centroid from list of `holes`
            and `hull` by calling `LinearRing::get_center`

        @params: none
        @return: `coordinate` average centroid of shape
    */

    coordinate center = hull.get_center();

    for (GeoGerry::LinearRing lr : holes) {
        coordinate nc = lr.get_center();
        center[0] += nc[0];
        center[1] += nc[1];
    }

    int size = 1 + holes.size();
    return {center[0] / size, center[1] / size};
}


coordinate GeoGerry::Multi_Shape::get_center() {
    /*
        @desc:
            returns average centroid from list of `holes`
            and `hull` by calling `LinearRing::get_center`

        @params: none
        @return: `coordinate` average centroid of shape
    */

    coordinate average = {0,0};

    for (Shape s : border) {
        coordinate center = s.hull.get_center();

        for (GeoGerry::LinearRing lr : s.holes) {
            coordinate nc = lr.get_center();
            center[0] += nc[0];
            center[1] += nc[1];
        }

        int size = 1 + s.holes.size();
        average[0] += center[0] / size;
        average[1] += center[1] / size;
    }

    return {(int)(average[0] / border.size()), (int)(average[1] / border.size())};
}


double GeoGerry::Shape::get_area() {
    /*
        @desc:
            gets the area of the hull of a shape
            minus the combined area of any holes

        @params: none
        @return: `double` totale area of shape
    */

    double area = hull.get_area();
    for (GeoGerry::LinearRing h : holes)
        area -= h.get_area();

    return area;
}


double GeoGerry::Shape::get_perimeter() {
    /*
        @desc:
            gets the sum perimeter of all LinearRings
            in a shape object, including holes

        @params: none
        @return: `double` total perimeter of shape
    */

    double perimeter = hull.get_perimeter();
    for (GeoGerry::LinearRing h : holes)
        perimeter += h.get_perimeter();

    return perimeter;
}


double Multi_Shape::get_area() {
    /*
        @desc: gets sum area of all Shape objects in border
        @params: none
        @return: `double` total area of shapes
    */

    double total = 0;
    for (Shape s : border)
        total += s.get_area();

    return total;
}


double GeoGerry::Multi_Shape::get_perimeter() {
    /*
        @desc:
            gets sum perimeter of a multi shape object
            by looping through each shape and calling method
            
        @params: none
        @return `double` total perimeter of shapes array
    */

    double p = 0;
    for (Shape shape : border)
        p += shape.get_perimeter();

    return p;
}


bool get_bordering(Shape s0, Shape s1) {
    /*
        @desc: gets whether or not two shapes touch each other
        @params: `Shape` s0, `Shape` s1: shapes to check bordering
        @return: `bool` shapes are boording
    */
    
    // create paths array from polygon
    cout << "x" << endl;
	ClipperLib::Paths subj;
    subj.push_back(ring_to_path(s0.hull));

    ClipperLib::Paths clip;
    clip.push_back(ring_to_path(s1.hull));

    ClipperLib::Paths solutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftPositive);

    Multi_Shape ms = paths_to_multi_shape(solutions);
    return (ms.border.size() == 1);
}


bool get_bordering(Multi_Shape s0, Shape s1) {
    /*
        @desc: gets whether or not two shapes touch each other
        @params: `Multi_Shape` s0, `Shape` s1: shapes to check bordering
        @return: `bool` shapes are boording
    */

    // create paths array from polygon
	ClipperLib::Paths subj;
    for (Shape s : s0.border)
        subj.push_back(ring_to_path(s.hull));

    ClipperLib::Paths clip;
    clip.push_back(ring_to_path(s1.hull));

    ClipperLib::Paths usolutions;
    ClipperLib::Paths xsolutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctXor, xsolutions, ClipperLib::pftNonZero);
    c.Execute(ClipperLib::ctUnion, usolutions, ClipperLib::pftNonZero);

    Multi_Shape msx = paths_to_multi_shape(xsolutions);
    Multi_Shape msu = paths_to_multi_shape(usolutions);

    return (msu.border.size() == s0.border.size() && msx.holes.size() == 0);
}


bool get_bordering(Multi_Shape s0, Multi_Shape s1) {
    /*
        @desc: gets whether or not two shapes touch each other
        @params: `Multi_Shape` s0, `Multi_Shape` s1: shapes to check bordering
        @return: `bool` shapes are boording
    */

    // create paths array from polygon
	ClipperLib::Paths subj;
    for (Shape s : s0.border)
        subj.push_back(ring_to_path(s.hull));

    ClipperLib::Paths clip;
    for (Shape s : s1.border)
        clip.push_back(ring_to_path(s.hull));

    ClipperLib::Paths usolutions;
    ClipperLib::Paths xsolutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctXor, xsolutions, ClipperLib::pftNonZero);
    c.Execute(ClipperLib::ctUnion, usolutions, ClipperLib::pftNonZero);

    Multi_Shape msx = paths_to_multi_shape(xsolutions);
    Multi_Shape msu = paths_to_multi_shape(usolutions);

    return (msu.border.size() < s0.border.size() + s1.border.size() && msx.holes.size() <= s0.holes.size() + s1.holes.size());
}


bool point_in_ring(GeoGerry::coordinate coord, GeoGerry::LinearRing lr) {
    /*
        @desc:
            gets whether or not a point is in a ring using
            the ray intersection method (clipper implementation)

        @ref: http://www.angusj.com/delphi/clipper/documentation/Docs/Units/ClipperLib/Functions/PointInPolygon.htm
        @params: 
            `coordinate` coord: the point to check
            `LinearRing` lr: the shape to check the point against
        
        @return: `bool` point is in/on polygon
    */

    // close ring for PIP problem
    if (lr.border[0] != lr.border[lr.border.size() - 1])
        lr.border.push_back(lr.border[0]);

    // convert to clipper types for builtin function
    ClipperLib::Path path = ring_to_path(lr);
    ClipperLib::IntPoint p(coord[0], coord[1]);
    return (!(ClipperLib::PointInPolygon(p, path) == 0));
}

bool get_inside(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    /*
        @desc:
            gets whether or not s0 is inside of 
            s1 using the intersection point method

        @params:
            `LinearRing` s0: ring inside `s1`
            `LinearRing` s1: ring containing `s0`

        @return: `bool` `s0` inside `s1`
    */

    for (coordinate c : s0.border)
        if (!point_in_ring(c, s1)) return false;

    return true;
}

bool get_inside_b(GeoGerry::Multi_Shape s0, GeoGerry::Shape s1) {
    /*
        @desc:
            gets whether or not s0 is inside of 
            s1 using the intersection point method

        @params:
            `LinearRing` s0: ring inside `s1`
            `LinearRing` s1: ring containing `s0`

        @return: `bool` `s0` inside `s1`
    */

    for (Shape s : s0.border)
        for (coordinate c : s.hull.border)
            if (point_in_ring(c, s1.hull)) return true;

    return false;
}


bool get_inside_first(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    /*
        @desc:
            gets whether or not the first point of s0 is
            inside of s1 using the intersection point method

        @params:
            `LinearRing` s0: ring inside `s1`
            `LinearRing` s1: ring containing `s0`

        @return: `bool` first coordinate of `s0` inside `s1`
    */

    return (point_in_ring(s0.border[0], s1));
}

p_index_set get_inner_boundary_precincts(Precinct_Group shape) {
    /*
        @desc:
            gets an array of indices that correspond
            to precincts on the inner edge of a Precinct Group
        
        @params: `Precinct_Group` shape: the precinct group to get inner precincts of
        @return: `p_index_set` indices of inner border precincts
    */

    p_index_set boundary_precincts;
    Multi_Shape exterior_border;
    exterior_border.border = shape.border;
    GeoDraw::Canvas canvas(900, 900);

    int i = 0;
    
    for (Precinct p : shape.precincts) {
        if (get_bordering(exterior_border, p)) {
            boundary_precincts.push_back(i);
            canvas.add_shape(p);
        }
        i++;
    }

    // canvas.draw();
    return boundary_precincts;
}


GeoGerry::p_index_set get_inner_boundary_precincts(p_index_set precincts, State state) {
    /*
        @desc:
            gets an array of indices that correspond
            to precincts on the inner edge of a precinct_index_set
        
        @params: 
            `p_index_set` precincts: the precinct index set to get inner precincts of
            `State` state: The precincts that correspond to the indices

        @return: `p_index_set` indices of inner border precincts
    */
    p_index_set boundary_precincts;

    Precinct_Group pg;
    for (p_index p : precincts)
        pg.add_precinct(state.precincts[p]);

    Multi_Shape border = generate_exterior_border(pg);

    int i = 0;
    for (Precinct p : pg.precincts) {
        if (get_bordering(border, p)) {
            boundary_precincts.push_back(i);
        }
        i++;
    }

    return boundary_precincts;
}


p_index_set get_bordering_shapes(vector<Shape> shapes, Shape shape) {
    /*
        @desc:
            returns set of indices corresponding to the Precinct_Groups that
            border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if (( shapes[i] != shape ) && get_bordering(shapes[i], shape)) vec.push_back(i);
    
    return vec;
}

p_index_set get_bordering_shapes(vector<Precinct_Group> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if (( shapes[i] != shape ) && get_bordering(shapes[i], shape)) vec.push_back(i);

    return vec;
}

p_index_set get_bordering_shapes(vector<Community> shapes, Community shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++) {
        if ((shapes[i] != shape) && get_bordering(shapes[i], shape)) vec.push_back(i);
    }
    
    return vec;
}


p_index_set get_bordering_shapes(vector<Community> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.xs
    */

    p_index_set vec;

    for (p_index i = 0; i < shapes.size(); i++) {
        if (shapes[i] != shape && get_bordering(shapes[i], shape)) vec.push_back(i);
        // else {
            // GeoDraw::Canvas c(900, 900);
            // c.add_shape(generate_exterior_border(shapes[i]));
            // c.add_shape(shape);
            // cout << "do not border... to debug draw here" << endl;
            // c.draw();
        // }
    }

    return vec;
}

p_index_set get_bordering_precincts(Precinct_Group shape, int p_index) {
    /*
        Returns precincts in a Precinct_Group that border
        a specified precinct index in that group
    */

    p_index_set precincts;

    for (int i = 0; i < shape.precincts.size(); i++) {
        if ( i != p_index ) {
            // don't check the same precinct
            if (get_bordering(shape.precincts[p_index], shape.precincts[i]))
                precincts.push_back(i);
        }
    }
    return precincts;
}

unit_interval Shape::get_compactness() {
    /*
        An implementation of the Schwartzberg compactness score.
        Returns the ratio of the perimeter of a shape to the
        circumference of a circle with the same area as that shape.
    */

    double circle_radius = sqrt(this->get_area() / PI);
    double circumference = 2 * circle_radius * PI;
    // std::cout << this->get_area() << ", " << circumference << ", " << this->get_perimeter();
    return (circumference / this->get_perimeter());
}

unit_interval Multi_Shape::get_compactness() {
    /*
        An implementation of the Schwartzberg compactness score.
        Returns the ratio of the perimeter of a shape to the
        circumference of a circle with the same area as that shape.
    */

    double circle_radius = sqrt(this->get_area() / PI);
    double circumference = 2 * circle_radius * PI;
    // std::cout << this->get_area() << ", " << circumference << ", " << this->get_perimeter() << std::endl;
    return (circumference / this->get_perimeter());
}

double get_standard_deviation_partisanship(Precinct_Group pg) {
    /*
        Returns the standard deviation of the partisanship
        ratio for a given array of precincts
    */

    vector<Precinct> p;
    for (Precinct pre : pg.precincts)
        if (pre.get_ratio() != -1) p.push_back(pre);

    double mean = p[0].get_ratio();

    for (int i = 1; i < p.size(); i++)
        mean += p[i].get_ratio();

    mean /= p.size();
    double dev_mean = pow(p[0].get_ratio() - mean, 2);

    for (int i = 1; i < p.size(); i++)
        dev_mean += pow(p[i].get_ratio() - mean, 2);

    dev_mean /= p.size();
    return (sqrt(dev_mean));
}

double get_standard_deviation_partisanship(Communities cs) {
    /*
        @desc: gets average partisanship standard deviation of communities within a group
        @params: `Communities` cs: community list to check
        @return: `double` average standard deviation of partisanships
    */

    double d = 0;
    for (Community c : cs) d += get_standard_deviation_partisanship(c);
    return (d / cs.size());
}

double get_median_partisanship(Precinct_Group pg) {
    /*
        Returns the median partisanship ratio
        for a given array of precincts
    */

    double median;
    vector<double> ratios;
    int s = pg.precincts.size();
    
    // get array of ratios
    for (Precinct p : pg.precincts)
        ratios.push_back(p.get_ratio());
    sort(ratios.begin(), ratios.end()); // sort array

    // get median from array of ratios
    if (s % 2 == 0) median = (ratios[(s - 1) / 2] + ratios[s / 2]) / 2.0;
    else median = ratios[s / 2];

    return median;
}

Multi_Shape generate_exterior_border(Precinct_Group precinct_group) {
    /*
        Get the exterior border of a shape with interior components.
        Equivalent to 'dissolve' in mapshaper - remove bordering edges.
        Uses the Clipper library by Angus Johnson to union many polygons
        efficiently.

        @params:
            `precinct_group`: A precinct group to generate the border of

        @return:
            Multi_Polygon: exterior border of `precinct_group`
    */ 

    // create paths array from polygon
	ClipperLib::Paths subj;

    for (Precinct p : precinct_group.precincts)
        for (ClipperLib::Path path : shape_to_paths(p))
            subj.push_back(path);


    ClipperLib::Paths solutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

    return paths_to_multi_shape(solutions);
}

ClipperLib::Path ring_to_path(GeoGerry::LinearRing ring) {
    /*
        Creates a clipper Path object from a
        given Shape object by looping through points
    */

    ClipperLib::Path p;
    for (coordinate point : ring.border )
        p.push_back(ClipperLib::IntPoint(point[0], point[1]));

    return p;
}

GeoGerry::LinearRing path_to_ring(ClipperLib::Path path) {
    /*
        Creates a shape object from a clipper Path
        object by looping through points
    */

    GeoGerry::LinearRing s;

    for (ClipperLib::IntPoint point : path ) {
        coordinate p = {point.X, point.Y};
        if (p[0] != 0 && p[1] != 0) s.border.push_back(p);
    }

    if (s.border[0] != s.border[s.border.size() - 1])
        s.border.push_back(s.border[0]);

    return s;
}

ClipperLib::Paths shape_to_paths(GeoGerry::Shape shape) {

    if (shape.hull.border[0] != shape.hull.border[shape.hull.border.size() - 1])
        shape.hull.border.push_back(shape.hull.border[0]);

    ClipperLib::Paths p;
    p.push_back(ring_to_path(shape.hull));
    
    for (GeoGerry::LinearRing ring : shape.holes) {
        if (ring.border[0] != ring.border[ring.border.size() - 1])
            ring.border.push_back(ring.border[0]);

        ClipperLib::Path path = ring_to_path(ring);
        ReversePath(path);
        p.push_back(path);
    }

    return p;
}


GeoGerry::Multi_Shape paths_to_multi_shape(ClipperLib::Paths paths) {
    /*
        @desc: 
              Create a Multi_Shape object from a clipper Paths
              (multi path) object through nested iteration

        @params: `ClipperLib::Paths` paths: A
        @warn:
            `ClipperLib::ReversePath` is arbitrarily called here,
            and there should be better ways to check whether or not
            it's actually needed
    */

    Multi_Shape ms;
    ReversePaths(paths);

    for (ClipperLib::Path path : paths) {
        if (!ClipperLib::Orientation(path)) {
            GeoGerry::LinearRing border = path_to_ring(path);
            if (border.border[0] == border.border[border.border.size() - 1]) {
                // border.border.insert(border.border.begin(), border.border[border.border.size() - 1]);
                GeoGerry::Shape s(border);
                ms.border.push_back(s);
            }
        }
        else {
            // std::cout << "hole" << std::endl;
            ReversePath(path);
            GeoGerry::LinearRing hole = path_to_ring(path);
            ms.holes.push_back(hole);
        }
    }

    return ms;
}

Multi_Shape poly_tree_to_shape(ClipperLib::PolyTree tree) {
    /*
        Loops through top-level children of a
        PolyTree to access outer-level polygons. Returns
        a multi_shape object containing these outer polys.
    */
   
    Multi_Shape ms;
    
    for (ClipperLib::PolyNode* polynode : tree.Childs) {
        // if (polynode->IsHole()) x++;
        GeoGerry::LinearRing s = path_to_ring(polynode->Contour);
        Shape shape(s);
        ms.border.push_back(shape);
    }

    return ms;
}


p_index_set get_ext_bordering_precincts(Precinct_Group precincts, State state) {
    /*
        @desc: a method for getting the precincts in a state that
               border a precinct group. This is used in the communities
               algorithm.

        @params
            `precincts`: The precinct group to find borders of
            `state`: A state object with lists of precincts to look through

        @return: `p_index_set` a set of precinct indices that border `precincts`
    */

    p_index_set bordering_pre;
    Multi_Shape border = generate_exterior_border(precincts);

    for (int i = 0; i < state.precincts.size(); i++) {
        if (!(std::find(precincts.precincts.begin(), precincts.precincts.end(), state.precincts[i]) != precincts.precincts.end())) {
            if (get_bordering(border, state.precincts[i])) bordering_pre.push_back(i);
        }
    }

    return bordering_pre;
}

GeoGerry::p_index_set get_ext_bordering_precincts(GeoGerry::Precinct_Group precincts, GeoGerry::p_index_set available_pre, GeoGerry::State state) {
    /*
        @desc: a method for getting the precincts in a state that
               border a precinct group. This is used in the communities
               algorithm.

        @params
            `precincts`: The precinct group to find borders of
            `state`: A state object with lists of precincts to look through

        @return: `p_index_set` a set of precinct indices that border `precincts`
    */

    p_index_set bordering_pre;
    Multi_Shape border = generate_exterior_border(precincts);

    for (int i = 0; i < state.precincts.size(); i++) {
        if (std::find(available_pre.begin(), available_pre.end(), i) != available_pre.end()) {
            if (!(std::find(precincts.precincts.begin(), precincts.precincts.end(), state.precincts[i]) != precincts.precincts.end())) {
                if (get_bordering(border, state.precincts[i])) bordering_pre.push_back(i);
            }
        }
    }

    return bordering_pre;
}


bool creates_island(GeoGerry::Precinct_Group set, GeoGerry::p_index remove) {
    /*
        @desc: determines whether removing a precinct index from a set of
               precincts will create an island or not

        @params
            `set`: The precinct group to check removal of precinct `remove` from
            `remove`: The precinct index to remove from the `set`

        @return: `bool` exchange creates an island
    */
    
    // remove precinct from set
    set.remove_precinct(set.precincts[remove]);
    return (set.border.size() > 1);
}


bool creates_island(GeoGerry::p_index_set set, GeoGerry::p_index remove, GeoGerry::State precincts) {
    /*
        @desc: determines whether removing a precinct index from a set of
               precincts will create an island or not

        @params
            `set`: an list of precinct indices corresponding to the `precincts.precincts` attribute
            `remove`: The precinct index to remove from the `set`
            `precincts`: A state object that contains precincts to match to `set`

        @return: `bool` exchange creates an island
    */
   

    // calculate initial number of islands in set
    // Precinct_Group pg_before;
    // for (p_index p : set)
    //     pg_before.add_precinct(precincts.precincts[p]);

    // int islands_before = generate_exterior_border(pg_before).border.size();

    // remove precinct from set
    set.erase(std::remove(set.begin(), set.end(), remove), set.end());

    // calculate new number of islands
    Precinct_Group pg_after;
    for (p_index p : set)
        pg_after.add_precinct_n(precincts.precincts[p]);

    int islands_after = generate_exterior_border(pg_after).border.size();

    return (islands_after > 1);
}


bool creates_island(GeoGerry::Precinct_Group set, GeoGerry::Precinct precinct) {
    set.remove_precinct(precinct);
    return (set.border.size() > 1);
}


p_index_set get_giveable_precincts(Community c, Communities cs) {
    /*
        @desc:
            Gets precincts that can be given to another
            community - can never return null for geometric reasons
    */
   
    p_index_set borders = get_inner_boundary_precincts(c);
    p_index_set exchangeable_precincts;

    // Multi_Shape ms(c.border);
    // GeoDraw::Canvas canvas(900, 900);

    for (p_index p : borders) {
        for (Community c_p : cs) {
            if ((c_p != c) && get_bordering(c_p, c.precincts[p]) && !creates_island(c, p)) {
                exchangeable_precincts.push_back(p);
                // canvas.add_shape(c.precincts[p]);
                break;
            }
        }
    }

    // canvas.add_shape(ms);
    // canvas.draw();

    return exchangeable_precincts;
}

vector<array<int, 2>> get_takeable_precincts(Community c, Communities cs) {
    /*
        {1, 5} => second community, 5th precinct

        @desc:
            Gets precincts that can be taken from another
            community - can never return null for geometric reasons
    */

    vector<array<int, 2>> takeable;

    p_index_set borders = get_bordering_shapes(cs, c);
    for (p_index b : borders) {
        p_index_set boundaries = get_inner_boundary_precincts(cs[b]);
        for (p_index p : boundaries) {
            if (get_bordering(c, cs[b].precincts[p])) takeable.push_back({b, p});
        }
    }

    return takeable;
}


Shape generate_gon(coordinate c, double radius, int n) {
    /*
        Takes a radius, center, and number of sides to generate
        a regular polygon around that center with that radius
    */

    double angle = 360 / n;
    coordinate_set coords;

    for (int i = 0; i < n; i++) {
        double x = radius * std::cos((angle * i) * PI/180);
        double y = radius * std::sin((angle * i) * PI/180);
        coords.push_back({(int)x + c[0], (int)y + c[1]});
    }

    LinearRing lr(coords);
    return Shape(lr);
}

// geos::geom::GeometryFactory::Ptr global_factory;

// Point* create_point(double x, double y) {
//     /* given coordinates creates a point object */
//     Coordinate c(x, y);
//     Point* p = global_factory->createPoint(c);
//     return p;
// }

// geos::geom::LinearRing* create_linearring(coordinate_set coords) {
//     // We will use a coordinate list to build the linearring
//     CoordinateArraySequence* cl = new CoordinateArraySequence();

//     for (coordinate c : coords) {
//         cl->add(Coordinate(c[0], c[1]));
//     }

//     geos::geom::LinearRing* lr = global_factory->createLinearRing(cl);
//     return lr; // our LinearRing
// }

// geos::geom::Geometry* shape_to_poly(GeoGerry::LinearRing shape) {
//     /*
//         Creates a GEOS library polygon object from a
//         given Shape object by looping through points
//     */

//     // blank holes vector
//     vector<geos::geom::LinearRing*>* holes = new vector<geos::geom::LinearRing*>;
//     // outer ring generated by looping through coordinates
//     geos::geom::LinearRing* outer = create_linearring(shape.border);
//     geos::geom::Polygon* poly = global_factory->createPolygon(outer, holes);

//     return poly;
// }

// geos::geom::Geometry::NonConstVect multi_shape_to_poly(GeoGerry::Multi_Shape ms) {
//     geos::geom::Geometry::NonConstVect geoms;

//     for (Shape s : ms.border) {
//         geos::geom::Geometry* geo = shape_to_poly(s.hull);
//         geoms.push_back(geo);
//     }

//     return geoms;
// }

// Shape poly_to_shape(const geos::geom::Geometry* path) {
//     /*
//         Creates a shape object from a clipper Path
//         object by looping through points
//     */

//     Shape s;
    
//     // write coordinates in path to vector of Coordinates
//     std::unique_ptr<geos::geom::CoordinateSequence> points = path->getCoordinates();
//     vector<Coordinate> coords;
//     points->toVector(coords);
    
//     for (Coordinate coord : coords) {
//         coordinate nc = {coord.x, coord.y};
//         s.hull.border.push_back(nc);
//     }

//     return s;
// }

// Multi_Shape* multipoly_to_shape(MultiPolygon* paths) {
//     /*
//         Create a Multi_Shape object from a clipper Paths
//         (multi path) object through nested iteration
//     */

//     Multi_Shape* ms;
    
//     return ms;
// }
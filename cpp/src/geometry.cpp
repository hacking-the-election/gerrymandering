/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:               Wed, Feb 26
 
 Definition of useful functions for
 computational geometry. Basic 
 calculation, area, bordering - no
 algorithmic specific methods.
========================================*/

#include "../include/geometry.hpp"
#include "../include/shape.hpp"   // class definitions
#include "../include/gui.hpp"     // for the draw function

// define geometric constants
#define PI 3.14159265358979323846
const int c = pow(10, 18);

using namespace GeoGerry;
using namespace std;

/*
    Following methods utilize the segment of segments typedefs.
    These are useful for summing distances, or checking colinearity,
    which is used for below border functions.
*/

segment coords_to_seg(coordinate c1, coordinate c2) {
    /*
        @desc: combines coordinates into a segment array
        @params: `c1`, `c2`: coordinates 1 and 2 in segment
        @return: `GeoGerry::segment` a segment with the coordinates provided
    */

    segment s = {{c1[0], c1[1], c2[0], c2[1]}};
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

double get_distance(coordinate c0, coordinate c1) {
    /*
        @desc: Distance formula on two separate points
        @params: `c1`, `c2`: coordinates 1 and 2 in segment
        @return: `double` the distance between the coordinates
    */

    return get_distance(coords_to_seg(c0, c1));
}

vector<double> get_equation(segment s) {
    /*
        @desc: Use slope/intercept form and substituting coordinates
               in order to determine equation of a line segment

        @params: `s`: the segment to calculate
        @return: `vector` slope and intercept
        @warn: Need handlers for div by 0
    */

    double m = (s[3] - s[1]) / (s[2] - s[0]);
    double b = -m * s[0] + s[1];

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

bool get_overlap(segment s0, segment s1) {
    /*
        @desc: Returns whether or not two segments' bounding boxes
               overlap, meaning one of the extremes of a segment are
               within the range of the other's. One shared point does
               not count to overlap.
        
        @params: `s0`, `s1`: two segments to check overlap
        @return: `bool` segments overlap
        @warn: This function is untested!
    */
    
    if (s0[0] > s0[2])
        return (
                ((s1[0] < s0[0]) && (s1[0] > s0[2]))
                || ((s1[2] < s0[0]) && (s1[2] > s0[2])) 
               );
    else
        return (
                ((s1[0] < s0[2]) && (s1[0] > s0[0]))
                || ((s1[2] < s0[2]) && (s1[2] > s0[0]))
               );
}

bool get_bordering(segment s0, segment s1) {
    /*
        @desc: Returns whether or not two segments are colinear and overlap
        @params: `s0`, `s1`: two segments to check bordering
        @return: `bool` segments border
    */

    return (get_colinear(s0, s1) && get_overlap(s0, s1));
}

segments GeoGerry::LinearRing::get_segments() {
    /*
        @desc: returns a vector of segments from the
               coordinate array of a LinearRing.border property
        @params: none
        @return: segments of a ring
    */

    segments segs;
    
    // loop through segments
    if (border[border.size() - 1] == border[0])
        border.pop_back();

    for (int i = 0; i < border.size(); i++) {
        coordinate c1 = border[i];  // starting coord
        coordinate c2;              // ending coord

        // find position of ending coordinate
        if (i == border.size() - 1)
            c2 = border[0];
        else
            c2 = border[i + 1];

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

    segments segs = hull.get_segments();
    
    for (LinearRing hole : holes) {
        for (segment seg : hole.get_segments()) segs.push_back(seg);
    }

    return segs;
}

segments GeoGerry::Multi_Shape::get_segments() {
    /*
        @desc: get a list of all segments in a multi_shape, for each shape, including holes
        @params: none
        @return: segments array of each shape
    */

    segments segs;
    for (Shape s : border)
        for (segment seg : s.get_segments()) segs.push_back(seg);

    return segs;
}


coordinate GeoGerry::LinearRing::get_center() {
    /* 
        @desc: Gets the centroid of a polygon with coords
        @ref: https://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
        @params: none
        @return: coordinate of centroid
    */

    coordinate centroid = {0, 0};
    double x0, y0, x1, y1;

    for (int i = 0; i < border.size() - 1; i++) {
        // assign coordinates to variables
        x0 = border[i][0];
        y0 = border[i][1];
        x1 = border[i + 1][0];
        y1 = border[i + 1][1];

        // get first factor in centroid formula
        double factor = ((x0 * y1) - (x1 * y0));
        
        // calculate current coordinate
        centroid[0] += (x0 + x1) * factor;
        centroid[1] += (y0 + y1) * factor;
    }

    // divide to find centroid
    centroid[0] /= (6 * this->get_area());
    centroid[1] /= (6 * this->get_area());

    return centroid;
}


double GeoGerry::LinearRing::get_area() {
    /*
        @desc: returns the area of a linear ring, using latitude * long
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
        @desc: returns average centroid from list of `holes`
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


double GeoGerry::Shape::get_area() {
    /*
        @desc: returns the area of the hull of a shape
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
        @desc: returns the sum perimeter of all LinearRings
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
        @desc: Returns sum area of all Shape objects in border
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
        @desc: returns sum perimeter of a multi shape object
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
        @desc: returns whether or not two shapes touch each other
        @params: `Shape` s0, `Shape` s1: shapes to check bordering
        @return: `bool` shapes are boording
    */
    
    for (segment seg0 : s0.get_segments())
        for (segment seg1 : s1.get_segments())
            if (get_equation(seg0) == get_equation(seg1) && get_overlap(seg0, seg1)) 
                return true;

    return false;
}


bool point_in_ring(GeoGerry::coordinate coord, GeoGerry::LinearRing lr) {
    /*
        @desc: returns whether or not a point is in a ring using
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
    ClipperLib::IntPoint p(coord[0] * c, coord[1] * c);
    return (!(ClipperLib::PointInPolygon(p, path) == 0));
}

bool get_inside(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    /*
        returns whether or not s0 is inside of 
        s1 using the intersection point method
    */

    for (coordinate c : s0.border)
        if (!point_in_ring(c, s1)) return false;

    return true;
}

bool get_inside_first(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    /*
        returns whether or not s0 is inside of 
        s1 using the intersection point method
    */

    return (point_in_ring(s0.border[0], s1));
}

bool get_inside_or(GeoGerry::LinearRing s0, GeoGerry::LinearRing s1) {
    for (coordinate c : s0.border)
        if (point_in_ring(c, s1)) return true;
    return false;
}

p_index_set get_inner_boundary_precincts(Precinct_Group shape) {
   
    /*
        returns an array of indices that correspond
        to precincts on the inner edge of a Precinct Group
    */

    p_index_set boundary_precincts;
    Multi_Shape exterior_border = generate_exterior_border(shape);
    
    int i = 0;
    
    for (Precinct p : shape.precincts) {
        if (get_bordering(p, exterior_border)) {
            boundary_precincts.push_back(i);
        }
        i++;
    }

    return boundary_precincts;
}



p_index_set get_bordering_shapes(vector<Shape> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if ( ( shapes[i] != shape ) && get_bordering(shapes[i], shape)) vec.push_back(i);
    
    return vec;
}

p_index_set get_bordering_shapes(vector<Precinct_Group> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if ( ( shapes[i] != shape ) && get_bordering(shapes[i], shape)) vec.push_back(i);
    
    return vec;
}

p_index_set get_bordering_shapes(vector<Community> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if ( ( shapes[i] != shape ) && get_bordering(shapes[i], shape)) vec.push_back(i);
    
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
    std::cout << this->get_area() << ", " << circumference << ", " << this->get_perimeter();
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
    std::cout << this->get_area() << ", " << circumference << ", " << this->get_perimeter() << std::endl;
    return (circumference / this->get_perimeter());
}

double get_standard_deviation_partisanship(Precinct_Group pg) {
    /*
        Returns the standard deviation of the partisanship
        ratio for a given array of precincts
    */

    vector<Precinct> p = pg.precincts;
    double mean = p[0].get_ratio();

    for (int i = 1; i < pg.precincts.size(); i++)
        mean += p[i].get_ratio();

    mean /= p.size();
    double dev_mean = pow(p[0].get_ratio() - mean, 2);

    for (int i = 1; i < p.size(); i++)
        dev_mean += pow(p[i].get_ratio() - mean, 2);

    return (sqrt(dev_mean));
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
    if (s % 2 == 0)
        median = (ratios[(s - 1) / 2] + ratios[s / 2]) / 2.0;
    else
        median = ratios[s / 2];

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

    ClipperLib::Path p(ring.border.size());

    for (coordinate point : ring.border ) {
        p << ClipperLib::IntPoint(point[0] * c, point[1] * c);
    }

    return p;
}

GeoGerry::LinearRing path_to_ring(ClipperLib::Path path) {
    /*
        Creates a shape object from a clipper Path
        object by looping through points
    */

    GeoGerry::LinearRing s;

    for (ClipperLib::IntPoint point : path ) {
        coordinate p = {((point.X) / (long double)c), ((point.Y) / (long double)c)};
        if (p[0] != 0 && p[1] != 0) s.border.push_back(p);
    }

    return s;
}

ClipperLib::Paths shape_to_paths(GeoGerry::Shape shape) {
    ClipperLib::Paths p;
    p.push_back(ring_to_path(shape.hull));
    
    for (GeoGerry::LinearRing ring : shape.holes) {
        ClipperLib::Path path = ring_to_path(ring);
        ReversePath(path);
        p.push_back(path);
    }

    return p;
}

GeoGerry::Multi_Shape paths_to_multi_shape(ClipperLib::Paths paths) {
    /*
        Create a Multi_Shape object from a clipper Paths
        (multi path) object through nested iteration
    */

    Multi_Shape ms;
    
    for (ClipperLib::Path path : paths) {
        // if (ClipperLib::Orientation(path)) {
            GeoGerry::LinearRing border = path_to_ring(path);
            GeoGerry::Shape s(border);
            ms.border.push_back(s);
        // }
        // else {
        //     std::cout << "hole" << std::endl;
        //     ReversePath(path);
        //     GeoGerry::LinearRing hole = path_to_ring(path);
        //     ms.border[ms.border.size() - 1].holes.push_back(hole);
        // }
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

// geos::geom::GeometryFactory::Ptr global_factory;

// Point* create_point(long double x, long double y) {
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

// boost_polygon ring_to_boost_poly(LinearRing shape) {

//     /*
//         Converts a shape object into a boost polygon object
//         by looping over each point and manually adding it to a 
//         boost polygon using assign_points and vectors
//     */

//     boost_polygon poly;

//     // create vector of boost points
//     std::vector<boost_point> points;

//     for (coordinate c : shape.border) 
//         points.push_back(boost_point(c[0],c[1])),

//     assign_points(poly, points);
//     correct(poly);

//     return poly;
// }

// LinearRing boost_poly_to_ring(boost_polygon poly) {

//     /*
//         Convert from a boost polygon into a Shape object.
//         Loop over each point in the polygon, add it to the
//         shape's border.
//     */

//     coordinate_set b;
//     vector<boost_point> const& points = poly.outer();

//     for (std::vector<boost_point>::size_type i = 0; i < points.size(); ++i)
//         b.push_back({get<0>(points[i]), get<1>(points[i])});

//     LinearRing shape(b);
//     return shape;
// }

// Shape boost_poly_to_shape(boost_multi_polygon poly) {

//     /*
//         overload the poly_to_shape function to work with multipolygons
//         loop over each polygon, use same process to add to shape's border
//         !!! WARNING: Should update shape data structure to work with
//             multipolygon set. Not quite sure how to do this yet...
//     */

//     coordinate_set b;

//     for (boost_polygon p : poly) {
//         vector<boost_point> const& points = p.outer();

//         for (std::vector<boost_point>::size_type i = 0; i < points.size(); ++i) {
//             b.push_back({(float) get<0>(points[i]), (float) get<1>(points[i])});
//         }
//     }

//     Shape shape(b);
//     return shape;
// }

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

    for (Precinct p : state.precincts) {
        if (get_bordering(p, border));
    }
}
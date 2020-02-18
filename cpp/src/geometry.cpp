/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:               Sun, Jan 19
 
 Definition of useful functions for
 computational geometry. Basic 
 calculation, area, bordering - no
 algorithmic specific methods.
========================================*/

#include "../include/geometry.hpp"
#include "../include/shape.hpp"   // class definitions
#include "../include/gui.hpp"     // for the draw function

#define PI M_PI
const int c = pow(10, 7);

coordinate Shape::center() {
    /* 
        Returns the average {x,y} of a shape.
        In the future, could use centroid algorithm
        for determining center - may be a better measure of center
    */

    coordinate coords = { border[0][0], border[0][1] };  // initialize with first values
    
    // loop and add x and y to respective sums
    for ( int i = 1; i < border.size(); i++ ) {
        coords[0] += border[i][0];
        coords[1] += border[i][1];
    }

    // divide by number of elements to average
    coords[0] /= border.size();
    coords[1] /= border.size();

    return coords; // return averages
}

float get_distance(segment s) {
    // Distance formula on a segment array
    return sqrt(pow((s[2] - s[0]), 2) + pow((s[3] - s[1]), 2));
}

float get_distance(coordinate c0, coordinate c1) {
    // Distance formula on two separate points
    return sqrt(pow((c1[0] - c0[0]), 2) + pow((c1[1] - c0[1]), 2));
}

float Shape::area() {
    
    /*
        Returns the area of a shape, using latitude * long area
        An implementation of the shoelace theorem, found at 
        https://www.mathopenref.com/coordpolygonarea.html
    */

    float area = 0;
    int points = border.size() - 1; // last index of border

    for ( int i = 0; i < border.size(); i++ ) {
        area += (border[points][0] + border[i][0]) * (border[points][1] - border[i][1]);
        points = i;
    }

    return (area / 2);
}

float Shape::perimeter() {
    /*
        Returns the perimeter of a shape object using
        latitude and longitude coordinates by summing distance
        formula distances for all segments
    */

    float t = 0;
    for (segment s : get_segments())
        t += get_distance(s);    

    return t;
}

segment coords_to_seg(coordinate c1, coordinate c2) {
    // combines coordinates into a segment array
    return {c1[0], c1[1], c2[0], c2[1]};
}

segments Shape::get_segments() {
    /*
        Returns a vector of segments from the
        coordinate array of a Shape.border property
    */

    segments segs;
    
    for (int i = 0; i < border.size(); i++) {

        coordinate c1 = border[i];
        coordinate c2;

        if (i == border.size() - 1)
            c2 = border[0];
        else
            c2 = border[i + 1];

        segs.push_back(coords_to_seg(c1, c2));
    }

    return segs;
}

vector<float> calculate_line(segment s) {
    /*
        Use slope/intercept form and substituting
        coordinates in order to determine equation
        of a line segment
    */

    float m = (s[3] - s[1]) / (s[2] - s[0]);
    float b = -m * s[0] + s[1];

    return {m, b};
}

bool segments_overlap(segment s0, segment s1) {
    /*
        Returns whether or not two segments' bounding boxes
        overlap, meaning one of the extremes of a segment are
        within the range of the other's
    */
   
    if (s0[0] > s0[2])
        return (((s1[0] < s0[0]) && (s1[0] > s0[2])) || ((s1[2] < s0[0]) && (s1[2] > s0[2])) );
    else
        return (((s1[0] < s0[2]) && (s1[0] > s0[0])) || ((s1[2] < s0[2]) && (s1[2] > s0[0])) );
}

bool are_bordering(Shape s0, Shape s1) {
    // returns whether or not two shapes touch each other

    for (segment seg0 : s0.get_segments()) {
        for (segment seg1 : s1.get_segments()) {
            if (calculate_line(seg0) == calculate_line(seg1) && segments_overlap(seg0, seg1)) {
                return true;
            }
        }
    }

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
    
    // for (Precinct p : shape.precincts) {
    //     if (are_bordering(p, exterior_border)) {
    //         boundary_precincts.push_back(i);
    //     }
    //     i++;
    // }

    return boundary_precincts;
}



p_index_set get_bordering_shapes(vector<Shape> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if ( ( shapes[i] != shape ) && are_bordering(shapes[i], shape)) vec.push_back(i);
    
    return vec;
}


p_index_set get_bordering_shapes(vector<Precinct_Group> shapes, Shape shape) {
    /*
        returns set of indices corresponding to the Precinct_Groups that
        border with the Precinct_Group[index] shape.
    */

    p_index_set vec;
    
    for (p_index i = 0; i < shapes.size(); i++)
        if ( ( shapes[i] != shape ) && are_bordering(shapes[i], shape)) vec.push_back(i);
    
    return vec;
}


p_index_set get_bordering_precincts(Precinct_Group shape, int p_index) {
    return {1};
}

unit_interval compactness(Shape shape) {

    /*
        An implementation of the Schwartzberg compactness score.
        Returns the ratio of the perimeter of a shape to the
        circumference of a circle with the same area as that shape.
    */

    float circle_radius = sqrt(shape.area() / PI);
    float circumference = 2 * circle_radius * PI;

    return 1 / (shape.perimeter() / circumference);
}

bool is_colinear(segment s0, segment s1) {
    return (calculate_line(s0) == calculate_line(s1));
}

float get_standard_deviation_partisanship(Precinct_Group pg) {
    /*
        Returns the standard deviation of the partisanship
        ratio for a given array of precincts
    */

    vector<Precinct> p = pg.precincts;
    float mean = p[0].get_ratio();

    for (int i = 1; i < pg.precincts.size(); i++)
        mean += p[i].get_ratio();

    mean /= p.size();
    float dev_mean = pow(p[0].get_ratio() - mean, 2);

    for (int i = 1; i < p.size(); i++)
        dev_mean += pow(p[i].get_ratio() - mean, 2);

    return ((float) sqrt(dev_mean));
}

float get_median_partisanship(Precinct_Group pg) {
    /*
        Returns the median partisanship ratio
        for a given array of precincts
    */

    float median;
    vector<float> ratios;
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

GeometryFactory::Ptr global_factory; // for creating geometries

Multi_Shape generate_exterior_border(Precinct_Group precinct_group) {
    /*
        Get the exterior border of a shape with interior components.
        Equivalent to 'dissolve' in mapshaper - remove bordering edges
    */ 
    const GeometryFactory* newFactory;
    vector<Geometry*>* geoms;
    int x = 0;
    for (Precinct p : precinct_group.precincts) {
        cout << x << endl;
        geoms->push_back(shape_to_poly(p));
        x++;
    }


    MultiPolygon* polys = newFactory->createMultiPolygon(geoms);
    unique_ptr<Geometry> u = polys->getBoundary();//geounion::UnaryUnionOp::Union(polys);
    cout << "a" << endl;

    Multi_Shape border;
    for (int i = 0; i < u->getNumGeometries(); i++) {
        const Geometry* geo = u->getGeometryN(i);
        border.border.push_back(poly_to_shape(geo));
    }
    cout << "a" << endl;

    return border;
    // return poly_to_shape(u);
    // return clipper_mult_int_to_shape(solutions);
}

p_index State::get_addable_precinct(p_index_set available_precincts, p_index current_precinct) {
    /*
        A method for the initial generation of the
        communities algorithm - returns the next addable
        precinct to a given community, to avoid creating islands
    */

    p_index ret;
    return ret;
}

LinearRing* create_linearring(coordinate_set coords) {
    // We will use a coordinate list to build the linearring
    CoordinateArraySequence* cl = new CoordinateArraySequence();

    for (coordinate c : coords) {
        cl->add(Coordinate(c[0], c[1]));
    }

    LinearRing* lr = global_factory->createLinearRing(cl);
    return lr; // our LinearRing
}

Point* create_point(double x, double y) {
    /* given coordinates creates a point object */
    Coordinate c(x, y);
    Point* p = global_factory->createPoint(c);
    return p;
}

Geometry* shape_to_poly(Shape shape) {
    /*
        Creates a GEOS library polygon object from a
        given Shape object by looping through points
    */
   cout << "b" << endl;

    // blank holes vector
    vector<LinearRing*>* holes = new vector<LinearRing*>;
       cout << "b" << endl;

    // outer ring generated by looping through coordinates
    LinearRing* outer = create_linearring(shape.border);
       cout << "b" << endl;

    Polygon* poly = global_factory->createPolygon(outer, holes);
   cout << "b" << endl;

    return poly;
}

Shape poly_to_shape(const Geometry* path) {
    /*
        Creates a shape object from a clipper Path
        object by looping through points
    */

    Shape s;
    
    // write coordinates in path to vector of Coordinates
    std::unique_ptr <CoordinateSequence> points = path->getCoordinates();
    vector<Coordinate> coords;
    points->toVector(coords);
    
    for (Coordinate coord : coords) {
        coordinate nc = {(float) coord.x, (float) coord.y};
        s.border.push_back(nc);
    }

    return s;
}

Multi_Shape* multipoly_to_shape(MultiPolygon* paths) {
    /*
        Create a Multi_Shape object from a clipper Paths
        (multi path) object through nested iteration
    */

    Multi_Shape* ms;
    
    return ms;
}
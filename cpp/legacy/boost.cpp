boost_polygon ring_to_boost_poly(LinearRing shape) {
    /*
        Converts a shape object into a boost polygon object
        by looping over each point and manually adding it to a 
        boost polygon using assign_points and vectors
    */

    boost_polygon poly;

    // create vector of boost points
    std::vector<boost_point> points;

    for (coordinate c : shape.border) 
        points.push_back(boost_point(c[0],c[1])),

    assign_points(poly, points);
    correct(poly);

    return poly;
}


LinearRing boost_poly_to_ring(boost_polygon poly) {
    /*
        Convert from a boost polygon into a Polygon object.
        Loop over each point in the polygon, add it to the
        shape's border.
    */

    coordinate_set b;
    vector<boost_point> const& points = poly.outer();

    for (std::vector<boost_point>::size_type i = 0; i < points.size(); ++i)
        b.push_back({get<0>(points[i]), get<1>(points[i])});

    LinearRing shape(b);
    return shape;
}


Polygon boost_poly_to_shape(boost_multi_polygon poly) {
    /*
        overload the poly_to_shape function to work with multipolygons
        loop over each polygon, use same process to add to shape's border
        !!! WARNING: Should update shape data structure to work with
            multipolygon set. Not quite sure how to do this yet...
    */

    coordinate_set b;

    for (boost_polygon p : poly) {
        vector<boost_point> const& points = p.outer();

        for (std::vector<boost_point>::size_type i = 0; i < points.size(); ++i) {
            b.push_back({(float) get<0>(points[i]), (float) get<1>(points[i])});
        }
    }

    Polygon shape(b);
    return shape;
}
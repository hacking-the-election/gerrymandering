/*=======================================
 geometry.cpp:                  k-vernooy
 last modified:               Sun, Jan 19
 
 Definition of useful functions for
 computational geometry. Basic 
 calculation, area, bordering - no
 algorithmic specific methods.
========================================*/

#include "../include/shape.hpp"   // class definitions
#include "../include/gui.hpp"     // for the draw function
#include <math.h>                 // for rounding functions
#include <cmath>

#define PI M_PI

coordinate Shape::center() {
    // returns the average {x,y} of a shape
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

float Shape::area() {
    
    /*
        returns the area of a shape, using latitude * long area
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
        returns the perimeter of a shape object
        using latitude and longitude coordinates
    */

    float t = 0;

    for (segment s : get_segments()) {
        float d = sqrt(pow((s[2] - s[0]), 2) + pow((s[3] - s[1]), 2));
        t += d;
    }

    return t;
}

segment coords_to_seg(coordinate c1, coordinate c2) {
    return {c1[0], c1[1], c2[0], c2[1]};
}

segments Shape::get_segments() {
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
    float m = (s[3] - s[1]) / (s[2] - s[0]);
    float b = -m * s[0] + s[1];

    return {m, b};
}

bool segments_overlap(segment s0, segment s1) {
    if (s0[0] > s0[2])
        return (((s1[0] < s0[0]) && (s1[0] > s0[2])) || ((s1[2] < s0[0]) && (s1[2] > s0[2])) );
    else
        return (((s1[0] < s0[2]) && (s1[0] > s0[0])) || ((s1[2] < s0[2]) && (s1[2] > s0[0])) );
}

p_index_set get_boundary_precincts(Precinct_Group shape) {
   
    /*
        returns an array of indices that correspond
        to precincts on the outer edge of a Precinct Group
    */

    p_index_set boundary_precincts;

    // iterate over each precinct
    for (int index = 0; index < shape.precincts.size(); index++) {
        segments border = shape.precincts[index].get_segments();
        bool is_border_precinct = true;
        
        int line_index = 0;
        // iterate over each line in the precinct
        while ((is_border_precinct) && (line_index < border.size())) {
            vector<float> equation = calculate_line(border[line_index]);
            int index_2 = 0;
            bool line_has_no_borders = false;

            // iterate over all other precincts
            while ((!line_has_no_borders) && (index_2 < shape.precincts.size())) {
                // skip when we're comparing the same precinct
                if (index_2 != index) {
                    // the other precinct to compare to
                    segments border_compare = shape.precincts[index_2].get_segments;

                    for (segment line_compare : border_compare) {
                        // iterate over all lines in the compare precinct
                        vector<float> equation_compare = calculate_line(line_compare);
                        
                        if ((equation_compare == equation) && (segments_overlap(line, line_compare))) {
                            is_border_precinct = false;
                        }
                    }
                }

                index_2++;
            }

            line_index++;
        }

        if (is_border_precinct) {
            boundary_precincts.push_back(index);
        }
    }

    return boundary_precincts;
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

    return 1/(shape.perimeter() / circumference);
}

coordinate_set generate_exterior_border(Precinct_Group precinct_group) {
    p_index_set boundary_precincts = get_boundary_precincts(precinct_group);
    
    for (p_index i : boundary_precincts) {

        Precinct precinct = precinct_group.precincts[i];
        coordinate_set border = precinct.border;

        for (int i = 0; i < border.size(); i++) {
            
            coordinate c0 = border[i];
            coordinate c1;

            if (i == border.size())
                c1 = border[0];
            else 
                c1 = border[i + 1];
            
            // calculate_line(c0, c1);
        }
    }
}
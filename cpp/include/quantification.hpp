/*=======================================
 quantification.hpp:            k-vernooy
 last modified:               Fri May 22
 
 Determines how gerrymandered a district
 is given a community map.
========================================*/

#pragma once

#include "shape.hpp"
#include "geometry.hpp"
#include "community.hpp"
#include "canvas.hpp"

namespace hte {
    namespace Geometry {
        std::array<double, 2> get_quantification(Graph&, Communities&, Multi_Polygon);
        double get_attr_from_mask(Precinct_Group, Multi_Polygon, double (*)(Precinct&));
        double collapse_vals(double a, double b);
    }
}

/*=======================================
 generate_communities.cpp:      k-vernooy
 last modified:                  Mon, Jun 22
 
 Run community generation algorithm and 
 print coordinates as geojson for a given
 state object
========================================*/

#include <boost/filesystem.hpp>
#include <iostream>
#include <chrono>

#include "../include/hte.h"

using namespace boost::filesystem;
using namespace std;
using namespace hte;


double GetCompactnessToMinimize(Communities& c) {
    return (Average(c, GetPreciseCompactness));
}

int main(int argc, char* argv[]) {
    State state = State::fromFile(argv[1]);
    Communities cs = KargerStein(state.network, stoi(argv[2]));
    for (int i = 0; i < cs.size(); i++) {
        cs[i].resetShape(state.network);
    }

    Canvas c(900, 900);
    c.addOutlines(ToOutline(cs));
    c.drawToWindow();
    c.clear();

    GradientDescentOptimization(state.network, cs, GetCompactnessToMinimize);
    c.addOutlines(ToOutline(cs));
    c.drawToWindow();
    return 0;
}

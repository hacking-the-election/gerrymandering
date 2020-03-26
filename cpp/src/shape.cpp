/*=======================================
 shape.cpp:                     k-vernooy
 last modified:                Thu Mar 12
 
 Definition of generic methods for shapes, 
 precincts, districts and states that do
 not include geometric and/or mathematical
 algorithms (see geometry.cpp).
========================================*/

#include "../include/shape.hpp"     // class definitions
#include "../include/geometry.hpp"  // class definitions
#include "../include/util.hpp"      // split and join

#define REP  // use rep / (dem + rep) as ratio

double GeoGerry::Precinct::get_ratio() {
    // retrieve ratio from precinct
    if (rep == 0 && dem == 0)
        return -1;

    #ifdef REP
        return (double) rep / ((double) dem + (double) rep);
    #endif
    #ifndef REP
        return dem / (dem + rep);
    #endif
}


double GeoGerry::Precinct_Group::get_ratio() {
    double average = 0;
    for (Precinct p : precincts)
        average += p.get_ratio();
    average /= precincts.size();

    return average;
}


std::vector<int> GeoGerry::Precinct::get_voter_data() {
    // get vector of voting data
    return {dem, rep};
}


int GeoGerry::Precinct_Group::get_population() {
    // Returns total population in a Precinct_Group

    int total = 0;
    for (GeoGerry::Precinct p : precincts)
        total += p.pop;

    return total;
}


void GeoGerry::Precinct_Group::remove_precinct(GeoGerry::Precinct pre) {
    /*
        @desc: Removes a precinct from a Precinct Group and updates the border with a difference
        @params: `Precinct` pre: Precinct to be removed
        @return: none
    */

    if (std::find(precincts.begin(), precincts.end(), pre) != precincts.end()) {
        precincts.erase(std::remove(precincts.begin(), precincts.end(), pre), precincts.end());

        ClipperLib::Paths subj;
        for (Shape shape : this->border)
            subj.push_back(ring_to_path(shape.hull));

        ClipperLib::Paths clip;
        clip.push_back(ring_to_path(pre.hull));

        ClipperLib::Paths solutions;
        ClipperLib::Clipper c; // the executor

        // execute union on paths array
        c.AddPaths(subj, ClipperLib::ptSubject, true);
        c.AddPaths(clip, ClipperLib::ptClip, true);
        c.Execute(ClipperLib::ctDifference, solutions, ClipperLib::pftNonZero);

        this->border = paths_to_multi_shape(solutions).border;
    }
    else {
        throw GeoGerry::Exceptions::PrecinctNotInGroup();
    }
}


void GeoGerry::Precinct_Group::add_precinct(GeoGerry::Precinct pre) {
    /*
        @desc: Adds a precinct to a Precinct Group and updates the border with a union
        @params: `Precinct` pre: Precinct to be added
        @return: none
    */
   
    precincts.push_back(pre);
    if (border.size() == 0)
        border.push_back(pre.hull);
    else {
        ClipperLib::Paths subj;
        for (Shape shape : border)
            subj.push_back(ring_to_path(shape.hull));

        ClipperLib::Paths clip;
        clip.push_back(ring_to_path(pre.hull));

        ClipperLib::Paths solutions;
        ClipperLib::Clipper c; // the executor

        // execute union on paths array
        c.AddPaths(subj, ClipperLib::ptSubject, true);
        c.AddPaths(clip, ClipperLib::ptClip, true);
        c.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftNonZero);

        this->border = paths_to_multi_shape(solutions).border;
    }
}


std::string GeoGerry::Community::save_frame() {
    /*
        @desc:
            Saves a community configuration into a string by using
            the precinct id's with a comma separated list.
        @params: none
        @return: `string` saved communities
    */

    std::string line;
    for (Precinct p : precincts) {
        line += "\"" + p.shape_id + "\", ";
    }

    line = line.substr(0, line.size() - 2);
    // cout << line << endl;
    return line;
}


GeoGerry::Communities GeoGerry::Community::load_frame(std::string read_path, State precinct_list) {
    /*
        @desc:
            Given file path and precinct reference, reads a saved
            community configuration into an array of communities
        @params:
            `string` read_path: path to the saved community frame
            `GeoGerry::State` precinct_list: reference to get precinct geodata given id's
        @return: `Communities` loaded community array
    */

    Communities cs;
    std::string file = readf(read_path);
    std::stringstream fs(file);
    std::string line;

    while (getline(fs, line)) {
        Community c;
        std::vector<std::string> vals = split(line, "\"");

        for (std::string v : vals) {
            if (v != ", ") { // v contains a precinct id
                for (Precinct p : precinct_list.precincts) {
                    if (p.shape_id == v) {
                        c.add_precinct_n(p);
                    }
                } 
            }
        }

        cs.push_back(c);
    }

    return cs;
}

bool GeoGerry::operator== (GeoGerry::LinearRing l1, GeoGerry::LinearRing l2) {
    return (l1.border == l2.border);
}

bool GeoGerry::operator!= (GeoGerry::LinearRing l1, GeoGerry::LinearRing l2) {
    return (l1.border != l2.border);
}

bool GeoGerry::operator== (GeoGerry::Shape p1, GeoGerry::Shape p2) {
    return (p1.hull == p2.hull && p1.holes == p2.holes);
}

bool GeoGerry::operator!= (GeoGerry::Shape p1, GeoGerry::Shape p2) {
    return (p1.hull != p2.hull || p1.holes != p2.holes);
}

bool GeoGerry::operator== (GeoGerry::Precinct p1, GeoGerry::Precinct p2) {
    return (p1.hull == p2.hull && p1.holes == p2.holes && p1.dem == p2.dem && p1.rep == p2.rep && p1.pop == p2.pop);
}

bool GeoGerry::operator!= (GeoGerry::Precinct p1, GeoGerry::Precinct p2) {
    return (p1.hull != p2.hull || p1.holes != p2.holes || p1.dem != p2.dem || p1.rep != p2.rep || p1.pop != p2.pop);
}

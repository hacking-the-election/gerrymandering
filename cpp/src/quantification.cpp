/*=======================================
 quantification.cpp:            k-vernooy
 last modified:              Fri, Jun 19
 
 Determines how gerrymandered a district
 is given a community map
========================================*/

#include <iostream>
#include "../include/hte.h"

using namespace std;
using namespace hte;


double hte::Algorithm::CollapseVals(double a, double b) {
    assert((a <= 1.0 && a >= 0.0));
    assert((b <= 1.0 && b >= 0.0));
    return (a * (2.0 * (0.5 - b)));
}


double hte::Algorithm::GetPopulationFromMask(Data::PrecinctGroup pg, Geometry::MultiPolygon mp) {
    double pop = 0;
    Geometry::BoundingBox bound = pg.getBoundingBox();
    for (Data::Precinct p : pg.precincts) {
        if (Geometry::GetBoundOverlap(p.getBoundingBox(), bound)) {
            // get the overlap between mp and p
            // create paths array from polygon
            ClipperLib::Path subj = Geometry::RingToPath(p.hull);

            ClipperLib::Paths clip;
            for (Geometry::Polygon p : mp.border)
                clip.push_back(Geometry::RingToPath(p.hull));

            ClipperLib::Paths solutions;
            ClipperLib::Clipper c; // the executor

            // execute union on paths array
            c.AddPath(subj, ClipperLib::ptSubject, true);
            c.AddPaths(clip, ClipperLib::ptClip, true);
            c.Execute(ClipperLib::ctIntersection, solutions, ClipperLib::pftNonZero);
            Geometry::MultiPolygon intersection = Geometry::PathsToMultiPolygon(solutions);
            double ratio = (abs(intersection.getSignedArea()) / abs(p.getSignedArea()));
            pop += (static_cast<double>(p.pop) * ratio);
        }
    }

    return pop;
}


std::map<Data::PoliticalParty, double> hte::Algorithm::GetPartisanshipFromMask(Data::PrecinctGroup pg, Geometry::MultiPolygon mp) {
    std::map<Data::PoliticalParty, double> partisanships;
    Geometry::BoundingBox bound = pg.getBoundingBox();

    for (auto& pair : pg.precincts[0].voterData) {
        partisanships[pair.first] = 0;
    }

    for (Data::Precinct p : pg.precincts) {
        if (Geometry::GetBoundOverlap(p.getBoundingBox(), bound)) {
            // get the overlap between mp and p
            // create paths array from polygon
            ClipperLib::Path subj = Geometry::RingToPath(p.hull);

            ClipperLib::Paths clip;
            for (Geometry::Polygon p : mp.border)
                clip.push_back(Geometry::RingToPath(p.hull));

            ClipperLib::Paths solutions;
            ClipperLib::Clipper c; // the executor

            // execute union on paths array
            c.AddPath(subj, ClipperLib::ptSubject, true);
            c.AddPaths(clip, ClipperLib::ptClip, true);
            c.Execute(ClipperLib::ctIntersection, solutions, ClipperLib::pftNonZero);
            Geometry::MultiPolygon intersection = Geometry::PathsToMultiPolygon(solutions);
            double ratio = (abs(intersection.getSignedArea()) / abs(p.getSignedArea()));

            for (auto& pair : p.voterData) {
                partisanships[pair.first] += (pair.second * ratio);
            }
        }
    }

    return partisanships;
}


std::map<Data::PoliticalParty, double> Algorithm::GetQuantification(Graph& graph, Communities& communities, Geometry::MultiPolygon district) {
    // determines how gerrymandered `district` is with the `communities` map.
    // communities had better already be updated.
    double val = 0.0;
    Geometry::BoundingBox db = district.getBoundingBox();
    // get all communities that overlap the current district

    
    Data::PrecinctGroup state;
    for (auto& pair : graph.vertices) {
        state.addPrecinct(*pair.second.precinct);
    }
    double districtPopulation = GetPopulationFromMask(state, district);

    int largestIndex = -1;
    int largestPop = -1;

    for (int i = 0; i < communities.size(); i++) {
        communities[i].resetShape(graph);
        if (Geometry::GetBoundOverlap(communities[i].shape.getBoundingBox(), db)) {
            // get intersection between the two shapes
            double pop = GetPopulationFromMask(communities[i].shape, district);
            if (pop > largestPop) {
                largestPop = pop;
                largestIndex = i;
            }
        }
    }

    // get the difference of the two shapes
    ClipperLib::Paths subj;
    for (Data::Precinct p : communities[largestIndex].shape.precincts) {
        subj.push_back(Geometry::RingToPath(p.hull));
    }

    ClipperLib::Paths clip;
    for (Geometry::Polygon p : district.border)
        clip.push_back(Geometry::RingToPath(p.hull));

    ClipperLib::Paths solutions;
    ClipperLib::Clipper c; // the executor

    // execute union on paths array
    c.AddPaths(subj, ClipperLib::ptSubject, true);
    c.AddPaths(clip, ClipperLib::ptClip, true);
    c.Execute(ClipperLib::ctDifference, solutions, ClipperLib::pftNonZero);
    Geometry::MultiPolygon popNotInDistrict = Geometry::PathsToMultiPolygon(solutions);
    std::map<Data::PoliticalParty, double> partisanships = GetPartisanshipFromMask(communities[largestIndex].shape, popNotInDistrict);

    double sum = 0;
    for (auto& pair : partisanships) {
        if (pair.first != Data::PoliticalParty::Total) {
            sum += pair.second;
        }
    }

    for (auto& pair : partisanships) {
        if (sum != 0) {
            partisanships[pair.first] /= sum;
        }
        else {
            partisanships[pair.first] = 0.5;   
        }
    }

    partisanships[Data::PoliticalParty::AbsoluteQuantification]
       = GetPopulationFromMask(communities[largestIndex].shape, popNotInDistrict) / communities[largestIndex].getPopulation();

    return partisanships;
}

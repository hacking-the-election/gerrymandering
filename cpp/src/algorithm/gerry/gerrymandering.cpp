/*===============================================
 community.cpp:                        k-vernooy
 last modified:                     Fri, Feb 28

 Definition of the gerrymandering quantification
 algorithm, given a district plan and base
 communities.

 Visit https://hacking-the-election.github.io for
 more information on this project.
 
 This algorithm has been implemented in C++ and in
 Python. If you're interested in contributing to the
 open source political movement, please contact us
 at: hacking.the.election@gmail.com
===============================================*/

#include <math.h>    // for rounding functions

#include "../../../include/shape.hpp"    // class definitions
#include "../../../include/util.hpp"     // array modification functions
#include "../../../include/geometry.hpp" // geometry modification, border functions
#include "../../../include/canvas.hpp" // geometry modification, border functions

using namespace std;
using namespace GeoGerry;
using namespace GeoDraw;


vector<unit_interval> State::quantify_gerrymandering(vector<Multi_Shape> districts, Communities base_communities) {
    // use 
    vector<unit_interval> vals;

    for (Multi_Shape district : districts) {
        unit_interval score = 0;
        vector<Community> inside = {};
        vector<Multi_Shape> intersections = {};

        for (Community base_community : base_communities) {
            ClipperLib::Paths subj;
            for (Shape s : district.border)
                for (ClipperLib::Path p : shape_to_paths(s))
                    subj.push_back(p);

            ClipperLib::Paths clip;
            for (Shape s : base_community.border)
                for (ClipperLib::Path p : shape_to_paths(s))
                    clip.push_back(p);
            
            ClipperLib::Paths solutions;
            ClipperLib::Clipper clipper; // the executor

            // execute union on paths array
            clipper.AddPaths(subj, ClipperLib::ptSubject, true);
            clipper.AddPaths(clip, ClipperLib::ptClip, true);
            clipper.Execute(ClipperLib::ctIntersection, solutions, ClipperLib::pftNonZero);

            if (solutions.size() > 0) {
                inside.push_back(base_community);
                Multi_Shape ms = paths_to_multi_shape(solutions);
                intersections.push_back(ms);
            }
        }

        vector<double> intersection_ratios;
        double largest = intersections[0].get_area();
        intersection_ratios.push_back(intersections[0].get_area() / district.get_area());

        for (int i = 1; i < intersections.size(); i++) {
            if (intersections[i].get_area() > largest) largest = intersections[i].get_area();
            intersection_ratios.push_back(intersections[i].get_area() / district.get_area());
        }

        score = 1 - (largest / district.get_area());
        cout << "Unmodified: " << score << endl;
        
        double average_community_area = base_communities.size();
        vector<double> weights;
        for (double ir : intersection_ratios) weights.push_back(ir / average_community_area);

        double weighted_average;
        double mult_sum = 0;
        double weight_sum = 0;

        for (double weight : weights) weight_sum += weight;
        for (int i = 0; i < weights.size(); i++)
            mult_sum += (weights[i] * inside[i].get_ratio());

        weighted_average = mult_sum / weight_sum;
        cout << weight_sum << endl;

        double part_factor = 0;
        for (int i = 0; i < weights.size(); i++)
            part_factor += weights[i] * pow((inside[i].get_ratio() - weighted_average), 2);
        part_factor /= weight_sum;

        cout << "part " << part_factor << endl;
        score *= part_factor;
        cout << "score " << score << endl;

        vals.push_back(score);
    }

    return vals;
}
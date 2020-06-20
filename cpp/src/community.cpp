/*===============================================
 community.cpp:                        k-vernooy
 last modified:                     Fri, Jun 19
 
 Definition of the community-generation algorithm
 quantifying gerrymandering and redistricting
 Detailed documentation can be found in the /docs
 root folder of this repository.
 
 Visit https://hacking-the-election.github.io for
 more information on this project.
 
 Our data sources for actually executing this
 algorithm can be seen at /data or on GitHub:
 https://github.com/hacking-the-election/data
 This algorithm has been implemented in C++ and in
 Python. If you're interested in contributing to
 the project, please contact us at
 hacking.the.election@gmail.com
================================================*/


#include <math.h>
#include <numeric>
#include <iostream>
#include <random>
#include <iomanip>

#include "../lib/Miniball.hpp"
#include "../include/hte.h"

using namespace std;
using namespace hte;
using namespace Geometry;
using namespace Graphics;
using namespace Data;
using namespace Algorithm;


#define VERBOSE 1
#define DEBUG 0
#define SPEED_OPT 0


class NodePtr {
    public:
        int id;
        int id_x;
        int degree;
    friend bool operator< (const NodePtr& l1, const NodePtr& l2);
};


bool operator< (const NodePtr& l1, const NodePtr& l2) {
    return (l1.degree < l2.degree);
}


void Community::resetShape(Graph& graph) {
    this->shape = PrecinctGroup();    
    for (int i = 0; i < this->vertices.size(); i++) {
        this->shape.addPrecinct(*graph.vertices[(vertices.begin() + i).key()].precinct);
    }
}


int hte::Algorithm::GetNumPrecinctsChanged(Graph& before, Graph& after) {
    int tot = 0;
    for (int i = 0; i < before.vertices.size(); i++) {
        if ((before.vertices.begin() + i).value().community != (after.vertices.begin() + i).value().community) {
            tot++;
        }
    }
    return tot;
}


int hte::Algorithm::Community::getPopulation() {
    int sum = 0;
    for (Precinct p : shape.precincts) sum += p.pop;
    return sum;
}


void Community::removeNode(int id) {
    this->removeEdgesTo(id);
    this->shape.removePrecinct(*vertices[id].precinct);
    this->vertices.erase(id);
}


void Community::addNode(Node& node) {
    this->vertices.insert({node.id, node});
    for (Edge edge : node.edges) {
        if (vertices.find(edge[1]) != vertices.end()) {
            this->addEdge(edge);
        }
    }

    this->shape.addPrecinct(*node.precinct);
}


void hte::Algorithm::SaveCommunitiesToFile(Communities cs, std::string out) {
    string file = "[";
    for (Community c : cs) {
        file += "[";
        for (Precinct p : c.shape.precincts)
            file += "'" + p.shapeId + "', ";
        file.pop_back(); file.pop_back();
        file += "], ";
    }

    file.pop_back(); file.pop_back();
    file += "]";
    Util::WriteFile(file, out);
}


Communities hte::Algorithm::LoadCommunitiesWithQuantification(std::string path, Graph& g, std::string tsv) {
    string file = Util::ReadFile(path);
    file = file.substr(1, file.size() - 3);
    vector<string> strs = Util::Split(file, "[");
    Communities communities(strs.size() - 1);
    
    for (int x = 0; x < strs.size(); x++) {
        if (strs[x] != "" && strs[x] != " ") {
            vector<string> s = Util::Split(strs[x], ",");
            for (int i = 0; i < s.size(); i++) {
                if (s[i] != " " && s[i] != "") {
                    string mod = s[i];
                    mod = mod.substr(mod.find("'") + 1, mod.size() - mod.find("'") - 1);
                    mod = mod.substr(0, mod.find("'"));

                    for (int n = 0; n < g.vertices.size(); n++) {
                        if ((g.vertices.begin() + n).value().precinct->shapeId == mod) {
                            communities[x - 1].addNode((g.vertices.begin() + n).value());
                            g.vertices[(g.vertices.begin() + n).key()].community = x - 1;
                        }
                    }
                }
            }
        }
    }

    vector<vector<double> > quant = LoadQuantification(tsv);
    for (int i = 0; i < quant[0].size(); i++) {
        communities[i].quantification = quant[0][i];
        communities[i].partisanQuantification = quant[1][i];
    }

    return communities;
}


vector<vector<double> > hte::Algorithm::LoadQuantification(std::string tsv) {
    vector<vector<double> > quant;
    string quantificationTsv = Util::ReadFile(tsv);
    string abs = Util::Split(quantificationTsv, "\n")[0];
    string part = Util::Split(quantificationTsv, "\n")[1];

    vector<double> q;
    for (string t : Util::Split(abs, "\t")) {
        q.push_back(stod(t));
    }
    quant.push_back(q);

    q.clear();
    for (string t : Util::Split(part, "\t")) {
        q.push_back(stod(t));
    }
    quant.push_back(q);
    return quant;
}


Communities hte::Algorithm::LoadCommunitiesFromFile(std::string path, Graph& g) {
    
    string file = Util::ReadFile(path);
    file = file.substr(1, file.size() - 3);
    vector<string> strs = Util::Split(file, "[");
    Communities communities(strs.size() - 1);
    
    for (int x = 0; x < strs.size(); x++) {
        if (strs[x] != "" && strs[x] != " ") {
            vector<string> s = Util::Split(strs[x], ",");
            for (int i = 0; i < s.size(); i++) {
                if (s[i] != " " && s[i] != "") {
                    string mod = s[i];
                    mod = mod.substr(mod.find("'") + 1, mod.size() - mod.find("'") - 1);
                    mod = mod.substr(0, mod.find("'"));

                    for (int n = 0; n < g.vertices.size(); n++) {
                        if ((g.vertices.begin() + n).value().precinct->shapeId == mod) {
                            communities[x - 1].addNode((g.vertices.begin() + n).value());
                            g.vertices[(g.vertices.begin() + n).key()].community = x - 1;
                        }
                    }
                }
            }
        }
    }

    return communities;
}


int GetPopulation(Communities& c) {
    int sum = 0;
    for (Community cs : c) {
        sum += cs.getPopulation();
    }
    return sum;
}


double hte::Algorithm::GetDistanceFromPop(Communities& cs, double threshold) {
    double av = 0;
    double av_pop = GetPopulation(cs) / cs.size();
    for (Community& c : cs) {
        av += (1.0 - abs(1.0 - (double)c.getPopulation() / av_pop));
    }
    double r = av / cs.size();

    if (r >= threshold) return 1.0;
    else return r;
}


double hte::Algorithm::GetCompactness(Community& community) {
    /*
        @desc: finds the reock compactness of a `Community`
        @params: `Community&` community: community object to find compactness of
        @return: `double` reock compactness
        @ref: https://fisherzachary.github.io/public/r-output.html
    */

    
    // return (0.4);
    Point2d center = community.shape.getCentroid(); 
    double farthest = 0;

    for (Precinct& p : community.shape.precincts) {
        if (pow(center.x - p.getCentroid().x, 2) + pow(center.y - p.getCentroid().y, 2) > farthest) {
            farthest = pow(center.x - p.getCentroid().x, 2) + pow(center.y - p.getCentroid().y, 2);
        }
    }

    return (community.shape.getSignedArea() / (farthest * PI));
}


double hte::Algorithm::GetPreciseCompactness(Community& community) {
    /*
        @desc: finds the reock compactness of a `Community`
        @params: `Community&` community: community object to find compactness of
        @return: `double` reock compactness
        @ref: https://fisherzachary.github.io/public/r-output.html
    */

    Point2dVec lp;
    vector<vector<double> > p;

    for (Precinct pre : community.shape.precincts) {
        lp.insert(lp.end(), pre.hull.border.begin(), pre.hull.border.end());
    }

    p.reserve(lp.size());

    for (int i = 0; i < lp.size(); i++)
        p.emplace_back(vector<double>{static_cast<double>(lp[i].x), static_cast<double>(lp[i].)y}));

    lp.clear();

    MB mb (2, p.begin(), p.end());
    return (double)community.shape.getArea() / (mb.squared_radius() * PI);

}


double hte::Algorithm::GetPartisanshipStdev(Community& community) {
    double average = 0;
    map<PoliticalParty, vector<int> > total_data;

    for (auto& pair : community.vertices) {
        // for each vertex
        for (auto& p2 : pair.second.precinct->voterData) {
            // for each party
            if (p2.first != PoliticalParty::Total && p2.second >= 0) {
                total_data[p2.first].push_back(p2.second);
            }
        }
    }

    for (auto& pair : total_data) {
        average += Util::GetStdev(pair.second);
    }
    
    return (average / total_data.size());
}


double hte::Algorithm::GetScalarizedMetric(Communities& cs) {
    return ((Average(cs, GetPreciseCompactness) + GetDistanceFromPop(cs, 0.99)) / 2);
}


bool ExchangePrecinct(Graph& g, Communities& cs, int node_to_take, int community_to_take) {
    // moves a node from its community into `community_to_take`
    int nttc = g.vertices[node_to_take].community;
    if (cs[nttc].vertices.size() == 1) {
        return false;
    }

    if (!RemoveEdgesTo(cs[nttc], node_to_take).isConnected()) return false;

    cs[nttc].removeNode(node_to_take);
    cs[community_to_take].addNode(g.vertices[node_to_take]);
    g.vertices[node_to_take].community = community_to_take;
    return true;
}


vector<int> get_takeable_precincts(Graph& g, Communities& c, int in) {
    vector<int> takeable = {};

    for (auto& pair : c[in].vertices) {
        for (Edge& e : g.vertices[pair.first].edges) {
            if (g.vertices[e[1]].community != in) {
                if (std::find(takeable.begin(), takeable.end(), e[1]) == takeable.end()) {
                    takeable.push_back(e[1]);
                }
            }
        }
    }

    return takeable;
}


vector<vector<int> > get_giveable_precincts(Graph& g, Communities& c, int in) {
    /*
        @desc: Find precincts bordering another community
        @params:
            `Graph&` g: graph for reference
            `Communities&` c: communities to check
            `int` in: index of community to check

        @return: `vector<array<int, 2>` giveable precincts, and the community to give to
    */

    vector<vector<int> > giveable = {};

    for (auto& pair : c[in].vertices) {
        for (Edge& edge : g.vertices[pair.first].edges) {
            // if the node borders a precinct not in the community
            if (g.vertices[edge[1]].community != in) {
                giveable.push_back({
                    pair.first, g.vertices[edge[1]].community
                });
                break;
            }
        }
    }

    return giveable;
}


double hte::Algorithm::Average(Communities& communities, double (*measure)(Community&)) {
    /*
        @desc: find average `measure` of the `communities`

        @params:
            `Communities&` communities: list of communities measure
            `double (*measure)(Community&)`: pointer to function of type double with param `Community&`
        
        @return: `double` average measure
    */

    double sum = 0;
    for (int i = 0; i < communities.size(); i++) sum += measure(communities[i]);
    return (sum / communities.size());
}


vector<array<int, 2> > GetAllExchanges(Graph& g, Communities& cs) {
    vector<array<int, 2> > exchanges = {};
    for (auto& pair : g.vertices) {
        vector<int> dc = {};
        for (Edge e : pair.second.edges) {
            if (g.vertices[e[1]].community != pair.second.community) {
                if (std::find(dc.begin(), dc.end(), g.vertices[e[1]].community) == dc.end()) {
                    dc.push_back(g.vertices[e[1]].community);
                    exchanges.push_back({pair.second.id, g.vertices[e[1]].community});
                }
            }
        }
    }

    return exchanges;
}


void hte::Algorithm::GradientDescentOptimization(Graph& g, Communities& cs, double (*measure)(Communities&)) {
    while (true) {
        Graph before = g;
        array<int, 2> bestExchange;
        double largestMeasure = measure(cs);
        bool canBeBetter = false;
        vector<array<int, 2> > exchanges = GetAllExchanges(g, cs);

        for (array<int, 2> exchangeP : exchanges) {
            int initCommunity = g.vertices[exchangeP[0]].community;
            if (ExchangePrecinct(g, cs, exchangeP[0], exchangeP[1])) {
                double m = measure(cs);
                if (m > largestMeasure) {
                    largestMeasure = m;
                    bestExchange = exchangeP;
                    canBeBetter = true;
                }
            }
            ExchangePrecinct(g, cs, exchangeP[0], initCommunity);
        }

        if (!canBeBetter) {
            cout << largestMeasure << endl;
            break;
        }

        ExchangePrecinct(g, cs, bestExchange[0], bestExchange[1]);
    }
}


void hte::Algorithm::SimulatedAnnealingOptimization(Graph& g, Communities& cs, double (*measure)(Community&)) {
    double Ec = Average(cs, measure);
    double Tmax = 30, Tmin = 0, T = Tmax;
    double Cool = 0.99976;
    int Epochs = 40000, Epoch = 0;

    while (Epoch < Epochs) {
        Epoch++;
        vector<array<int, 2> > allExchanges = GetAllExchanges(g, cs);
        array<int, 2> chosenExchange;
        int initCommunity;
        int choice = -1;

        do {
            int newChoice = Util::RandInt(0, allExchanges.size());
            while (choice == newChoice) {
                newChoice = Util::RandInt(0, allExchanges.size());
            }
            choice = newChoice;
            chosenExchange = allExchanges[newChoice];
            initCommunity = g.vertices[chosenExchange[0]].community;
        } while (!ExchangePrecinct(g, cs, chosenExchange[0], chosenExchange[1]));

        double En = Average(cs, measure);
        double x = Util::RandUnitInterval();

        if (En < Ec) {
            Ec = En;
            cout << T << "\t" << Ec << endl;
        }
        else if ((T / Tmax) > x) {
            cout << T << "\t" << Ec << endl;
            Ec = En;
        }
        else {
            ExchangePrecinct(g, cs, chosenExchange[0], initCommunity);
        }

        T *= Cool;
    }
}


//  Create a randomly partitioned graph with the Karger-Stein algorithm
Communities hte::Algorithm::KargerStein(Graph& g1, int nCommunities) {
    // initizize copy of graph
    Graph g = g1;
    const int MAX_SEARCH = 100;

    while (g.vertices.size() > nCommunities) {
        Edge min;
        int tMin = 1000000;

        for (int i = 0; i < g.vertices.size(); i++) {
            // choose random node and edge of that node
            int nodeToRemoveId = (g.vertices.begin() + Util::RandInt(0, g.vertices.size() - 1)).key();
            int nodeToCollapseId = g.vertices[nodeToRemoveId].edges[Util::RandInt(0, g.vertices[nodeToRemoveId].edges.size() - 1)][1];
            int t = g.vertices[nodeToRemoveId].collapsed.size() + g.vertices[nodeToCollapseId].collapsed.size();

            // if combining the `collapsed` vector of the nodes
            // on this edge is smaller than min, add new min
            if (t < tMin) {
                min = {nodeToRemoveId, nodeToCollapseId};
                tMin = t;
            }
            
            // create max cap for searching for min
            if (i == MAX_SEARCH) break;
        }


        // add the edges from node to remove to node to collapse
        for (Edge edge : g.vertices[min[0]].edges) {
            if (edge[1] != min[1]) {
                g.addEdge({edge[1], min[1]});
            }
        }

        // update collapsed of node_to_collapse
        g.vertices[min[1]].collapsed.push_back(min[0]);

        for (int c : g.vertices[min[0]].collapsed) {
            if (find(g.vertices[min[1]].collapsed.begin(), g.vertices[min[1]].collapsed.end(), c) == g.vertices[min[1]].collapsed.end()) {
                g.vertices[min[1]].collapsed.push_back(c);
            }
        }

        // remove node_to_remove
        g.removeNode(min[0]);
    }

    // create vector of size `n_communities`
    Communities communities(nCommunities);
    
    for (int i = 0; i < g.vertices.size(); i++) {
        // update communities with precincts according to `collapsed` vectors
        (g.vertices.begin() + i).value().collapsed.push_back((g.vertices.begin() + i).key());
        communities[i].vertices = g1.getInducedSubgraph((g.vertices.begin() + i).value().collapsed).vertices;

        for (int x : (g.vertices.begin() + i).value().collapsed) {
            g1.vertices[x].community = i;
        }
    }

    return communities;
}

#include "../include/hte_common.h"


// void Community::resetShape(Graph& graph) {
//     this->shape = PrecinctGroup();    
//     for (int i = 0; i < this->vertices.size(); i++) {
//         this->shape.addPrecinct(*graph.vertices[(vertices.begin() + i).key()].precinct);
//     }
// }


// int hte::GetNumPrecinctsChanged(Graph& before, Graph& after) {
//     int tot = 0;
//     for (int i = 0; i < before.vertices.size(); i++) {
    
//         if ((before.vertices.begin() + i).value().community != (after.vertices.begin() + i).value().community) {
//             tot++;
//         }
//     }
//     return tot;
// }


// int hte::Community::getPopulation() {
//     int sum = 0;
//     for (Precinct p : shape.precincts) sum += p.pop;
//     return sum;
// }


// void Community::removeNode(int id) {
//     this->removeEdgesTo(id);
//     this->shape.removePrecinct(*vertices[id].precinct);
//     this->vertices.erase(id);
// }


// void Community::addNode(Node& node) {
//     this->vertices.insert({node.id, node});
//     for (Edge edge : node.edges) {
//         if (vertices.find(edge[1]) != vertices.end()) {
//             this->addEdge(edge);
//         }
//     }

//     this->shape.addPrecinct(*node.precinct);
// }


// void hte::SaveCommunitiesToFile(Communities cs, std::string out) {
//     string file = "[";
//     for (Community c : cs) {
//         file += "[";
//         for (Precinct p : c.shape.precincts)
//             file += "'" + p.shapeId + "', ";
//         file.pop_back(); file.pop_back();
//         file += "], ";
//     }

//     file.pop_back(); file.pop_back();
//     file += "]";
//     WriteFile(file, out);
// }


// double hte::GetDistanceFromPop(Communities& cs, double threshold) {
//     double av = 0;
//     double av_pop = GetPopulation(cs) / cs.size();
//     for (Community& c : cs) {
//         av += (1.0 - abs(1.0 - (double)c.getPopulation() / av_pop));
//     }
//     double r = av / cs.size();

//     if (r >= threshold) return 1.0;
//     else return r;
// }


// double hte::GetPreciseCompactness(Community& community) {
//     /*
//         @desc: finds the reock compactness of a `Community`
//         @params: `Community&` community: community object to find compactness of
//         @return: `double` reock compactness
//         @ref: https://fisherzachary.github.io/public/r-output.html
//     */

//     Point2dVec lp;
//     vector<vector<double> > p;
//     for (Precinct pre : community.shape.precincts) {
//         lp.insert(lp.end(), pre.hull.border.begin(), pre.hull.border.end());
//     }

//     p.reserve(lp.size());
//     for (int i = 0; i < lp.size(); i++)
//         p.emplace_back(vector<double>{static_cast<double>(lp[i].x), static_cast<double>(lp[i].y)});
//     lp.clear();

//     MB mb (2, p.begin(), p.end());
//     return static_cast<double>(community.shape.getArea()) / (mb.squared_radius() * PI);
// }


// double hte::GetPartisanshipStdev(Community& community) {
//     double average = 0;
//     map<PoliticalParty, vector<int> > total_data;

//     for (auto& pair : community.vertices) {
//         // for each vertex
//         for (auto& p2 : pair.second.precinct->voterData) {
//             // for each party
//             if (p2.first != PoliticalParty::Total && p2.second >= 0) {
//                 total_data[p2.first].push_back(p2.second);
//             }
//         }
//     }

//     for (auto& pair : total_data) {
//         average += GetStdev(pair.second);
//     }
    
//     return (average / total_data.size());
// }


// double hte::GetScalarizedMetric(Communities& cs) {
//     return ((Average(cs, GetPreciseCompactness) + GetDistanceFromPop(cs, 0.99)) / 2);
// }


// bool hte::ExchangePrecinct(Graph& g, Communities& cs, int node_to_take, int community_to_take) {
//     int nttc = g.vertices[node_to_take].community;
//     if (cs[nttc].vertices.size() == 1) {
//         return false;
//     }

//     if (!RemoveEdgesTo(cs[nttc], node_to_take).isConnected()) return false;

//     cs[nttc].removeNode(node_to_take);
//     cs[community_to_take].addNode(g.vertices[node_to_take]);
//     g.vertices[node_to_take].community = community_to_take;
//     return true;
// }


// double hte::Average(Communities& communities, double (*measure)(Community&)) {
//     double sum = 0;
//     for (int i = 0; i < communities.size(); i++) sum += measure(communities[i]);
//     return (sum / communities.size());
// }


// vector<array<int, 2> > hte::GetAllExchanges(Graph& g, Communities& cs) {
//     vector<array<int, 2> > exchanges = {};
//     for (auto& pair : g.vertices) {
//         vector<int> dc = {};
//         for (Edge e : pair.second.edges) {
//             if (g.vertices[e[1]].community != pair.second.community) {
//                 if (std::find(dc.begin(), dc.end(), g.vertices[e[1]].community) == dc.end()) {
//                     dc.push_back(g.vertices[e[1]].community);
//                     exchanges.push_back({pair.second.id, g.vertices[e[1]].community});
//                 }
//             }
//         }
//     }

//     return exchanges;
// }


// void hte::GradientDescentOptimization(Graph& g, Communities& cs, double (*measure)(Communities&)) {
//     while (true) {
//         Graph before = g;
//         array<int, 2> bestExchange;
//         double largestMeasure = measure(cs);
//         bool canBeBetter = false;
//         vector<array<int, 2> > exchanges = GetAllExchanges(g, cs);

//         for (array<int, 2> exchangeP : exchanges) {
//             int initCommunity = g.vertices[exchangeP[0]].community;
//             if (ExchangePrecinct(g, cs, exchangeP[0], exchangeP[1])) {
//                 double m = measure(cs);
//                 if (m > largestMeasure) {
//                     largestMeasure = m;
//                     bestExchange = exchangeP;
//                     canBeBetter = true;
//                 }
//             }
//             ExchangePrecinct(g, cs, exchangeP[0], initCommunity);
//         }

//         if (!canBeBetter) {
//             cout << largestMeasure << endl;
//             break;
//         }

//         cout << largestMeasure << endl;
//         ExchangePrecinct(g, cs, bestExchange[0], bestExchange[1]);
//     }
// }


// void hte::SimulatedAnnealingOptimization(AdjacencyListGraph<std::shared_ptr<GeoUnit>, int>& g, Communities& cs, double (*measure)(Community&)) {
//     double Ec = Average(cs, measure);

//     double Tmax = 30, Tmin = 0, T = Tmax;
//     double Cool = 0.99976;
//     int Epochs = 40000, Epoch = 0;

//     while (Epoch < Epochs) {
//         Epoch++;
//         vector<array<int, 2> > allExchanges = GetAllExchanges(g, cs);
//         array<int, 2> chosenExchange;
//         int initCommunity;
//         int choice = -1;

//         do {
//             int newChoice = RandInt(0, allExchanges.size());
//             while (choice == newChoice) {
//                 newChoice = RandInt(0, allExchanges.size());
//             }
//             choice = newChoice;
//             chosenExchange = allExchanges[newChoice];
//             initCommunity = g.vertices[chosenExchange[0]].community;
//         } while (!ExchangePrecinct(g, cs, chosenExchange[0], chosenExchange[1]));

//         double En = Average(cs, measure);
//         double x = RandUnitInterval();

//         if (En < Ec) {
//             Ec = En;
//             cout << T << "\t" << Ec << endl;
//         }
//         else if ((T / Tmax) > x) {
//             cout << T << "\t" << Ec << endl;
//             Ec = En;
//         }
//         else {
//             ExchangePrecinct(g, cs, chosenExchange[0], initCommunity);
//         }

//         T *= Cool;
//     }
// }


// //  Create a randomly partitioned graph with the Karger-Stein algorithm
// Communities hte::KargerStein(Graph& g1, int nCommunities) {
//     // initizize copy of graph
//     Graph g = g1;
//     const int MAX_SEARCH = 100;

//     while (g.vertices.size() > nCommunities) {
//         Edge min;
//         int tMin = 1000000;

//         for (int i = 0; i < g.vertices.size(); i++) {
//             // choose random node and edge of that node
//             int nodeToRemoveId = (g.vertices.begin() + RandInt(0, g.vertices.size() - 1)).key();
//             int nodeToCollapseId = g.vertices[nodeToRemoveId].edges[RandInt(0, g.vertices[nodeToRemoveId].edges.size() - 1)][1];
//             int t = g.vertices[nodeToRemoveId].collapsed.size() + g.vertices[nodeToCollapseId].collapsed.size();

//             // if combining the `collapsed` vector of the nodes
//             // on this edge is smaller than min, add new min
//             if (t < tMin) {
//                 min = {nodeToRemoveId, nodeToCollapseId};
//                 tMin = t;
//             }
            
//             // create max cap for searching for min
//             if (i == MAX_SEARCH) break;
//         }


//         // add the edges from node to remove to node to collapse
//         for (Edge edge : g.vertices[min[0]].edges) {
//             if (edge[1] != min[1]) {
//                 g.addEdge({edge[1], min[1]});
//             }
//         }

//         // update collapsed of node_to_collapse
//         g.vertices[min[1]].collapsed.push_back(min[0]);

//         for (int c : g.vertices[min[0]].collapsed) {
//             if (find(g.vertices[min[1]].collapsed.begin(), g.vertices[min[1]].collapsed.end(), c) == g.vertices[min[1]].collapsed.end()) {
//                 g.vertices[min[1]].collapsed.push_back(c);
//             }
//         }

//         // remove node_to_remove
//         g.removeNode(min[0]);
//     }

//     // create vector of size `n_communities`
//     Communities communities(nCommunities);
    
//     for (int i = 0; i < g.vertices.size(); i++) {
//         // update communities with precincts according to `collapsed` vectors
//         (g.vertices.begin() + i).value().collapsed.push_back((g.vertices.begin() + i).key());
//         communities[i].vertices = g1.getInducedSubgraph((g.vertices.begin() + i).value().collapsed).vertices;

//         for (int x : (g.vertices.begin() + i).value().collapsed) {
//             g1.vertices[x].community = i;
//         }
//     }

//     return communities;
// }

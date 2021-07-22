#ifndef _HTE_GRAPH_H
#define _HTE_GRAPH_H

#include <iostream>
#include <vector>
#include <random>
#include <memory>

#include <unordered_map>
#include <eigen3/Eigen/Eigen>


namespace hte {


template<class K, class T>
class GraphBase
{
public:

    // do we want to make this unconstructable?
    GraphBase() {}

    virtual int getNumComponents() {}
    virtual bool isConnected() {}

    bool hasArticulationPoint() {}
    std::vector<K> getArticulationPoints() {}

    virtual void addNode(const K id, const T& node) {vertexMap.insert({id, node});}
    virtual void addNode(const K id, T&& node) {vertexMap.insert({id, std::move(node)});}

    virtual void removeNode(const T& node) {}
    virtual void removeNode(const K id) {}

    virtual void removeEdgesTo(const T& node) {}
    virtual void removeEdgesTo(const K id) {}

    virtual void addEdge(const K from, const K to) {}
    virtual void removeEdge(const K from, const K to) {}

protected:
    template<class Func>
    void dfsRecursor(const K& v, Func&& f, bool doBreak = false) {};
    std::unordered_map<K, T> vertexMap;

private:
};


template<class K, class T>
class AdjacencyListGraph : GraphBase<K, T>
{
public:
    AdjacencyListGraph() {}

private:
    std::unordered_map<K, std::vector<K>> adjacencyList;
};


template<class K, class T, class M>
class AdjacencyMatrixGraph : GraphBase<K, T>
{
    AdjacencyMatrixGraph() {}
    AdjacencyMatrixGraph(const AdjacencyListGraph<K, T>& g);

    Eigen::SparseMatrix<K, Eigen::Dynamic> adjacencyMatrix;

    template<class Func, class... Args>
    AdjacencyMatrixGraph<K, T, M> getSubgraphIf(Func&& functor);
    // AdjacencyMatrixGraph<K, T, M> getSubgraphIf(std::function<bool(Args)...> functor);
};


}
#endif
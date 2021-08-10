#ifndef _HTE_GRAPH_H
#define _HTE_GRAPH_H

#include <iostream>
#include <vector>
#include <random>
#include <memory>

#include <unordered_map>
#include <eigen3/Eigen/Eigen>


namespace hte {


template<typename K, typename T>
class GraphBase
{
public:
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

private:
    std::unordered_map<K, T> vertexMap;
};


template<typename K, typename T>
class AdjacencyListGraph : public GraphBase<K, T>
{
public:
    AdjacencyListGraph() {}

private:
    std::unordered_map<K, std::vector<K>> adjacencyList;

    template<typename Ret>
    void dfsRecursor(const K v, const std::unordered_map<K, bool>& visited, const std::function<Ret(const K)>& f, bool doBreak = false);

    template<typename Func, typename... Args>
    AdjacencyListGraph<K, T> getSubgraphIf(Func&& functor);
};


template<typename K, typename T, typename M>
class AdjacencyMatrixGraph : public GraphBase<K, T>
{
    AdjacencyMatrixGraph() {}
    AdjacencyMatrixGraph(const AdjacencyListGraph<K, T>& g);

    Eigen::SparseMatrix<K, Eigen::Dynamic> adjacencyMatrix;
    // AdjacencyMatrixGraph<K, T, M> getSubgraphIf(std::function<bool(Args)...> functor);
};


}
#endif
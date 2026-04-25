#pragma once

#include "domain/edge.h"
#include "domain/node.h"
#include "domain/types.h"

#include <unordered_map>
#include <vector>
#include <string>

namespace domain {

class Graph {
public:
    void addNode(Node node);
    void addEdge(Edge edge);
    
    // Метод для создания пересадок между узлами
    void addTransfer(NodeId from, NodeId to, double time_minutes, const std::string& name = "Пересадка");

    bool hasNode(NodeId id) const;
    const Node& getNode(NodeId id) const;
    const std::vector<Edge>& getEdgesFrom(NodeId id) const;

    // Геттеры для UI   
    const std::unordered_map<NodeId, Node>& getAllNodes() const;
    const std::unordered_map<NodeId, std::vector<Edge>>& getAllEdges() const;

    std::size_t nodeCount() const;

private:
    std::unordered_map<NodeId, Node> nodes_;
    std::unordered_map<NodeId, std::vector<Edge>> adj_;
};

} // namespace domain
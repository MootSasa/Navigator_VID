#pragma once

#include "domain/edge.h"
#include "domain/node.h"
#include "domain/types.h"
#include "domain/time_variants.h"

#include <unordered_map>
#include <vector>
#include <memory>

namespace domain {

class Graph {
public:
    void addNode(Node node) {
        NodeId id = node.id;
        nodes_[id] = std::move(node);
        adj_.try_emplace(id);
    }

    void addEdge(Edge edge) {
        NodeId from = edge.from;
        adj_[from].push_back(std::move(edge));
    }

    bool hasNode(NodeId id) const {
        return nodes_.count(id) > 0;
    }

    const Node& getNode(NodeId id) const {
        return nodes_.at(id);
    }

    const std::vector<Edge>& getEdgesFrom(NodeId id) const {
        return adj_.at(id);
    }

    std::size_t nodeCount() const {
        return nodes_.size();
    }

    const std::unordered_map<NodeId, Node>& getAllNodes() const {
        return nodes_;
    }

    const std::unordered_map<NodeId, std::vector<Edge>>& getAllEdges() const {
        return adj_;
    }

    void addTransfer(NodeId from, NodeId to, double time_minutes, const std::string& name = "Пересадка") {
        Edge edge;
        edge.id = -1; // у пересадок может не быть ID из CSV
        edge.from = from;
        edge.to = to;
        edge.transport = TransportType::Walk; // пересадка - это пешком
        
        edge.length_meters = time_minutes * 60.0 * 1.0; // условная длина
        edge.name = name;
        edge.logic = std::make_unique<StaticLogic>();
    
    addEdge(std::move(edge));
    }

private:
    std::unordered_map<NodeId, Node> nodes_;
    std::unordered_map<NodeId, std::vector<Edge>> adj_;
};

} // namespace domain

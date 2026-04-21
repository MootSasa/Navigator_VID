#pragma once

#include "domain/edge.h"
#include "domain/node.h"
#include "domain/types.h"

#include <unordered_map>
#include <vector>

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

private:
    std::unordered_map<NodeId, Node> nodes_;
    std::unordered_map<NodeId, std::vector<Edge>> adj_;
};

#include "domain/graph.h"
#include "domain/time_variants.h"
#include <memory> 
#include <utility>

namespace domain {

void Graph::addNode(Node node) {
    NodeId id = node.id;
    nodes_[id] = std::move(node);
    adj_.try_emplace(id);
}

void Graph::addEdge(Edge edge) {
    NodeId from = edge.from;
    adj_[from].push_back(std::move(edge));
}

void Graph::addTransfer(NodeId from, NodeId to, double time_minutes, const std::string& name) {
    Edge edge;
    edge.id = -1; // У пересадок нет фиксированного ID из CSV
    edge.from = from;
    edge.to = to;
    edge.transport = TransportType::Walk; 
    
    // Переводим время в условную длину: 1 минута ~ 60 секунд * 1 м/с (усредненная скорость)
    edge.length_meters = time_minutes * 60.0 * 1.0; 
    edge.name = name;
    
    // Для пересадок используем статическую логику (пешком)
    edge.logic = std::make_unique<StaticLogic>();
    
    addEdge(std::move(edge));
}

bool Graph::hasNode(NodeId id) const {
    return nodes_.find(id) != nodes_.end();
}

const Node& Graph::getNode(NodeId id) const {
    return nodes_.at(id);
}

const std::vector<Edge>& Graph::getEdgesFrom(NodeId id) const {
    return adj_.at(id);
}

const std::unordered_map<NodeId, Node>& Graph::getAllNodes() const {
    return nodes_;
}

const std::unordered_map<NodeId, std::vector<Edge>>& Graph::getAllEdges() const {
    return adj_;
}

std::size_t Graph::nodeCount() const {
    return nodes_.size();
}

} // namespace domain
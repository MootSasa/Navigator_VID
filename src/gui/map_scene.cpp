#include "map_scene.h"
#include "node_item.h"
#include "edge_item.h"
#include "route_item.h"

#include <QGraphicsSceneMouseEvent>

MapScene::MapScene(QObject* parent)
    : QGraphicsScene(parent)
{
    constexpr qreal inf = 1e7;
    setSceneRect(-inf, -inf, 2 * inf, 2 * inf);

    routeItem_ = new RouteItem();
    addItem(routeItem_);
}

void MapScene::loadGraph(const domain::Graph& graph)
{
    // Очищаем старые элементы
    clear();
    nodeItems_.clear();

    routeItem_ = new RouteItem();
    addItem(routeItem_);

    const auto& allEdges = graph.getAllEdges();
    for (auto it = allEdges.begin(); it != allEdges.end(); ++it) {
        const auto& edges = it->second;
        for (const auto& edge : edges) {
            if (!graph.hasNode(edge.from) || !graph.hasNode(edge.to)) continue;

            const auto& fromNode = graph.getNode(edge.from);
            const auto& toNode = graph.getNode(edge.to);

            QPointF fromPos(fromNode.coords.x, -fromNode.coords.y);
            QPointF toPos(toNode.coords.x, -toNode.coords.y);

            auto* item = new EdgeItem(edge.id, edge.transport,
                                      edge.length_meters, edge.name,
                                      fromPos, toPos);
            addItem(item);
        }
    }

    const auto& allNodes = graph.getAllNodes();
    for (auto it = allNodes.begin(); it != allNodes.end(); ++it) {
        const auto& node = it->second;
        if (node.type == domain::NodeType::Intersection) continue;
        auto* item = new NodeItem(node);
        addItem(item);
        nodeItems_.insert(node.id, item);

        connect(item, &NodeItem::nodeClicked,
                this, &MapScene::onNodeClicked);
    }
}

void MapScene::highlightRoute(const application::RouteResult& result,
                              const domain::Graph& graph)
{
    routeItem_->setRoute(result, graph);
}

void MapScene::clearRoute()
{
    routeItem_->clearRoute();
}

void MapScene::setNodeHighlight(domain::NodeId id, NodeItem::HighlightRole role)
{
    auto* item = nodeItems_.value(id, nullptr);
    if (item) {
        item->setHighlightRole(role);
    }

    if (role == NodeItem::HighlightRole::Start) {
        startNodeId_ = id;
    } else if (role == NodeItem::HighlightRole::Goal) {
        goalNodeId_ = id;
    }
}

void MapScene::clearNodeHighlights()
{
    for (auto it = nodeItems_.begin(); it != nodeItems_.end(); ++it) {
        it.value()->setHighlightRole(NodeItem::HighlightRole::None);
    }
    startNodeId_ = -1;
    goalNodeId_ = -1;
}

void MapScene::onNodeClicked(domain::NodeId id)
{
    emit nodeSelected(id);
}

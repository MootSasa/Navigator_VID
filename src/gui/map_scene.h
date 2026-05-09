#pragma once

#include "domain/graph.h"
#include "application/a_star_solver.h"
#include "node_item.h"

#include <QGraphicsScene>
#include <QHash>

class EdgeItem;
class RouteItem;

/**
 * @brief Сцена карты - управляет графическими элементами графа.
 *
 * Создаёт NodeItem и EdgeItem из данных графа,
 * управляет подсветкой маршрута через RouteItem.
 */
class MapScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit MapScene(QObject* parent = nullptr);

    /// Загружает граф в сцену, создавая графические элементы.
    void loadGraph(const domain::Graph& graph);

    /// Подсвечивает маршрут на карте.
    void highlightRoute(const application::RouteResult& result,
                        const domain::Graph& graph);

    /// Очищает подсветку маршрута.
    void clearRoute();

    /// Устанавливает выделение узла (старт/финиш).
    void setNodeHighlight(domain::NodeId id, NodeItem::HighlightRole role);

    /// Очищает все выделения узлов.
    void clearNodeHighlights();

signals:
    /// Сигнал выбора узла пользователем.
    void nodeSelected(domain::NodeId id);

private slots:
    /// Обработка клика по узлу.
    void onNodeClicked(domain::NodeId id);

private:
    QHash<domain::NodeId, NodeItem*> nodeItems_;
    RouteItem* routeItem_ = nullptr;

    domain::NodeId startNodeId_ = -1;
    domain::NodeId goalNodeId_ = -1;
};

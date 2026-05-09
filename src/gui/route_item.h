#pragma once

#include "application/a_star_solver.h"
#include "domain/graph.h"

#include <QGraphicsObject>
#include <QColor>
#include <QVector>
#include <QLineF>

/**
 * @brief Графический элемент маршрута поверх графа.
 *
 * Рисует анимированную линию вдоль рёбер маршрута
 * с маркерами старта и финиша.
 */
class RouteItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(double dashOffset READ dashOffset WRITE setDashOffset)

public:
    explicit RouteItem(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    /// Устанавливает маршрут для отображения.
    void setRoute(const application::RouteResult& result,
                  const domain::Graph& graph);

    /// Очищает маршрут.
    void clearRoute();

    /// Возвращает текущий offset для анимации.
    double dashOffset() const { return dashOffset_; }

    /// Устанавливает offset для анимации.
    void setDashOffset(double offset);

private:
    /// Возвращает цвет маршрута по стратегии.
    QColor routeColor() const;

    QVector<QLineF> segments_;
    QPointF startPos_;
    QPointF goalPos_;
    std::string strategyName_;
    double dashOffset_ = 0.0;

    static constexpr double kRouteWidth = 5.0;
    static constexpr double kMarkerRadius = 10.0;
    static constexpr double kPadding = 16.0;
};

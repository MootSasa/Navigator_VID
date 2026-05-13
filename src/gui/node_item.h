#pragma once

#include "domain/node.h"
#include "domain/types.h"

#include <QGraphicsObject>
#include <QColor>

/**
 * @brief Графический элемент узла на карте.
 *
 * Отображается по-разному в зависимости от NodeType.
 * Поддерживает hover-эффект и выделение при выборе как старт/финиш.
 */
class NodeItem : public QGraphicsObject {
    Q_OBJECT

public:
    /// Роль выделения узла
    enum class HighlightRole {
        None,      ///< Не выделен
        Start,     ///< Начальная точка маршрута
        Goal       ///< Конечная точка маршрута
    };

    explicit NodeItem(const domain::Node& node, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    /// Возвращает ID узла.
    domain::NodeId nodeId() const { return node_.id; }

    /// Возвращает тип узла.
    domain::NodeType nodeType() const { return node_.type; }

    /// Устанавливает роль выделения (старт/финиш/нет).
    void setHighlightRole(HighlightRole role);

    /// Возвращает текущую роль выделения.
    HighlightRole highlightRole() const { return highlightRole_; }

signals:
    /// Сигнал клика по узлу.
    void nodeClicked(domain::NodeId id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    /// Возвращает радиус отображения в зависимости от типа.
    double radius() const;

    /// Возвращает цвет заливки в зависимости от типа.
    QColor fillColor() const;

    /// Возвращает цвет обводки при выделении.
    QColor highlightColor() const;

    domain::Node node_;
    HighlightRole highlightRole_ = HighlightRole::None;
    bool hovered_ = false;

    static constexpr double kIntersectionRadius = 4.0;
    static constexpr double kBusStopRadius = 6.0;
    static constexpr double kMetroRadius = 8.0;
    static constexpr double kPoiRadius = 6.0;
    static constexpr double kHoverScale = 1.5;
    static constexpr double kPadding = 4.0;
};

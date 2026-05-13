#pragma once

#include "domain/types.h"

#include <QGraphicsObject>
#include <QColor>
#include <QLineF>
#include <string>

/**
 * @brief Графический элемент ребра на карте.
 *
 * Отображается линией с цветом и стилем, зависящими от TransportType.
 * Поддерживает hover-эффект с tooltip.
 * Не хранит domain::Edge целиком, а только нужные для отрисовки поля.
 */
class EdgeItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit EdgeItem(domain::EdgeId id,
                      domain::TransportType transport,
                      double length_meters,
                      const std::string& name,
                      const QPointF& from_pos,
                      const QPointF& to_pos,
                      QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    /// Возвращает ID ребра.
    domain::EdgeId edgeId() const { return edgeId_; }

    /// Возвращает тип транспорта ребра.
    domain::TransportType transportType() const { return transport_; }

    /// Возвращает линию ребра в координатах сцены.
    QLineF line() const { return line_; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    /// Возвращает цвет линии по типу транспорта.
    QColor lineColor() const;

    /// Возвращает толщину линии по типу транспорта.
    double lineWidth() const;

    /// Возвращает стиль пера (сплошная/пунктир).
    Qt::PenStyle penStyle() const;

    domain::EdgeId edgeId_;
    domain::TransportType transport_;
    double lengthMeters_;
    std::string name_;
    QLineF line_;
    bool hovered_ = false;

    static constexpr double kWalkWidth = 1.5;
    static constexpr double kCarWidth = 2.0;
    static constexpr double kTaxiWidth = 2.0;
    static constexpr double kBusWidth = 2.5;
    static constexpr double kMetroWidth = 3.0;
    static constexpr double kHoverExtra = 1.5;
    static constexpr double kPadding = 6.0;
};

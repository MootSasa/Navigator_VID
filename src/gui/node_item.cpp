#include "node_item.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

NodeItem::NodeItem(const domain::Node& node, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , node_(node)
{
    setPos(node_.coords.x, -node_.coords.y);  // Y-инверсия
    setAcceptHoverEvents(true);
    setZValue(10);  // Узлы поверх рёбер
    setToolTip(QString::fromStdString(node_.name.empty() ?
        ("Узел " + std::to_string(node_.id)) : node_.name));
}

QRectF NodeItem::boundingRect() const
{
    const double r = radius() * (hovered_ ? kHoverScale : 1.0);
    const double pad = r + kPadding;
    return QRectF(-pad, -pad, 2 * pad, 2 * pad);
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                     QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing);

    const double r = radius() * (hovered_ ? kHoverScale : 1.0);

    // Обводка при выделении
    if (highlightRole_ != HighlightRole::None) {
        QColor hc = highlightColor();
        painter->setPen(QPen(hc, 3.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), r + 4, r + 4);
    }

    // Основная фигура
    QColor fill = fillColor();
    painter->setPen(QPen(fill.darker(130), 1.5));
    painter->setBrush(fill);

    if (node_.type == domain::NodeType::Poi) {
        // Ромб для POI
        QPolygonF diamond;
        diamond << QPointF(0, -r)
                << QPointF(r, 0)
                << QPointF(0, r)
                << QPointF(-r, 0);
        painter->drawPolygon(diamond);
    } else {
        // Круг для остальных типов
        painter->drawEllipse(QPointF(0, 0), r, r);
    }

    // Подпись при hover
    if (hovered_) {
        QString name = QString::fromStdString(node_.name);
        if (name.isEmpty()) {
            name = QString("Узел %1").arg(node_.id);
        }
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(QPen(Qt::black));

        QRectF textRect = painter->fontMetrics().boundingRect(name);
        double tx = -textRect.width() / 2.0;
        double ty = -(r + 12 + textRect.height());

        // Фон подписи
        QRectF bgRect(tx - 4, ty - 2, textRect.width() + 8, textRect.height() + 4);
        painter->setBrush(QColor(255, 255, 255, 200));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(bgRect, 3, 3);

        // Текст
        painter->setPen(QPen(Qt::black));
        painter->drawText(QPointF(tx, ty + textRect.height() - 2), name);
    }
}

void NodeItem::setHighlightRole(HighlightRole role)
{
    if (highlightRole_ != role) {
        highlightRole_ = role;
        update();
    }
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    emit nodeClicked(node_.id);
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    hovered_ = true;
    setCursor(Qt::PointingHandCursor);
    update();
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    hovered_ = false;
    setCursor(Qt::ArrowCursor);
    update();
}

double NodeItem::radius() const
{
    switch (node_.type) {
        case domain::NodeType::Intersection:  return kIntersectionRadius;
        case domain::NodeType::BusStop:       return kBusStopRadius;
        case domain::NodeType::MetroStation:  return kMetroRadius;
        case domain::NodeType::Poi:           return kPoiRadius;
    }
    return kIntersectionRadius;
}

QColor NodeItem::fillColor() const
{
    switch (node_.type) {
        case domain::NodeType::Intersection:  return QColor("#9E9E9E");
        case domain::NodeType::BusStop:       return QColor("#2196F3");
        case domain::NodeType::MetroStation:  return QColor("#E53935");
        case domain::NodeType::Poi:           return QColor("#FB8C00");
    }
    return QColor("#9E9E9E");
}

QColor NodeItem::highlightColor() const
{
    switch (highlightRole_) {
        case HighlightRole::Start: return QColor("#4CAF50");
        case HighlightRole::Goal:  return QColor("#F44336");
        case HighlightRole::None:  return QColor();
    }
    return QColor();
}

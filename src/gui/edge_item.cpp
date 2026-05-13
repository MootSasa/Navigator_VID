#include "edge_item.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QCursor>

EdgeItem::EdgeItem(domain::EdgeId id,
                   domain::TransportType transport,
                   double length_meters,
                   const std::string& name,
                   const QPointF& from_pos,
                   const QPointF& to_pos,
                   QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , edgeId_(id)
    , transport_(transport)
    , lengthMeters_(length_meters)
    , name_(name)
    , line_(from_pos, to_pos)
{
    setAcceptHoverEvents(true);
    setZValue(5);  // Рёбра под узлами

    QString tip = QString::fromStdString(name_) +
                  QString(" (%1 м)").arg(lengthMeters_, 0, 'f', 0);
    setToolTip(tip);
}

QRectF EdgeItem::boundingRect() const
{
    double w = lineWidth() + (hovered_ ? kHoverExtra : 0.0) + kPadding;
    return QRectF(
        qMin(line_.p1().x(), line_.p2().x()) - w,
        qMin(line_.p1().y(), line_.p2().y()) - w,
        qAbs(line_.p2().x() - line_.p1().x()) + 2 * w,
        qAbs(line_.p2().y() - line_.p1().y()) + 2 * w
    );
}

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                     QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QColor color = lineColor();
    double width = lineWidth();
    Qt::PenStyle style = penStyle();

    if (hovered_) {
        width += kHoverExtra;
    }

    QPen pen(color, width, style);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    painter->drawLine(line_);
}

void EdgeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    hovered_ = true;
    setCursor(Qt::PointingHandCursor);
    update();
}

void EdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    hovered_ = false;
    setCursor(Qt::ArrowCursor);
    update();
}

QColor EdgeItem::lineColor() const
{
    switch (transport_) {
        case domain::TransportType::Walk:  return QColor("#66BB6A");
        case domain::TransportType::Car:   return QColor("#757575");
        case domain::TransportType::Taxi:  return QColor("#FDD835");
        case domain::TransportType::Bus:   return QColor("#1E88E5");
        case domain::TransportType::Metro: return QColor("#E53935");
    }
    return QColor("#757575");
}

double EdgeItem::lineWidth() const
{
    switch (transport_) {
        case domain::TransportType::Walk:  return kWalkWidth;
        case domain::TransportType::Car:   return kCarWidth;
        case domain::TransportType::Taxi:  return kTaxiWidth;
        case domain::TransportType::Bus:   return kBusWidth;
        case domain::TransportType::Metro: return kMetroWidth;
    }
    return kCarWidth;
}

Qt::PenStyle EdgeItem::penStyle() const
{
    if (transport_ == domain::TransportType::Walk) {
        return Qt::DashLine;
    }
    return Qt::SolidLine;
}

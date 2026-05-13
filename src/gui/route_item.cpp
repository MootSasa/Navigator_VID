#include "route_item.h"

#include <QPainter>
#include <QPropertyAnimation>

RouteItem::RouteItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setZValue(20);  // Маршрут поверх всего

    // Пульсирующая анимация пунктира
    auto* anim = new QPropertyAnimation(this, "dashOffset");
    anim->setDuration(1000);
    anim->setStartValue(0.0);
    anim->setKeyValueAt(0.5, 10.0);
    anim->setEndValue(0.0);
    anim->setLoopCount(-1);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

QRectF RouteItem::boundingRect() const
{
    if (segments_.isEmpty()) {
        return QRectF();
    }

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::min();
    double maxY = std::numeric_limits<double>::min();

    for (const auto& seg : segments_) {
        minX = qMin(minX, qMin(seg.p1().x(), seg.p2().x()));
        minY = qMin(minY, qMin(seg.p1().y(), seg.p2().y()));
        maxX = qMax(maxX, qMax(seg.p1().x(), seg.p2().x()));
        maxY = qMax(maxY, qMax(seg.p1().y(), seg.p2().y()));
    }

    minX = qMin(minX, qMin(startPos_.x(), goalPos_.x()));
    minY = qMin(minY, qMin(startPos_.y(), goalPos_.y()));
    maxX = qMax(maxX, qMax(startPos_.x(), goalPos_.x()));
    maxY = qMax(maxY, qMax(startPos_.y(), goalPos_.y()));

    return QRectF(minX - kPadding, minY - kPadding,
                  maxX - minX + 2 * kPadding,
                  maxY - minY + 2 * kPadding);
}

void RouteItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                     QWidget* /*widget*/)
{
    if (segments_.isEmpty()) return;

    painter->setRenderHint(QPainter::Antialiasing);

    QColor color = routeColor();

    // Фоновая полупрозрачная линия (свечение)
    QPen glowPen(QColor(color.red(), color.green(), color.blue(), 60),
                 kRouteWidth + 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(glowPen);
    for (const auto& seg : segments_) {
        painter->drawLine(seg);
    }

    // Основная линия маршрута
    QPen routePen(color, kRouteWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    routePen.setDashPattern({6, 4});
    routePen.setDashOffset(dashOffset_);
    painter->setPen(routePen);
    for (const auto& seg : segments_) {
        painter->drawLine(seg);
    }

    // Маркер старта
    painter->setPen(QPen(Qt::white, 2));
    painter->setBrush(QColor("#4CAF50"));
    painter->drawEllipse(startPos_, kMarkerRadius, kMarkerRadius);
    painter->setPen(QPen(Qt::white, 2));
    painter->setFont(QFont("Segoe UI", 8, QFont::Bold));
    QRectF startRect(startPos_.x() - kMarkerRadius, startPos_.y() - kMarkerRadius,
                     2 * kMarkerRadius, 2 * kMarkerRadius);
    painter->drawText(startRect, Qt::AlignCenter, "A");

    // Маркер финиша
    painter->setPen(QPen(Qt::white, 2));
    painter->setBrush(QColor("#F44336"));
    painter->drawEllipse(goalPos_, kMarkerRadius, kMarkerRadius);
    painter->setPen(QPen(Qt::white, 2));
    QRectF goalRect(goalPos_.x() - kMarkerRadius, goalPos_.y() - kMarkerRadius,
                    2 * kMarkerRadius, 2 * kMarkerRadius);
    painter->drawText(goalRect, Qt::AlignCenter, "B");
}

void RouteItem::setRoute(const application::RouteResult& result,
                         const domain::Graph& graph)
{
    segments_.clear();
    strategyName_ = result.strategy_name;

    if (result.nodes.empty()) return;

    // Строим сегменты по узлам маршрута
    for (size_t i = 0; i + 1 < result.nodes.size(); ++i) {
        const auto& from = graph.getNode(result.nodes[i]);
        const auto& to = graph.getNode(result.nodes[i + 1]);
        QPointF p1(from.coords.x, -from.coords.y);
        QPointF p2(to.coords.x, -to.coords.y);
        segments_.append(QLineF(p1, p2));
    }

    // Позиции маркеров
    const auto& startNode = graph.getNode(result.nodes.front());
    const auto& goalNode = graph.getNode(result.nodes.back());
    startPos_ = QPointF(startNode.coords.x, -startNode.coords.y);
    goalPos_ = QPointF(goalNode.coords.x, -goalNode.coords.y);

    update();
}

void RouteItem::clearRoute()
{
    segments_.clear();
    startPos_ = QPointF();
    goalPos_ = QPointF();
    strategyName_.clear();
    update();
}

void RouteItem::setDashOffset(double offset)
{
    dashOffset_ = offset;
    update();
}

QColor RouteItem::routeColor() const
{
    if (strategyName_ == "fastest")    return QColor("#42A5F5");
    if (strategyName_ == "cheapest")   return QColor("#66BB6A");
    if (strategyName_ == "convenient") return QColor("#AB47BC");
    return QColor("#5C6BC0");
}

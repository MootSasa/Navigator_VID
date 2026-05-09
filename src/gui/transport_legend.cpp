#include "transport_legend.h"

#include <QPainter>
#include <QPaintEvent>

TransportLegend::TransportLegend(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(160, 222);
}

QSize TransportLegend::sizeHint() const
{
    return QSize(160, 222);
}

void TransportLegend::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    painter.setBrush(QColor(30, 30, 46, 200));
    painter.setPen(QPen(QColor(49, 50, 68), 1));
    painter.drawRoundedRect(rect(), 8, 8);

    painter.setPen(QColor(205, 214, 244));
    QFont titleFont("Segoe UI", 10, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(10, 20, "Узлы");

    // Узлы
    struct NodeEntry { QString label; QColor color; double r; bool diamond; };
    QVector<NodeEntry> nodes = {
        {"Перекрёсток", QColor("#9E9E9E"), 4.0, false},
        {"Остановка",   QColor("#2196F3"), 5.0, false},
        {"Метро",       QColor("#E53935"), 6.0, false},
        {"Здание",       QColor("#FB8C00"), 5.0, true},
    };

    double y = 36;
    for (const auto& n : nodes) {
        painter.setPen(QPen(n.color.darker(130), 1.5));
        painter.setBrush(n.color);
        if (n.diamond) {
            QPolygonF diamond;
            diamond << QPointF(14, y - n.r)
                    << QPointF(14 + n.r, y)
                    << QPointF(14, y + n.r)
                    << QPointF(14 - n.r, y);
            painter.drawPolygon(diamond);
        } else {
            painter.drawEllipse(QPointF(14, y), n.r, n.r);
        }
        painter.setPen(QColor(205, 214, 244));
        QFont labelFont("Segoe UI", 9);
        painter.setFont(labelFont);
        painter.drawText(28, y + 4, n.label);
        y += 20;
    }

    // Разделитель
    y += 4;
    painter.setPen(QPen(QColor(69, 71, 90), 1));
    painter.drawLine(8, y, width() - 8, y);
    y += 14;

    // Рёбра
    painter.setPen(QColor(205, 214, 244));
    painter.setFont(titleFont);
    painter.drawText(10, y, "Маршруты");
    y += 16;

    struct EdgeEntry { QString label; QColor color; Qt::PenStyle style; double w; };
    QVector<EdgeEntry> edges = {
        {"Пешком",      QColor("#66BB6A"), Qt::DashLine, 1.5},
        {"Авто / Такси", QColor("#757575"), Qt::SolidLine, 2.0},
        {"Автобус",     QColor("#1E88E5"), Qt::SolidLine, 2.5},
        {"Метро",       QColor("#E53935"), Qt::SolidLine, 3.0},
    };

    for (const auto& e : edges) {
        QPen pen(e.color, e.w, e.style, Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(6, y, 22, y);

        painter.setPen(QColor(205, 214, 244));
        painter.setFont(QFont("Segoe UI", 9));
        painter.drawText(28, y + 4, e.label);
        y += 18;
    }
}

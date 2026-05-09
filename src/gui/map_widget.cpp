#include "map_widget.h"

#include <QWheelEvent>
#include <QResizeEvent>
#include <QScrollBar>

MapWidget::MapWidget(MapScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , scene_(scene)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::NoDrag);
    setBackgroundBrush(QColor("#FAFAFA"));

    // Легенда в правом нижнем углу
    legend_ = new TransportLegend(this);
    legend_->move(10, 10);
    legend_->raise();
}

void MapWidget::wheelEvent(QWheelEvent* event)
{
    const double factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
    event->accept();
}

void MapWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // Начинаем отслеживание: может быть клик или перетаскивание
        leftState_ = LeftButtonState::PossibleClick;
        pressPos_ = event->pos();
        lastPanPos_ = event->pos();
        event->accept();
        return;
    }

    if (event->button() == Qt::MiddleButton) {
        middlePanning_ = true;
        middlePanStart_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
    // Обработка левой кнопки
    if (leftState_ == LeftButtonState::PossibleClick) {
        QPoint delta = event->pos() - pressPos_;
        if (delta.manhattanLength() > kDragThreshold) {
            leftState_ = LeftButtonState::Panning;
            setCursor(Qt::ClosedHandCursor);
            horizontalScrollBar()->setValue(
                horizontalScrollBar()->value() - delta.x());
            verticalScrollBar()->setValue(
                verticalScrollBar()->value() - delta.y());
            lastPanPos_ = event->pos();
        }
        event->accept();
        return;
    }

    if (leftState_ == LeftButtonState::Panning) {
        QPoint delta = event->pos() - lastPanPos_;
        lastPanPos_ = event->pos();
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }

    // Обработка средней кнопки
    if (middlePanning_) {
        QPoint delta = event->pos() - middlePanStart_;
        middlePanStart_ = event->pos();
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (leftState_ == LeftButtonState::PossibleClick) {
            leftState_ = LeftButtonState::None;
            QGraphicsView::mousePressEvent(
                new QMouseEvent(QEvent::MouseButtonPress,
                                event->position(), event->globalPosition(),
                                Qt::LeftButton, Qt::NoButton,
                                event->modifiers()));
            QGraphicsView::mouseReleaseEvent(event);
            return;
        }
        if (leftState_ == LeftButtonState::Panning) {
            leftState_ = LeftButtonState::None;
            setCursor(Qt::ArrowCursor);
            event->accept();
            return;
        }
    }

    if (event->button() == Qt::MiddleButton && middlePanning_) {
        middlePanning_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void MapWidget::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    // Позиционируем легенду
    legend_->move(width() - legend_->width() - 10,
                  height() - legend_->height() - 10);
    legend_->raise();

    if (firstShow_) {
        fitToView();
        firstShow_ = false;
    }
}

void MapWidget::fitToView()
{
    QRectF itemsRect = scene_->itemsBoundingRect();
    if (itemsRect.isValid()) {
        itemsRect.adjust(-50, -50, 50, 50);
        fitInView(itemsRect, Qt::KeepAspectRatio);
    }
}

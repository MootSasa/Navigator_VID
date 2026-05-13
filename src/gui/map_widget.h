#pragma once

#include "map_scene.h"
#include "transport_legend.h"

#include <QGraphicsView>

/**
 * @brief Виджет карты.
 *
 * Оборачивает QGraphicsView, добавляя:
 * - Зум колесом мыши с центром на курсоре
 * - Перетаскивание левой кнопкой мыши
 *   и клик для выбора узлов
 * - Легенду в углу
 * - Первоначальный fit-to-view
 */
class MapWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapWidget(MapScene* scene, QWidget* parent = nullptr);

public slots:
    /// Подгоняет содержимое под размер виджета (центрирование + fit-to-view).
    void fitToView();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:

    MapScene* scene_;
    TransportLegend* legend_;

    /// Состояния обработки левой кнопки мыши
    enum class LeftButtonState {
        None,           ///< Не нажата
        PossibleClick,  ///< Нажата, но ещё не перетаскивание
        Panning         ///< Перетаскивание
    };
    LeftButtonState leftState_ = LeftButtonState::None;
    QPoint pressPos_;          ///< Позиция нажатия
    QPoint lastPanPos_;        ///< Последняя позиция при перетаскивания

    bool middlePanning_ = false;  ///< Перетаскивание средней кнопкой
    QPoint middlePanStart_;

    bool firstShow_ = true;

    static constexpr int kDragThreshold = 5;  ///< Порог в пикселях для различения клик/перетаскивание
};

#pragma once

#include <QWidget>

/**
 * @brief Легенда условных обозначений для карты.
 *
 * Компактный полупрозрачный виджет, отображающий
 * цвета и стили для типов транспорта и узлов.
 */
class TransportLegend : public QWidget {
    Q_OBJECT

public:
    explicit TransportLegend(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
};

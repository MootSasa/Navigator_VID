#pragma once

#include "types.h"

#include <string>

namespace domain {

/**
 * @brief Двумерная точка (координаты узла в метрах от условного центра).
 */
struct Point {
    double x = 0.0;
    double y = 0.0;
};

/**
 * @brief Узел графа (перекрёсток, остановка, станция метро, точка интереса).
 */
struct Node {
    NodeId id = 0;                          ///< Уникальный идентификатор
    NodeType type = NodeType::Intersection;  ///< Тип узла
    std::string name;                        ///< Человекочитаемое название
    Point coords;                           ///< Координаты в метрах
};

} // namespace domain

#pragma once

namespace domain {

using NodeId = int;  ///< Уникальный идентификатор узла
using EdgeId = int;  ///< Уникальный идентификатор ребра

/**
 * @brief Тип транспорта.
 */
enum class TransportType {
    Walk,   ///< Пешком
    Car,    ///< Личный автомобиль
    Taxi,   ///< Такси
    Bus,    ///< Автобус
    Metro   ///< Метро
};

/**
 * @brief Тип узла графа.
 */
enum class NodeType {
    Intersection,    ///< Перекрёсток
    BusStop,         ///< Автобусная остановка
    MetroStation,    ///< Станция метро
    Poi              ///< Точка интереса (POI)
};

} // namespace domain

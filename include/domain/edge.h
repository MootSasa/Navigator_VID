#pragma once

#include "i_time_variant.h"
#include "types.h"

#include <memory>
#include <string>

namespace domain {

/**
 * @brief Ребро графа (путь между двумя узлами).
 *
 * Содержит информацию о типе транспорта, длине, названии
 * и логике расчёта стоимости/времени (через ITimeVariant).
 */
struct Edge {
    EdgeId id = 0;                                    ///< Уникальный ID ребра
    NodeId from = 0;                                  ///< ID начального узла
    NodeId to = 0;                                    ///< ID конечного узла
    TransportType transport = TransportType::Walk;     ///< Тип транспорта
    double length_meters = 0.0;                       ///< Длина ребра в метрах
    std::string name;                                 ///< Название (улица, номер маршрута)
    std::unique_ptr<ITimeVariant> logic;              ///< Логика расчёта (статическая, расписание и т.д.)
};

} // namespace domain

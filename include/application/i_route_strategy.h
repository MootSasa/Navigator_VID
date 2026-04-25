#pragma once

#include "../domain/combined_cost.h"
#include "../domain/node.h"
#include "../domain/transport_params.h"

namespace domain {
struct Edge;
} // namespace domain

namespace application {

/**
 * @brief Интерфейс стратегии поиска маршрута.
 *
 * Определяет, как вычисляется стоимость ребра,
 * какая эвристика используется в A* и как сравниваются стоимости.
 */
class IRouteStrategy {
public:
    virtual ~IRouteStrategy() = default;

    /**
     * @brief Создает объект стоимости для конкретного ребра и контекста.
     * @param edge Ребро графа
     * @param params Параметры транспорта
     * @param travel_seconds Время движения в секундах
     * @param wait_seconds Время ожидания в секундах
     * @return Объект CombinedCost
     */
    virtual domain::CombinedCost create_cost(
        const domain::Edge& edge,
        const domain::TransportParams& params,
        double travel_seconds,
        double wait_seconds
    ) const = 0;

    /**
     * @brief Эвристическая функция для A*.
     * @param from Начальный узел
     * @param to Целевой узел
     * @param params Параметры транспорта
     * @return Оценка стоимости от from до to
     */
    virtual domain::CombinedCost heuristic(
        const domain::Node& from,
        const domain::Node& to,
        const domain::TransportParams& params
    ) const = 0;

    /**
     * @brief Сравнивает две стоимости (a < b по критерию стратегии).
     * @param a Первая стоимость
     * @param b Вторая стоимость
     * @return true если a лучше (меньше) чем b
     */
    virtual bool less(const domain::CombinedCost& a, const domain::CombinedCost& b) const = 0;
};

} // namespace application

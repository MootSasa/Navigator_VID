#pragma once

#include "../domain/combined_cost.h"
#include "../domain/graph.h"
#include "../domain/transport_params.h"
#include "../domain/types.h"

#include <chrono>
#include <optional>
#include <vector>

namespace application {

class IRouteStrategy;

/**
 * @brief Результат поиска маршрута.
 */
struct RouteResult {
    std::vector<domain::NodeId> nodes;   ///< Последовательность узлов маршрута
    std::vector<domain::EdgeId> edges;  ///< Последовательность рёбер маршрута
    domain::CombinedCost total_cost;     ///< Итоговая стоимость маршрута
};

/**
 * @brief Ищет маршрут между двумя узлами с помощью алгоритма A*.
 * @param graph Граф для поиска
 * @param start ID начального узла
 * @param goal ID целевого узла
 * @param strategy Стратегия поиска маршрута
 * @param params Параметры транспорта
 * @param departure Время отправления
 * @return Результат маршрута или nullopt если маршрут не найден
 */
std::optional<RouteResult> findRoute(
    const domain::Graph& graph,
    domain::NodeId start,
    domain::NodeId goal,
    const IRouteStrategy& strategy,
    const domain::TransportParams& params,
    std::chrono::system_clock::time_point departure
);

} // namespace application

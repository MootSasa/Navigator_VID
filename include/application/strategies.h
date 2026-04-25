#pragma once

#include "i_route_strategy.h"

namespace application {

/**
 * @brief Стратегия "Самый быстрый" - минимизация времени в пути.
 */
class FastestStrategy : public IRouteStrategy {
public:
    domain::CombinedCost create_cost(
        const domain::Edge& edge,
        const domain::TransportParams& params,
        double travel_seconds,
        double wait_seconds
    ) const override;

    domain::CombinedCost heuristic(
        const domain::Node& from,
        const domain::Node& to,
        const domain::TransportParams& params
    ) const override;

    bool less(const domain::CombinedCost& a, const domain::CombinedCost& b) const override;
};

/**
 * @brief Стратегия "Самый дешёвый" - минимизация денег и усталости.
 *
 * Пешком считается усталость (не рубли), автобус/метро - фиксированный билет,
 * авто - топливо, такси - базовая цена + цена за км.
 */
class CheapestStrategy : public IRouteStrategy {
public:
    domain::CombinedCost create_cost(
        const domain::Edge& edge,
        const domain::TransportParams& params,
        double travel_seconds,
        double wait_seconds
    ) const override;

    domain::CombinedCost heuristic(
        const domain::Node& from,
        const domain::Node& to,
        const domain::TransportParams& params
    ) const override;

    bool less(const domain::CombinedCost& a, const domain::CombinedCost& b) const override;
};

/**
 * @brief Стратегия "Самый удобный" - минимизация пересадок.
 *
 * При равном числе пересадок предпочитает более быстрый маршрут.
 */
class MostConvenientStrategy : public IRouteStrategy {
public:
    domain::CombinedCost create_cost(
        const domain::Edge& edge,
        const domain::TransportParams& params,
        double travel_seconds,
        double wait_seconds
    ) const override;

    domain::CombinedCost heuristic(
        const domain::Node& from,
        const domain::Node& to,
        const domain::TransportParams& params
    ) const override;

    bool less(const domain::CombinedCost& a, const domain::CombinedCost& b) const override;
};

} // namespace application

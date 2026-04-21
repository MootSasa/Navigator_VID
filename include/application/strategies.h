#pragma once

#include "application/i_route_strategy.h"

class FastestStrategy : public IRouteStrategy {
public:
    CombinedCost create_cost(
        const Edge& edge,
        const TransportParams& params,
        double travel_seconds,
        double wait_seconds
    ) const override;

    CombinedCost heuristic(
        const Node& from,
        const Node& to,
        const TransportParams& params
    ) const override;

    bool less(const CombinedCost& a, const CombinedCost& b) const override;
};

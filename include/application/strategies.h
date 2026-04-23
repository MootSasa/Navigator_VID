#pragma once

#include "application/i_route_strategy.h"

namespace application {

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

} // namespace application

#pragma once

#include "domain/combined_cost.h"
#include "domain/node.h"
#include "domain/transport_params.h"

namespace domain {
struct Edge;
} // namespace domain

namespace application {

class IRouteStrategy {
public:
    virtual ~IRouteStrategy() = default;

    virtual domain::CombinedCost create_cost(
        const domain::Edge& edge,
        const domain::TransportParams& params,
        double travel_seconds,
        double wait_seconds
    ) const = 0;

    virtual domain::CombinedCost heuristic(
        const domain::Node& from,
        const domain::Node& to,
        const domain::TransportParams& params
    ) const = 0;

    virtual bool less(const domain::CombinedCost& a, const domain::CombinedCost& b) const = 0;
};

} // namespace application

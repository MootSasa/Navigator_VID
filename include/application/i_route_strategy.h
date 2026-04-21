#pragma once

#include "domain/combined_cost.h"
#include "domain/node.h"
#include "domain/transport_params.h"

struct Edge;

class IRouteStrategy {
public:
    virtual ~IRouteStrategy() = default;

    virtual CombinedCost create_cost(
        const Edge& edge,
        const TransportParams& params,
        double travel_seconds,
        double wait_seconds
    ) const = 0;

    virtual CombinedCost heuristic(
        const Node& from,
        const Node& to,
        const TransportParams& params
    ) const = 0;

    virtual bool less(const CombinedCost& a, const CombinedCost& b) const = 0;
};

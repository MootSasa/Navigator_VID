#pragma once

#include "domain/combined_cost.h"
#include "domain/graph.h"
#include "domain/transport_params.h"
#include "domain/types.h"

#include <chrono>
#include <optional>
#include <vector>

class IRouteStrategy;

struct RouteResult {
    std::vector<NodeId> nodes;
    std::vector<EdgeId> edges;
    CombinedCost total_cost;
};

std::optional<RouteResult> findRoute(
    const Graph& graph,
    NodeId start,
    NodeId goal,
    const IRouteStrategy& strategy,
    const TransportParams& params,
    std::chrono::system_clock::time_point departure
);

#pragma once

#include "domain/combined_cost.h"
#include "domain/graph.h"
#include "domain/transport_params.h"
#include "domain/types.h"

#include <chrono>
#include <optional>
#include <vector>

namespace application {

class IRouteStrategy;

struct RouteResult {
    std::vector<domain::NodeId> nodes;
    std::vector<domain::EdgeId> edges;
    domain::CombinedCost total_cost;
};

std::optional<RouteResult> findRoute(
    const domain::Graph& graph,
    domain::NodeId start,
    domain::NodeId goal,
    const IRouteStrategy& strategy,
    const domain::TransportParams& params,
    std::chrono::system_clock::time_point departure
);

} // namespace application

#include "domain/time_variants.h"

#include "application/i_route_strategy.h"
#include "domain/edge.h"

#include <chrono>

std::optional<EdgeResult> StaticLogic::calculate(
    const Edge& edge,
    const QueryContext& ctx,
    const IRouteStrategy& strategy
) const {
    double speed_mps = ctx.params.speedMps(edge.transport);
    if (speed_mps <= 0.0) {
        return std::nullopt;
    }

    double travel_sec = edge.length_meters / speed_mps;

    auto travel_duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(
        std::chrono::duration<double>(travel_sec)
    );
    auto arrival = ctx.departure_time + travel_duration;

    CombinedCost cost = strategy.create_cost(edge, ctx.params, travel_sec, 0.0);

    return EdgeResult{arrival, cost};
}

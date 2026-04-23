#include "application/strategies.h"

#include "domain/edge.h"

#include <cmath>

namespace application {

domain::CombinedCost FastestStrategy::create_cost(
    const domain::Edge& /*edge*/,
    const domain::TransportParams& /*params*/,
    double travel_seconds,
    double wait_seconds
) const {
    domain::CombinedCost c;
    c.time_seconds = travel_seconds + wait_seconds;
    return c;
}

domain::CombinedCost FastestStrategy::heuristic(
    const domain::Node& from,
    const domain::Node& to,
    const domain::TransportParams& params
) const {
    double dx = from.coords.x - to.coords.x;
    double dy = from.coords.y - to.coords.y;
    double dist = std::sqrt(dx * dx + dy * dy);

    domain::CombinedCost c;
    c.time_seconds = dist / params.maxSpeedMps();
    return c;
}

bool FastestStrategy::less(const domain::CombinedCost& a, const domain::CombinedCost& b) const {
    return a.time_seconds < b.time_seconds;
}

} // namespace application

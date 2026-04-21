#include "application/strategies.h"

#include "domain/edge.h"

#include <cmath>

CombinedCost FastestStrategy::create_cost(
    const Edge& /*edge*/,
    const TransportParams& /*params*/,
    double travel_seconds,
    double wait_seconds
) const {
    CombinedCost c;
    c.time_seconds = travel_seconds + wait_seconds;
    return c;
}

CombinedCost FastestStrategy::heuristic(
    const Node& from,
    const Node& to,
    const TransportParams& params
) const {
    double dx = from.coords.x - to.coords.x;
    double dy = from.coords.y - to.coords.y;
    double dist = std::sqrt(dx * dx + dy * dy);

    CombinedCost c;
    c.time_seconds = dist / params.maxSpeedMps();
    return c;
}

bool FastestStrategy::less(const CombinedCost& a, const CombinedCost& b) const {
    return a.time_seconds < b.time_seconds;
}

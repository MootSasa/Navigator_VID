#include "application/navigation_facade.h"

#include <stdexcept>

namespace application {

NavigationFacade::NavigationFacade(
    std::unique_ptr<domain::Graph> graph,
    domain::TransportParams params
) : graph_(std::move(graph))
  , params_(std::move(params))
{
}

IRouteStrategy& NavigationFacade::getStrategy(const std::string& type) {
    if (type == "fastest") {
        return fastest_;
    } else if (type == "cheapest") {
        return cheapest_;
    } else if (type == "convenient") {
        return convenient_;
    }
    // По умолчанию - самая быстрая
    return fastest_;
}

std::optional<RouteResult> NavigationFacade::findRoute(
    domain::NodeId start,
    domain::NodeId goal,
    const std::string& strategy_type,
    std::chrono::system_clock::time_point departure
) {
    IRouteStrategy& strategy = getStrategy(strategy_type);
    auto result = application::findRoute(*graph_, start, goal, strategy, params_, departure);
    if (result) {
        result->strategy_name = strategy_type;
    }
    return result;
}

std::vector<RouteResult> NavigationFacade::findAllRoutes(
    domain::NodeId start,
    domain::NodeId goal,
    std::chrono::system_clock::time_point departure
) {
    std::vector<RouteResult> results;

    auto r1 = application::findRoute(*graph_, start, goal, fastest_, params_, departure);
    if (r1) { r1->strategy_name = "fastest"; results.push_back(*r1); }

    auto r2 = application::findRoute(*graph_, start, goal, cheapest_, params_, departure);
    if (r2) { r2->strategy_name = "cheapest"; results.push_back(*r2); }

    auto r3 = application::findRoute(*graph_, start, goal, convenient_, params_, departure);
    if (r3) { r3->strategy_name = "convenient"; results.push_back(*r3); }

    return results;
}

const domain::Graph& NavigationFacade::getGraph() const {
    return *graph_;
}

const domain::TransportParams& NavigationFacade::getParams() const {
    return params_;
}

bool NavigationFacade::hasNode(domain::NodeId id) const {
    return graph_->hasNode(id);
}

} // namespace application

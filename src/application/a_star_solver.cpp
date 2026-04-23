#include "application/a_star_solver.h"

#include "application/i_route_strategy.h"
#include "domain/edge.h"
#include "domain/i_time_variant.h"

#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace application {

namespace {

struct LastRoute {
    domain::TransportType transport;
    std::string name;
};

struct OpenNode {
    domain::NodeId id;
    domain::CombinedCost g;
    domain::CombinedCost f;
    std::chrono::system_clock::time_point arrival;
    std::optional<LastRoute> last_route;
};

bool sameRoute(const LastRoute& a, const domain::Edge& e) {
    return a.transport == e.transport && a.name == e.name;
}

} // anonymous namespace

std::optional<RouteResult> findRoute(
    const domain::Graph& graph,
    domain::NodeId start,
    domain::NodeId goal,
    const IRouteStrategy& strategy,
    const domain::TransportParams& params,
    std::chrono::system_clock::time_point departure
) {
    if (!graph.hasNode(start) || !graph.hasNode(goal)) {
        return std::nullopt;
    }

    auto cmp = [&strategy](const OpenNode& a, const OpenNode& b) {
        return strategy.less(b.f, a.f);
    };
    std::priority_queue<OpenNode, std::vector<OpenNode>, decltype(cmp)> open(cmp);

    std::unordered_map<domain::NodeId, domain::CombinedCost> best_g;
    std::unordered_map<domain::NodeId, std::pair<domain::NodeId, domain::EdgeId>> came_from;

    domain::CombinedCost g0;
    domain::CombinedCost h0 = strategy.heuristic(graph.getNode(start), graph.getNode(goal), params);
    open.push(OpenNode{start, g0, g0 + h0, departure, std::nullopt});
    best_g[start] = g0;

    while (!open.empty()) {
        OpenNode cur = open.top();
        open.pop();

        if (cur.id == goal) {
            std::vector<domain::NodeId> path_nodes;
            std::vector<domain::EdgeId> path_edges;
            domain::NodeId n = goal;
            path_nodes.push_back(n);
            while (came_from.count(n) > 0) {
                auto [prev, e] = came_from[n];
                path_edges.push_back(e);
                path_nodes.push_back(prev);
                n = prev;
            }
            std::reverse(path_nodes.begin(), path_nodes.end());
            std::reverse(path_edges.begin(), path_edges.end());
            return RouteResult{path_nodes, path_edges, cur.g};
        }

        auto bg_it = best_g.find(cur.id);
        if (bg_it != best_g.end() && strategy.less(bg_it->second, cur.g)) {
            continue;
        }

        for (const domain::Edge& edge : graph.getEdgesFrom(cur.id)) {
            domain::QueryContext ctx{cur.arrival, params};
            auto res = edge.logic->calculate(edge, ctx, strategy);
            if (!res) continue;

            int transfer_add = 0;
            if (edge.transport != domain::TransportType::Walk) {
                if (cur.last_route && !sameRoute(*cur.last_route, edge)) {
                    transfer_add = 1;
                }
            }

            domain::CombinedCost edge_cost = res->cost;
            edge_cost.transfers += transfer_add;
            domain::CombinedCost new_g = cur.g + edge_cost;

            auto it = best_g.find(edge.to);
            if (it == best_g.end() || strategy.less(new_g, it->second)) {
                best_g[edge.to] = new_g;
                came_from[edge.to] = {cur.id, edge.id};

                std::optional<LastRoute> next_last = cur.last_route;
                if (edge.transport != domain::TransportType::Walk) {
                    next_last = LastRoute{edge.transport, edge.name};
                }

                domain::CombinedCost h = strategy.heuristic(graph.getNode(edge.to), graph.getNode(goal), params);
                open.push(OpenNode{
                    edge.to,
                    new_g,
                    new_g + h,
                    res->arrival_time,
                    next_last
                });
            }
        }
    }

    return std::nullopt;
}

} // namespace application

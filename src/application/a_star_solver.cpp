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

/// Ключ состояния: NodeId, hasPublicTicket, hasLeftCar.
/// hasLeftCar = true означает, что мы уже воспользовались личным авто
/// и затем вышли из него - вернуться на личное авто нельзя, но такси можно.
struct StateKey {
    domain::NodeId node;
    bool has_public_ticket;
    bool has_left_car;

    bool operator==(const StateKey& o) const {
        return node == o.node && has_public_ticket == o.has_public_ticket && has_left_car == o.has_left_car;
    }
};

struct StateKeyHash {
    size_t operator()(const StateKey& k) const {
        return std::hash<int>()(k.node)
             ^ (std::hash<bool>()(k.has_public_ticket) << 1)
             ^ (std::hash<bool>()(k.has_left_car) << 2);
    }
};

struct OpenNode {
    domain::NodeId id;
    domain::CombinedCost g;
    domain::CombinedCost f;
    std::chrono::system_clock::time_point arrival;
    std::optional<LastRoute> last_route;
    bool has_public_ticket = false;
    bool has_left_car = false;
};

/// Информация о переходе для восстановления пути
struct TransitionInfo {
    StateKey prev_key;
    domain::EdgeId edge_id;
    domain::TransportType actual_transport;  ///< Фактический тип транспорта (Car->Taxi если есть билет)
};

bool isPublicTransport(domain::TransportType t) {
    return t == domain::TransportType::Bus || t == domain::TransportType::Metro;
}

/// Определяет фактический тип транспорта для ребра.
/// Если это Car-ребро и куплен билет на общ. транспорт - едем на такси.
domain::TransportType actualTransportType(domain::TransportType edge_transport, bool has_public_ticket) {
    if (edge_transport == domain::TransportType::Car && has_public_ticket) {
        return domain::TransportType::Taxi;
    }
    return edge_transport;
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

    std::unordered_map<StateKey, domain::CombinedCost, StateKeyHash> best_g;
    std::unordered_map<StateKey, TransitionInfo, StateKeyHash> came_from;

    domain::CombinedCost g0;
    domain::CombinedCost h0 = strategy.heuristic(graph.getNode(start), graph.getNode(goal), params);
    StateKey start_key{start, false, false};
    open.push(OpenNode{start, g0, g0 + h0, departure, std::nullopt, false, false});
    best_g[start_key] = g0;

    while (!open.empty()) {
        OpenNode cur = open.top();
        open.pop();

        StateKey cur_key{cur.id, cur.has_public_ticket, cur.has_left_car};

        if (cur.id == goal) {
            // Восстановление пути
            std::vector<domain::NodeId> path_nodes;
            std::vector<domain::EdgeId> path_edges;
            std::vector<domain::TransportType> path_transports;
            StateKey k = cur_key;
            path_nodes.push_back(k.node);
            while (came_from.count(k) > 0) {
                auto& info = came_from[k];
                path_edges.push_back(info.edge_id);
                path_transports.push_back(info.actual_transport);
                path_nodes.push_back(info.prev_key.node);
                k = info.prev_key;
            }
            std::reverse(path_nodes.begin(), path_nodes.end());
            std::reverse(path_edges.begin(), path_edges.end());
            std::reverse(path_transports.begin(), path_transports.end());
            return RouteResult{path_nodes, path_edges, path_transports, cur.g, {}};
        }

        auto bg_it = best_g.find(cur_key);
        if (bg_it != best_g.end() && strategy.less(bg_it->second, cur.g)) {
            continue;
        }

        for (const domain::Edge& edge : graph.getEdgesFrom(cur.id)) {
            // Определяем фактический тип транспорта
            domain::TransportType actual = actualTransportType(edge.transport, cur.has_public_ticket);

            // Личный автомобиль запрещён, если куплен билет на общ. транспорт.
            // Но по Car-ребрам можно проехать на такси (actual == Taxi).
            // Если actual == Car - значит билета нет, едем на личном авто.
            // Если actual == Taxi - значит билет есть, едем на такси по Car-ребру.

            // Нельзя вернуться на личный авто после выхода из него.
            if (actual == domain::TransportType::Car && cur.has_left_car) {
                continue;
            }

            domain::QueryContext ctx{cur.arrival, params};
            auto res = edge.logic->calculate(edge, ctx, strategy);
            if (!res) continue;

            // Пересчитываем стоимость, если фактический транспорт - такси
            domain::CombinedCost edge_cost = res->cost;
            if (actual == domain::TransportType::Taxi && edge.transport == domain::TransportType::Car) {
                // Заменяем стоимость Car на стоимость Taxi
                double distance_km = edge.length_meters / 1000.0;
                edge_cost.money_rub = params.taxi_base_price + distance_km * params.taxi_price_per_km;
            }

            // Подсчёт пересадок:
            // - Car/Taxi: смена дороги не пересадка
            // - Bus/Metro: смена маршрута = пересадка
            // - Смена вида транспорта (Taxi->Bus, Bus->Metro и т.п.) = пересадка
            int transfer_add = 0;
            if (isPublicTransport(actual)) {
                // Общ. транспорт: пересадка при смене маршрута
                if (cur.last_route) {
                    auto& prev = *cur.last_route;
                    if (prev.transport != actual || prev.name != edge.name) {
                        transfer_add = 1;
                    }
                }
            } else if (actual == domain::TransportType::Taxi) {
                // Такси: пересадка только при смене на общ. транспорт
                // (Taxi->Taxi не пересадка)
                if (cur.last_route && isPublicTransport(cur.last_route->transport)) {
                    transfer_add = 1;
                }
            }
            // Car и Walk: пересадок нет

            edge_cost.transfers += transfer_add;

            // Безлимитный билет на общественный транспорт:
            // покупается один раз при первом использовании Bus/Metro,
            // действует на все виды общ. транспорта на всём маршруте.
            bool next_has_ticket = cur.has_public_ticket;
            if (isPublicTransport(edge.transport)) {
                if (cur.has_public_ticket) {
                    // Билет уже куплен - общ. транспорт бесплатный
                    edge_cost.money_rub = 0.0;
                } else {
                    // Первое использование общ. транспорта - покупаем билет
                    next_has_ticket = true;
                }
            }

            domain::CombinedCost new_g = cur.g + edge_cost;

            // Отслеживание выхода из личного авто:
            // has_left_car = true, если мы когда-то ехали на Car и теперь не на Car.
            // Taxi не считается - на такси можно переключаться свободно.
            bool next_has_left_car = cur.has_left_car;
            if (actual == domain::TransportType::Car) {
                // Сейчас едем на личном авто - ещё не вышли
                next_has_left_car = false;
            } else if (cur.last_route && cur.last_route->transport == domain::TransportType::Car) {
                // Были на личном авто и теперь переключились - вышли из авто
                next_has_left_car = true;
            }
            // Если уже has_left_car, оно сохраняется

            StateKey next_key{edge.to, next_has_ticket, next_has_left_car};
            auto it = best_g.find(next_key);
            if (it == best_g.end() || strategy.less(new_g, it->second)) {
                best_g[next_key] = new_g;
                came_from[next_key] = TransitionInfo{cur_key, edge.id, actual};

                std::optional<LastRoute> next_last = cur.last_route;
                if (actual != domain::TransportType::Walk) {
                    next_last = LastRoute{actual, edge.name};
                }

                domain::CombinedCost h = strategy.heuristic(graph.getNode(edge.to), graph.getNode(goal), params);
                open.push(OpenNode{
                    edge.to,
                    new_g,
                    new_g + h,
                    res->arrival_time,
                    next_last,
                    next_has_ticket,
                    next_has_left_car
                });
            }
        }
    }

    return std::nullopt;
}

} // namespace application

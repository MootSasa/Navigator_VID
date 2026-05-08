#include "application/strategies.h"

#include "domain/edge.h"

#include <cmath>

namespace application {

// ==================== FastestStrategy ====================

domain::CombinedCost FastestStrategy::create_cost(
    const domain::Edge& edge,
    const domain::TransportParams& params,
    double travel_seconds,
    double wait_seconds
) const {
    domain::CombinedCost c;
    c.time_seconds = travel_seconds + wait_seconds;

    double distance_km = edge.length_meters / 1000.0;

    switch (edge.transport) {
        case domain::TransportType::Walk:
            c.money_rub = 0.0;
            break;
        case domain::TransportType::Car:
            c.money_rub = distance_km * (params.car_fuel_l_per_100km / 100.0) * params.car_fuel_rub_per_l;
            break;
        case domain::TransportType::Taxi:
            c.money_rub = params.taxi_base_price + distance_km * params.taxi_price_per_km;
            break;
        case domain::TransportType::Bus:
            c.money_rub = params.bus_ticket_rub;
            break;
        case domain::TransportType::Metro:
            c.money_rub = params.metro_ticket_rub;
            break;
    }

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
    if (a.time_seconds != b.time_seconds) return a.time_seconds < b.time_seconds;
    // При равном времени - предпочитаем дешевле
    if (a.money_rub != b.money_rub) return a.money_rub < b.money_rub;
    // При прочих равных - меньше усталости
    return a.fatigue < b.fatigue;
}

// ==================== CheapestStrategy ====================

domain::CombinedCost CheapestStrategy::create_cost(
    const domain::Edge& edge,
    const domain::TransportParams& params,
    double travel_seconds,
    double wait_seconds
) const {
    domain::CombinedCost c;
    c.time_seconds = travel_seconds + wait_seconds;

    double distance_km = edge.length_meters / 1000.0;

    switch (edge.transport) {
        case domain::TransportType::Walk:
            // Пешком - усталость
            c.fatigue = distance_km * params.walk_fatigue_per_km;
            c.money_rub = 0.0;
            break;
        case domain::TransportType::Car:
            // Личный автомобиль - топливо
            c.money_rub = distance_km * (params.car_fuel_l_per_100km / 100.0) * params.car_fuel_rub_per_l;
            c.fatigue = 0.0;
            break;
        case domain::TransportType::Taxi:
            // Такси - базовая цена + цена за км
            c.money_rub = params.taxi_base_price + distance_km * params.taxi_price_per_km;
            c.fatigue = 0.0;
            break;
        case domain::TransportType::Bus:
            // Автобус - фиксированная цена билета
            c.money_rub = params.bus_ticket_rub;
            c.fatigue = 0.0;
            break;
        case domain::TransportType::Metro:
            // Метро - фиксированная цена билета
            c.money_rub = params.metro_ticket_rub;
            c.fatigue = 0.0;
            break;
    }

    return c;
}

domain::CombinedCost CheapestStrategy::heuristic(
    const domain::Node& from,
    const domain::Node& to,
    const domain::TransportParams& params
) const {
    // Допустимая эвристика: пешком = 0 руб + усталость.
    double dx = from.coords.x - to.coords.x;
    double dy = from.coords.y - to.coords.y;
    double dist_km = std::sqrt(dx * dx + dy * dy) / 1000.0;

    domain::CombinedCost c;
    c.money_rub = 0.0;
    c.fatigue = dist_km * params.walk_fatigue_per_km;
    return c;
}

bool CheapestStrategy::less(const domain::CombinedCost& a, const domain::CombinedCost& b) const {
    // Основной критерий - стоимость
    if (a.money_rub != b.money_rub) return a.money_rub < b.money_rub;
    // При равной стоимости - предпочитаем более быстрый
    if (a.time_seconds != b.time_seconds) return a.time_seconds < b.time_seconds;
    // При прочих равных - меньше усталости
    return a.fatigue < b.fatigue;
}

// ==================== MostConvenientStrategy ====================

domain::CombinedCost MostConvenientStrategy::create_cost(
    const domain::Edge& edge,
    const domain::TransportParams& params,
    double travel_seconds,
    double wait_seconds
) const {
    domain::CombinedCost c;
    c.time_seconds = travel_seconds + wait_seconds;

    double distance_km = edge.length_meters / 1000.0;

    switch (edge.transport) {
        case domain::TransportType::Walk:
            c.fatigue = distance_km * params.walk_fatigue_per_km;
            c.money_rub = 0.0;
            break;
        case domain::TransportType::Car:
            c.money_rub = distance_km * (params.car_fuel_l_per_100km / 100.0) * params.car_fuel_rub_per_l;
            c.fatigue = 0.0;
            break;
        case domain::TransportType::Taxi:
            c.money_rub = params.taxi_base_price + distance_km * params.taxi_price_per_km;
            c.fatigue = 0.0;
            break;
        case domain::TransportType::Bus:
            c.money_rub = params.bus_ticket_rub;
            c.fatigue = 0.0;
            break;
        case domain::TransportType::Metro:
            c.money_rub = params.metro_ticket_rub;
            c.fatigue = 0.0;
            break;
    }

    return c;
}

domain::CombinedCost MostConvenientStrategy::heuristic(
    const domain::Node& /*from*/,
    const domain::Node& /*to*/,
    const domain::TransportParams& /*params*/
) const {
    // Допустимая эвристика: 0 пересадок - минимально возможное значение
    domain::CombinedCost c;
    c.transfers = 0;
    return c;
}

bool MostConvenientStrategy::less(const domain::CombinedCost& a, const domain::CombinedCost& b) const {
    // Основной критерий - минимум пересадок
    if (a.transfers != b.transfers) return a.transfers < b.transfers;
    // При равном числе пересадок - предпочитаем более быстрый
    return a.time_seconds < b.time_seconds;
}

} // namespace application

#include "application/strategies.h"

#include "domain/edge.h"

#include <cmath>

namespace application {

// ==================== FastestStrategy ====================

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
            // Пешком - усталость, а не рубли
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
    // Эвристика: минимальная возможная стоимость на единицу расстояния
    // Самый дешёвый способ - пешком (0 руб), но с усталостью.
    // Для допустимой эвристики считаем, что можно пройти пешком с минимальной усталостью.
    double dx = from.coords.x - to.coords.x;
    double dy = from.coords.y - to.coords.y;
    double dist_km = std::sqrt(dx * dx + dy * dy) / 1000.0;

    domain::CombinedCost c;
    // Пешком: 0 руб + усталость. Усталость учитывается при сравнении через вес.
    c.fatigue = dist_km * params.walk_fatigue_per_km;
    c.money_rub = 0.0;
    return c;
}

bool CheapestStrategy::less(const domain::CombinedCost& a, const domain::CombinedCost& b) const {
    // Сравниваем по money_rub + fatigue (с весом 1:1)
    // Усталость учитывается при выборе маршрута, но не в итоговой сумме
    double cost_a = a.money_rub + a.fatigue;
    double cost_b = b.money_rub + b.fatigue;
    if (cost_a != cost_b) return cost_a < cost_b;
    // При равной стоимости - предпочитаем более быстрый
    return a.time_seconds < b.time_seconds;
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

    // Стоимость и усталость тоже считаем для полноты данных
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

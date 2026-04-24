#include "domain/time_variants.h"

#include "application/i_route_strategy.h"
#include "domain/edge.h"

#include <chrono>

namespace domain {

std::optional<EdgeResult> StaticLogic::calculate(
    const Edge& edge,
    const QueryContext& ctx,
    const application::IRouteStrategy& strategy
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

std::optional<EdgeResult> TimeWindowLogic::calculate(
    const Edge& edge, 
    const QueryContext& ctx, 
    const application::IRouteStrategy& strategy
) const {
    std::time_t now_c = std::chrono::system_clock::to_time_t(ctx.departure_time);
    std::tm* now_tm = std::localtime(&now_c);
    int current_hour = now_tm->tm_hour;

    bool is_closed = false;
    if (close_hour_ < open_hour_) {
        is_closed = (current_hour >= close_hour_ && current_hour < open_hour_);
    } else {
        is_closed = (current_hour >= close_hour_ || current_hour < open_hour_);
    }

    if (is_closed) {
        return std::nullopt;
    }

    double speed_mps = ctx.params.speedMps(edge.transport);
    if (speed_mps <= 0.0) return std::nullopt;

    double travel_sec = edge.length_meters / speed_mps;
    auto travel_duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(
        std::chrono::duration<double>(travel_sec)
    );
    auto arrival = ctx.departure_time + travel_duration;

    CombinedCost cost = strategy.create_cost(edge, ctx.params, travel_sec, 0.0);

    return EdgeResult{arrival, cost};
}

// =================== ЛОГИКА МЕТРО ===================
std::optional<EdgeResult> FrequencyBasedLogic::calculate(
    const Edge& edge, const QueryContext& ctx, const application::IRouteStrategy& strategy
) const {
    std::time_t now_c = std::chrono::system_clock::to_time_t(ctx.departure_time);
    std::tm* now_tm = std::localtime(&now_c);
    int hour = now_tm->tm_hour;

    // Определяем час пик (6:00-10:00 и 17:00-21:00)
    bool is_peak = (hour >= 6 && hour < 10) || (hour >= 17 && hour < 21);
    
    // Берем интервал из параметров
    double interval_min = is_peak ? ctx.params.metro_peak_interval_min : ctx.params.metro_offpeak_interval_min;
    
    // Среднее время ожидания — половина интервала
    double wait_sec = (interval_min / 2.0) * 60.0; 

    double speed_mps = ctx.params.speedMps(edge.transport);
    if (speed_mps <= 0.0) return std::nullopt;

    double travel_sec = edge.length_meters / speed_mps;
    auto total_duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(
        std::chrono::duration<double>(travel_sec + wait_sec)
    );
    auto arrival = ctx.departure_time + total_duration;

    CombinedCost cost = strategy.create_cost(edge, ctx.params, travel_sec, wait_sec);
    return EdgeResult{arrival, cost};
}

// =================== ЛОГИКА АВТОБУСА ===================
std::optional<EdgeResult> ScheduledLogic::calculate(
    const Edge& edge, const QueryContext& ctx, const application::IRouteStrategy& strategy
) const {
    if (schedule_.empty()) return std::nullopt;

    std::time_t now_c = std::chrono::system_clock::to_time_t(ctx.departure_time);
    std::tm* now_tm = std::localtime(&now_c);
    int current_minutes = now_tm->tm_hour * 60 + now_tm->tm_min;

    // Ищем ближайший автобус
    int wait_minutes = -1;
    for (int dep_time : schedule_) {
        if (dep_time >= current_minutes) {
            wait_minutes = dep_time - current_minutes;
            break;
        }
    }

    // Если сегодня автобусов больше нет, ждем до первого рейса завтра
    if (wait_minutes == -1) {
        wait_minutes = (1440 - current_minutes) + schedule_.front();
    }

    double wait_sec = wait_minutes * 60.0;
    double speed_mps = ctx.params.speedMps(edge.transport);
    if (speed_mps <= 0.0) return std::nullopt;

    double travel_sec = edge.length_meters / speed_mps;
    auto total_duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(
        std::chrono::duration<double>(travel_sec + wait_sec)
    );
    auto arrival = ctx.departure_time + total_duration;

    CombinedCost cost = strategy.create_cost(edge, ctx.params, travel_sec, wait_sec);
    return EdgeResult{arrival, cost};
}

} // namespace domain

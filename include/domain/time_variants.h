#pragma once

#include "domain/i_time_variant.h"

#include <vector>

namespace domain {

class StaticLogic : public ITimeVariant {
public:
    std::optional<EdgeResult> calculate(
        const Edge& edge,
        const QueryContext& ctx,
        const application::IRouteStrategy& strategy
    ) const override;
};

class TimeWindowLogic : public ITimeVariant {
public:
    TimeWindowLogic(int close_hour, int open_hour) 
        : close_hour_(close_hour), open_hour_(open_hour) {}

    std::optional<EdgeResult> calculate(
        const Edge& edge, 
        const QueryContext& ctx, 
        const application::IRouteStrategy& strategy
    ) const override;

private:
    int close_hour_;
    int open_hour_;
};


// Логика метро: интервал зависит от времени суток
class FrequencyBasedLogic : public ITimeVariant {
public:
    std::optional<EdgeResult> calculate(
        const Edge& edge, 
        const QueryContext& ctx, 
        const application::IRouteStrategy& strategy
    ) const override;
};

// Логика автобусов: ходит по точному расписанию
class ScheduledLogic : public ITimeVariant {
public:
    // Конструктор принимает список минут от начала дня
    explicit ScheduledLogic(std::vector<int> departure_minutes) 
        : schedule_(std::move(departure_minutes)) {}

    std::optional<EdgeResult> calculate(
        const Edge& edge, 
        const QueryContext& ctx, 
        const application::IRouteStrategy& strategy
    ) const override;

private:
    std::vector<int> schedule_;
};

} // namespace domain

#pragma once

#include "i_time_variant.h"

#include <vector>

namespace domain {

/**
 * @brief Статическая логика - для пеших и автомобильных дорог.
 *
 * Время и стоимость рассчитываются по скорости и длине без учёта расписания.
 */
class StaticLogic : public ITimeVariant {
public:
    std::optional<EdgeResult> calculate(
        const Edge& edge,
        const QueryContext& ctx,
        const application::IRouteStrategy& strategy
    ) const override;
};

/**
 * @brief Логика разводных мостов - ребро недоступно в ночное время.
 *
 * Закрыто с close_hour_ до open_hour_. Если close_hour_ < open_hour_,
 * интервал в пределах одних суток, иначе - через полночь.
 */
class TimeWindowLogic : public ITimeVariant {
public:
    /**
     * @brief Конструктор.
     * @param close_hour Час закрытия моста
     * @param open_hour Час открытия моста
     */
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

/**
 * @brief Логика метро - интервал зависит от времени суток.
 *
 * Утром и вечером (6:00-10:00, 17:00-21:00) - пик, интервал из metro_peak_interval_min.
 * Днём - off-peak, интервал из metro_offpeak_interval_min.
 * Среднее время ожидания - половина интервала.
 */
class FrequencyBasedLogic : public ITimeVariant {
public:
    std::optional<EdgeResult> calculate(
        const Edge& edge,
        const QueryContext& ctx,
        const application::IRouteStrategy& strategy
    ) const override;
};

/**
 * @brief Логика автобусов - ходит по точному расписанию.
 *
 * Конструктор принимает список минут от начала дня.
 * Если сегодня автобусов больше нет, ожидание до первого рейса завтра.
 */
class ScheduledLogic : public ITimeVariant {
public:
    /**
     * @brief Конструктор.
     * @param departure_minutes Список времён отправления в минутах от начала дня
     */
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

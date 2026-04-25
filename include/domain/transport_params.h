#pragma once

#include "types.h"

#include <algorithm>

namespace domain {

/**
 * @brief Параметры всех видов транспорта.
 *
 * Содержит скорости, стоимости и интервалы.
 * Значения по умолчанию используются как fallback при отсутствии CSV.
 * Реальные значения загружаются из transport_params.csv через GraphBuilder::buildTransportParams().
 */
struct TransportParams {
    double walk_speed_kmh = 5.0;        ///< Скорость пешком (км/ч)
    double walk_fatigue_per_km = 10.0;  ///< Усталость за 1 км пешком

    double car_speed_kmh = 40.0;        ///< Скорость автомобиля (км/ч)
    double car_fuel_l_per_100km = 8.0;  ///< Расход топлива (л/100км)
    double car_fuel_rub_per_l = 50.0;   ///< Цена топлива (руб/л)

    double taxi_speed_kmh = 40.0;       ///< Скорость такси (км/ч)
    double taxi_price_per_km = 20.0;    ///< Цена такси за км (руб)
    double taxi_base_price = 150.0;     ///< Базовая цена посадки в такси (руб)

    double bus_speed_kmh = 25.0;        ///< Скорость автобуса (км/ч)
    double bus_ticket_rub = 55.0;       ///< Стоимость билета на автобус (руб)

    double metro_speed_kmh = 50.0;              ///< Скорость метро (км/ч)
    double metro_ticket_rub = 55.0;              ///< Стоимость билета на метро (руб)
    double metro_peak_interval_min = 2.0;        ///< Интервал метро в часы пик (мин)
    double metro_offpeak_interval_min = 5.0;     ///< Интервал метро вне часов пик (мин)

    /**
     * @brief Возвращает скорость транспорта в м/с.
     * @param t Тип транспорта
     * @return Скорость в метрах в секунду
     */
    double speedMps(TransportType t) const {
        double kmh = walk_speed_kmh;
        switch (t) {
            case TransportType::Walk:  kmh = walk_speed_kmh;  break;
            case TransportType::Car:   kmh = car_speed_kmh;   break;
            case TransportType::Taxi:  kmh = taxi_speed_kmh;  break;
            case TransportType::Bus:   kmh = bus_speed_kmh;   break;
            case TransportType::Metro: kmh = metro_speed_kmh; break;
        }
        return kmh * 1000.0 / 3600.0;
    }

    /**
     * @brief Возвращает максимальную скорость среди всех видов транспорта в м/с.
     * @return Максимальная скорость в метрах в секунду
     */
    double maxSpeedMps() const {
        double kmh = std::max({
            walk_speed_kmh, car_speed_kmh, taxi_speed_kmh,
            bus_speed_kmh, metro_speed_kmh
        });
        return kmh * 1000.0 / 3600.0;
    }
};

} // namespace domain

#pragma once

#include "domain/types.h"

#include <algorithm>

struct TransportParams {
    double walk_speed_kmh = 5.0;
    double walk_fatigue_per_km = 10.0;

    double car_speed_kmh = 40.0;
    double car_fuel_l_per_100km = 8.0;
    double car_fuel_rub_per_l = 50.0;

    double taxi_speed_kmh = 40.0;
    double taxi_price_per_km = 20.0;
    double taxi_base_price = 150.0;

    double bus_speed_kmh = 25.0;
    double bus_ticket_rub = 55.0;

    double metro_speed_kmh = 50.0;
    double metro_ticket_rub = 55.0;
    double metro_peak_interval_min = 2.0;
    double metro_offpeak_interval_min = 5.0;

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

    double maxSpeedMps() const {
        double kmh = std::max({
            walk_speed_kmh, car_speed_kmh, taxi_speed_kmh,
            bus_speed_kmh, metro_speed_kmh
        });
        return kmh * 1000.0 / 3600.0;
    }
};

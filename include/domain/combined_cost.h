#pragma once

namespace domain {

struct CombinedCost {
    double time_seconds = 0.0;
    double money_rub = 0.0;
    double fatigue = 0.0;
    int transfers = 0;
};

inline CombinedCost operator+(const CombinedCost& a, const CombinedCost& b) {
    return CombinedCost{
        a.time_seconds + b.time_seconds,
        a.money_rub + b.money_rub,
        a.fatigue + b.fatigue,
        a.transfers + b.transfers
    };
}

} // namespace domain

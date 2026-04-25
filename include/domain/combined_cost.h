#pragma once

namespace domain {

/**
 * @brief Комбинированная стоимость маршрута.
 *
 * Содержит время, деньги, усталость и количество пересадок.
 * Стратегия поиска определяет, какие поля важны для сравнения.
 */
struct CombinedCost {
    double time_seconds = 0.0;  ///< Время в пути (секунды)
    double money_rub = 0.0;     ///< Денежная стоимость (рубли)
    double fatigue = 0.0;       ///< Усталость (условные единицы)
    int transfers = 0;          ///< Количество пересадок
};

/**
 * @brief Сложение двух стоимостей.
 * @param a Первая стоимость
 * @param b Вторая стоимость
 * @return Суммарная стоимость
 */
inline CombinedCost operator+(const CombinedCost& a, const CombinedCost& b) {
    return CombinedCost{
        a.time_seconds + b.time_seconds,
        a.money_rub + b.money_rub,
        a.fatigue + b.fatigue,
        a.transfers + b.transfers
    };
}

} // namespace domain

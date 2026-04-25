#include "application/strategies.h"
#include "domain/edge.h"
#include "domain/time_variants.h"
#include "domain/transport_params.h"

#include <gtest/gtest.h>

// ==================== Тесты CheapestStrategy ====================

TEST(CheapestStrategyTest, WalkCostIsFatigueNotMoney) {
    application::CheapestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Walk;
    edge.length_meters = 1000.0;  // 1 км

    auto cost = strategy.create_cost(edge, params, 100.0, 0.0);

    EXPECT_DOUBLE_EQ(cost.money_rub, 0.0);
    EXPECT_DOUBLE_EQ(cost.fatigue, 10.0);  // 1 км * 10 fatigue_per_km
    EXPECT_DOUBLE_EQ(cost.time_seconds, 100.0);
}

TEST(CheapestStrategyTest, CarCostIsFuel) {
    application::CheapestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Car;
    edge.length_meters = 10000.0;  // 10 км

    auto cost = strategy.create_cost(edge, params, 100.0, 0.0);

    // 10 км * (8 л/100км / 100) * 50 руб/л = 10 * 0.08 * 50 = 40 руб
    EXPECT_DOUBLE_EQ(cost.money_rub, 40.0);
    EXPECT_DOUBLE_EQ(cost.fatigue, 0.0);
}

TEST(CheapestStrategyTest, BusCostIsFixedTicket) {
    application::CheapestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Bus;
    edge.length_meters = 5000.0;  // 5 км

    auto cost = strategy.create_cost(edge, params, 100.0, 0.0);

    EXPECT_DOUBLE_EQ(cost.money_rub, 55.0);  // Фиксированная цена билета
    EXPECT_DOUBLE_EQ(cost.fatigue, 0.0);
}

TEST(CheapestStrategyTest, MetroCostIsFixedTicket) {
    application::CheapestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Metro;
    edge.length_meters = 10000.0;  // 10 км

    auto cost = strategy.create_cost(edge, params, 100.0, 0.0);

    EXPECT_DOUBLE_EQ(cost.money_rub, 55.0);
    EXPECT_DOUBLE_EQ(cost.fatigue, 0.0);
}

TEST(CheapestStrategyTest, TaxiCostIsBasePlusPerKm) {
    application::CheapestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Taxi;
    edge.length_meters = 5000.0;  // 5 км

    auto cost = strategy.create_cost(edge, params, 100.0, 0.0);

    // 150 + 5 * 20 = 250 руб
    EXPECT_DOUBLE_EQ(cost.money_rub, 250.0);
    EXPECT_DOUBLE_EQ(cost.fatigue, 0.0);
}

TEST(CheapestStrategyTest, LessComparesMoneyPlusFatigue) {
    application::CheapestStrategy strategy;

    domain::CombinedCost a;
    a.money_rub = 10.0;
    a.fatigue = 5.0;

    domain::CombinedCost b;
    b.money_rub = 20.0;
    b.fatigue = 0.0;

    // a: 10+5=15, b: 20+0=20 => a < b
    EXPECT_TRUE(strategy.less(a, b));
    EXPECT_FALSE(strategy.less(b, a));
}

TEST(CheapestStrategyTest, LessTiebreaksByTime) {
    application::CheapestStrategy strategy;

    domain::CombinedCost a;
    a.money_rub = 10.0;
    a.fatigue = 5.0;
    a.time_seconds = 200.0;

    domain::CombinedCost b;
    b.money_rub = 10.0;
    b.fatigue = 5.0;
    b.time_seconds = 100.0;

    // Одинаковая стоимость, но b быстрее
    EXPECT_TRUE(strategy.less(b, a));
}

TEST(CheapestStrategyTest, HeuristicIsAdmissible) {
    application::CheapestStrategy strategy;
    domain::TransportParams params;

    domain::Node from;
    from.coords = domain::Point{0.0, 0.0};

    domain::Node to;
    to.coords = domain::Point{1000.0, 0.0};  // 1 км

    auto h = strategy.heuristic(from, to, params);

    // Эвристика: пешком 1 км = 10 fatigue, 0 руб
    EXPECT_DOUBLE_EQ(h.fatigue, 10.0);
    EXPECT_DOUBLE_EQ(h.money_rub, 0.0);
}

// ==================== Тесты MostConvenientStrategy ====================

TEST(MostConvenientStrategyTest, WalkCostHasZeroTransfers) {
    application::MostConvenientStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Walk;
    edge.length_meters = 1000.0;

    auto cost = strategy.create_cost(edge, params, 100.0, 0.0);

    EXPECT_EQ(cost.transfers, 0);
}

TEST(MostConvenientStrategyTest, BusCostHasCorrectMoney) {
    application::MostConvenientStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Bus;
    edge.length_meters = 5000.0;

    auto cost = strategy.create_cost(edge, params, 100.0, 0.0);

    EXPECT_DOUBLE_EQ(cost.money_rub, 55.0);
}

TEST(MostConvenientStrategyTest, LessComparesTransfersFirst) {
    application::MostConvenientStrategy strategy;

    domain::CombinedCost a;
    a.transfers = 0;
    a.time_seconds = 1000.0;

    domain::CombinedCost b;
    b.transfers = 1;
    b.time_seconds = 100.0;

    // a имеет меньше пересадок, даже если медленнее
    EXPECT_TRUE(strategy.less(a, b));
}

TEST(MostConvenientStrategyTest, LessTiebreaksByTime) {
    application::MostConvenientStrategy strategy;

    domain::CombinedCost a;
    a.transfers = 1;
    a.time_seconds = 200.0;

    domain::CombinedCost b;
    b.transfers = 1;
    b.time_seconds = 100.0;

    // Одинаковое число пересадок, b быстрее
    EXPECT_TRUE(strategy.less(b, a));
}

TEST(MostConvenientStrategyTest, HeuristicIsZeroTransfers) {
    application::MostConvenientStrategy strategy;
    domain::TransportParams params;

    domain::Node from;
    from.coords = domain::Point{0.0, 0.0};

    domain::Node to;
    to.coords = domain::Point{1000.0, 0.0};

    auto h = strategy.heuristic(from, to, params);

    // Допустимая эвристика: 0 пересадок
    EXPECT_EQ(h.transfers, 0);
}

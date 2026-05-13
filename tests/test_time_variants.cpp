#include "application/strategies.h"
#include "domain/edge.h"
#include "domain/time_variants.h"
#include "domain/transport_params.h"

#include <gtest/gtest.h>

#include <chrono>
#include <ctime>
#include <memory>

// Вспомогательная функция для создания time_point с корректной инициализацией tm
// (Windows mktime требует валидных tm_mday/tm_mon/tm_year)
static std::chrono::system_clock::time_point makeTime(int hour, int min) {
    std::tm tm = {};
    tm.tm_year = 124;   // 2024
    tm.tm_mon  = 0;     // January
    tm.tm_mday = 1;
    tm.tm_hour = hour;
    tm.tm_min  = min;
    tm.tm_sec  = 0;
    tm.tm_isdst = 0;
    std::time_t t = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(t);
}

// ==================== Тесты TimeWindowLogic ====================

TEST(TimeWindowLogicTest, BridgeOpenDuringDay) {
    // Мост закрыт с 1:00 до 5:00
    domain::TimeWindowLogic logic(1, 5);
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Car;
    edge.length_meters = 1000.0;

    // 12:00 - мост открыт
    auto departure = makeTime(12, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    ASSERT_TRUE(result.has_value());
    EXPECT_GT(result->cost.time_seconds, 0.0);
}

TEST(TimeWindowLogicTest, BridgeClosedAtNight) {
    // Мост закрыт с 1:00 до 5:00
    domain::TimeWindowLogic logic(1, 5);
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Car;
    edge.length_meters = 1000.0;

    // 3:00 - мост закрыт
    auto departure = makeTime(3, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    EXPECT_FALSE(result.has_value());
}

TEST(TimeWindowLogicTest, BridgeClosedAtBoundaryClose) {
    // Мост закрыт с 1:00 до 5:00
    domain::TimeWindowLogic logic(1, 5);
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Car;
    edge.length_meters = 1000.0;

    // 1:00 - граница закрытия
    auto departure = makeTime(1, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    EXPECT_FALSE(result.has_value());
}

TEST(TimeWindowLogicTest, BridgeOpenAtBoundaryOpen) {
    // Мост закрыт с 1:00 до 5:00
    domain::TimeWindowLogic logic(1, 5);
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Car;
    edge.length_meters = 1000.0;

    // 5:00 - граница открытия
    auto departure = makeTime(5, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    EXPECT_TRUE(result.has_value());
}

TEST(TimeWindowLogicTest, BridgeClosedOverMidnight) {
    // Мост закрыт с 23:00 до 5:00 (через полночь)
    domain::TimeWindowLogic logic(23, 5);
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Car;
    edge.length_meters = 1000.0;

    // 23:00 - закрыт
    auto dep23 = makeTime(23, 0);
    domain::QueryContext ctx23{dep23, params};
    EXPECT_FALSE(logic.calculate(edge, ctx23, strategy).has_value());

    // 2:00 - закрыт
    auto dep2 = makeTime(2, 0);
    domain::QueryContext ctx2{dep2, params};
    EXPECT_FALSE(logic.calculate(edge, ctx2, strategy).has_value());

    // 10:00 - открыт
    auto dep10 = makeTime(10, 0);
    domain::QueryContext ctx10{dep10, params};
    EXPECT_TRUE(logic.calculate(edge, ctx10, strategy).has_value());
}

// ==================== Тесты FrequencyBasedLogic ====================

TEST(FrequencyBasedLogicTest, MetroPeakInterval) {
    domain::FrequencyBasedLogic logic;
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Metro;
    edge.length_meters = 5000.0;

    // 8:00 - час пик (6-10), интервал 2 мин, ожидание 1 мин
    auto departure = makeTime(8, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    ASSERT_TRUE(result.has_value());

    double travel_sec = 5000.0 / params.speedMps(domain::TransportType::Metro);
    double wait_sec = (2.0 / 2.0) * 60.0;  // 60 сек
    EXPECT_NEAR(result->cost.time_seconds, travel_sec + wait_sec, 1e-6);
}

TEST(FrequencyBasedLogicTest, MetroOffPeakInterval) {
    domain::FrequencyBasedLogic logic;
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Metro;
    edge.length_meters = 5000.0;

    // 14:00 - не час пик, интервал 5 мин, ожидание 2.5 мин
    auto departure = makeTime(14, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    ASSERT_TRUE(result.has_value());

    double travel_sec = 5000.0 / params.speedMps(domain::TransportType::Metro);
    double wait_sec = (5.0 / 2.0) * 60.0;  // 150 сек
    EXPECT_NEAR(result->cost.time_seconds, travel_sec + wait_sec, 1e-6);
}

TEST(FrequencyBasedLogicTest, MetroEveningPeak) {
    domain::FrequencyBasedLogic logic;
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Metro;
    edge.length_meters = 5000.0;

    // 18:00 - вечерний час пик (17-21)
    auto departure = makeTime(18, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    ASSERT_TRUE(result.has_value());

    double wait_sec = (2.0 / 2.0) * 60.0;  // пик интервал
    double travel_sec = 5000.0 / params.speedMps(domain::TransportType::Metro);
    EXPECT_NEAR(result->cost.time_seconds, travel_sec + wait_sec, 1e-6);
}

// ==================== Тесты ScheduledLogic ====================

TEST(ScheduledLogicTest, BusWaitsForNextDeparture) {
    // Автобус ходит в 08:00, 08:15, 08:30
    domain::ScheduledLogic logic({480, 495, 510});  // минуты от начала дня
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Bus;
    edge.length_meters = 2500.0;

    // Приходим в 08:10 - ждём до 08:15 (5 мин)
    auto departure = makeTime(8, 10);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    ASSERT_TRUE(result.has_value());

    double travel_sec = 2500.0 / params.speedMps(domain::TransportType::Bus);
    double wait_sec = 5.0 * 60.0;  // 5 минут ожидания
    EXPECT_NEAR(result->cost.time_seconds, travel_sec + wait_sec, 1e-6);
}

TEST(ScheduledLogicTest, BusArrivesExactlyAtDeparture) {
    // Автобус ходит в 08:00, 08:15
    domain::ScheduledLogic logic({480, 495});
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Bus;
    edge.length_meters = 2500.0;

    // Приходим ровно в 08:00 - ожидание 0
    auto departure = makeTime(8, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    ASSERT_TRUE(result.has_value());

    double travel_sec = 2500.0 / params.speedMps(domain::TransportType::Bus);
    double wait_sec = 0.0;
    EXPECT_NEAR(result->cost.time_seconds, travel_sec + wait_sec, 1e-6);
}

TEST(ScheduledLogicTest, BusAfterLastDepartureWaitsUntilTomorrow) {
    // Автобус ходит только в 08:00, 08:15
    domain::ScheduledLogic logic({480, 495});
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Bus;
    edge.length_meters = 2500.0;

    // Приходим в 22:00 - все рейсы прошли, ждём до завтра 08:00
    auto departure = makeTime(22, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    ASSERT_TRUE(result.has_value());

    // Ожидание: (1440 - 1320) + 480 = 120 + 480 = 600 минут
    double wait_sec = 600.0 * 60.0;
    double travel_sec = 2500.0 / params.speedMps(domain::TransportType::Bus);
    EXPECT_NEAR(result->cost.time_seconds, travel_sec + wait_sec, 1e-6);
}

TEST(ScheduledLogicTest, EmptyScheduleReturnsNullopt) {
    // Пустое расписание - автобусов нет
    domain::ScheduledLogic logic({});
    application::FastestStrategy strategy;
    domain::TransportParams params;

    domain::Edge edge;
    edge.transport = domain::TransportType::Bus;
    edge.length_meters = 2500.0;

    auto departure = makeTime(8, 0);
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    EXPECT_FALSE(result.has_value());
}

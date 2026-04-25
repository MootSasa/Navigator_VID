#include "application/strategies.h"
#include "domain/edge.h"
#include "domain/time_variants.h"
#include "domain/transport_params.h"

#include <gtest/gtest.h>

#include <chrono>
#include <ctime>
#include <memory>

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
    std::tm tm = {};
    tm.tm_hour = 12;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm = {};
    tm.tm_hour = 3;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm = {};
    tm.tm_hour = 1;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm = {};
    tm.tm_hour = 5;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm23 = {};
    tm23.tm_hour = 23;
    tm23.tm_min = 0;
    auto dep23 = std::chrono::system_clock::from_time_t(std::mktime(&tm23));
    domain::QueryContext ctx23{dep23, params};
    EXPECT_FALSE(logic.calculate(edge, ctx23, strategy).has_value());

    // 2:00 - закрыт
    std::tm tm2 = {};
    tm2.tm_hour = 2;
    tm2.tm_min = 0;
    auto dep2 = std::chrono::system_clock::from_time_t(std::mktime(&tm2));
    domain::QueryContext ctx2{dep2, params};
    EXPECT_FALSE(logic.calculate(edge, ctx2, strategy).has_value());

    // 10:00 - открыт
    std::tm tm10 = {};
    tm10.tm_hour = 10;
    tm10.tm_min = 0;
    auto dep10 = std::chrono::system_clock::from_time_t(std::mktime(&tm10));
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
    std::tm tm = {};
    tm.tm_hour = 8;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm = {};
    tm.tm_hour = 14;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm = {};
    tm.tm_hour = 18;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm = {};
    tm.tm_hour = 8;
    tm.tm_min = 10;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm = {};
    tm.tm_hour = 8;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
    std::tm tm = {};
    tm.tm_hour = 22;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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

    std::tm tm = {};
    tm.tm_hour = 8;
    tm.tm_min = 0;
    auto departure = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    domain::QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    EXPECT_FALSE(result.has_value());
}

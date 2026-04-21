#include "application/strategies.h"
#include "domain/edge.h"
#include "domain/time_variants.h"
#include "domain/transport_params.h"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>

TEST(StaticLogicTest, WalkTravelTime) {
    TransportParams params;
    StaticLogic logic;
    FastestStrategy strategy;

    Edge edge;
    edge.id = 1;
    edge.from = 1;
    edge.to = 2;
    edge.transport = TransportType::Walk;
    edge.length_meters = 1000.0;

    std::chrono::system_clock::time_point departure{};
    QueryContext ctx{departure, params};

    auto result = logic.calculate(edge, ctx, strategy);
    ASSERT_TRUE(result.has_value());

    double expected_sec = 1000.0 / (5.0 * 1000.0 / 3600.0);
    EXPECT_NEAR(result->cost.time_seconds, expected_sec, 1e-6);

    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(
        result->arrival_time - departure
    ).count();
    EXPECT_NEAR(delta, expected_sec, 1e-3);
}

TEST(StaticLogicTest, CarFasterThanWalk) {
    TransportParams params;
    StaticLogic logic;
    FastestStrategy strategy;

    Edge walk_edge;
    walk_edge.transport = TransportType::Walk;
    walk_edge.length_meters = 1000.0;

    Edge car_edge;
    car_edge.transport = TransportType::Car;
    car_edge.length_meters = 1000.0;

    std::chrono::system_clock::time_point departure{};
    QueryContext ctx{departure, params};

    auto w = logic.calculate(walk_edge, ctx, strategy);
    auto c = logic.calculate(car_edge, ctx, strategy);
    ASSERT_TRUE(w.has_value());
    ASSERT_TRUE(c.has_value());
    EXPECT_LT(c->cost.time_seconds, w->cost.time_seconds);
}

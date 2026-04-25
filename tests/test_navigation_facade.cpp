#include "application/navigation_facade.h"
#include "domain/edge.h"
#include "domain/graph.h"
#include "domain/node.h"
#include "domain/time_variants.h"
#include "domain/types.h"

#include <gtest/gtest.h>

#include <memory>

// ==================== Вспомогательные функции ====================

static domain::Node makeNode(domain::NodeId id, double x, double y, domain::NodeType type = domain::NodeType::Intersection) {
    domain::Node n;
    n.id = id;
    n.type = type;
    n.coords = domain::Point{x, y};
    return n;
}

static domain::Edge makeEdge(domain::EdgeId id, domain::NodeId from, domain::NodeId to,
                              domain::TransportType t, double len, const std::string& name = "") {
    domain::Edge e;
    e.id = id;
    e.from = from;
    e.to = to;
    e.transport = t;
    e.length_meters = len;
    e.name = name;
    e.logic = std::make_unique<domain::StaticLogic>();
    return e;
}

// Создаёт тестовый граф с несколькими маршрутами
static application::NavigationFacade makeTestFacade() {
    auto graph = std::make_unique<domain::Graph>();
    domain::TransportParams params;

    // Узлы
    graph->addNode(makeNode(1, 0, 0));
    graph->addNode(makeNode(2, 500, 0, domain::NodeType::BusStop));
    graph->addNode(makeNode(3, 1000, 0, domain::NodeType::BusStop));
    graph->addNode(makeNode(4, 1000, 500, domain::NodeType::MetroStation));

    // Пешеходные рёбра
    graph->addEdge(makeEdge(1, 1, 2, domain::TransportType::Walk, 500, "Пешком"));
    graph->addEdge(makeEdge(2, 2, 3, domain::TransportType::Walk, 500, "Пешком"));
    graph->addEdge(makeEdge(3, 3, 4, domain::TransportType::Walk, 500, "Пешком"));

    // Автобус
    graph->addEdge(makeEdge(10, 1, 3, domain::TransportType::Bus, 1000, "Авт. 1"));

    // Метро
    graph->addEdge(makeEdge(20, 2, 4, domain::TransportType::Metro, 700, "Линия 1"));

    return application::NavigationFacade(std::move(graph), std::move(params));
}

// ==================== Тесты NavigationFacade ====================

TEST(NavigationFacadeTest, FindRouteFastest) {
    auto facade = makeTestFacade();

    auto result = facade.findRoute(1, 3, "fastest", std::chrono::system_clock::time_point{});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->nodes.front(), 1);
    EXPECT_EQ(result->nodes.back(), 3);
}

TEST(NavigationFacadeTest, FindRouteCheapest) {
    auto facade = makeTestFacade();

    auto result = facade.findRoute(1, 3, "cheapest", std::chrono::system_clock::time_point{});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->nodes.front(), 1);
    EXPECT_EQ(result->nodes.back(), 3);
    // Пешком дешевле (0 руб + усталость), чем автобус (55 руб)
    EXPECT_DOUBLE_EQ(result->total_cost.money_rub, 0.0);
}

TEST(NavigationFacadeTest, FindRouteConvenient) {
    auto facade = makeTestFacade();

    auto result = facade.findRoute(1, 3, "convenient", std::chrono::system_clock::time_point{});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->nodes.front(), 1);
    EXPECT_EQ(result->nodes.back(), 3);
}

TEST(NavigationFacadeTest, FindAllRoutes) {
    auto facade = makeTestFacade();

    auto results = facade.findAllRoutes(1, 3, std::chrono::system_clock::time_point{});
    ASSERT_EQ(results.size(), 3u);  // fastest, cheapest, convenient

    for (const auto& r : results) {
        EXPECT_EQ(r.nodes.front(), 1);
        EXPECT_EQ(r.nodes.back(), 3);
    }
}

TEST(NavigationFacadeTest, UnreachableGoal) {
    auto facade = makeTestFacade();

    // Узел 4 не связан с 1 напрямую через удобный путь, но связан через 2->4 метро
    // Попробуем несуществующий узел
    auto result = facade.findRoute(1, 999, "fastest", std::chrono::system_clock::time_point{});
    EXPECT_FALSE(result.has_value());
}

TEST(NavigationFacadeTest, HasNode) {
    auto facade = makeTestFacade();

    EXPECT_TRUE(facade.hasNode(1));
    EXPECT_TRUE(facade.hasNode(2));
    EXPECT_FALSE(facade.hasNode(999));
}

TEST(NavigationFacadeTest, GetGraph) {
    auto facade = makeTestFacade();

    const auto& graph = facade.getGraph();
    EXPECT_EQ(graph.nodeCount(), 4u);
}

TEST(NavigationFacadeTest, GetParams) {
    auto facade = makeTestFacade();

    const auto& params = facade.getParams();
    EXPECT_DOUBLE_EQ(params.walk_speed_kmh, 5.0);
    EXPECT_DOUBLE_EQ(params.bus_ticket_rub, 55.0);
}

TEST(NavigationFacadeTest, DefaultStrategyIsFastest) {
    auto facade = makeTestFacade();

    // Без указания стратегии - fastest (по умолчанию)
    auto result = facade.findRoute(1, 3);
    ASSERT_TRUE(result.has_value());
}

TEST(NavigationFacadeTest, UnknownStrategyDefaultsToFastest) {
    auto facade = makeTestFacade();

    auto result = facade.findRoute(1, 3, "unknown_strategy", std::chrono::system_clock::time_point{});
    ASSERT_TRUE(result.has_value());
}

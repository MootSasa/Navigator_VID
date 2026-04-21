#include "application/a_star_solver.h"
#include "application/strategies.h"
#include "domain/edge.h"
#include "domain/graph.h"
#include "domain/time_variants.h"
#include "domain/transport_params.h"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>

namespace {

Node node(NodeId id, double x, double y) {
    Node n;
    n.id = id;
    n.coords = Point{x, y};
    return n;
}

Edge edge(EdgeId id, NodeId from, NodeId to, TransportType t, double len, const std::string& name = "") {
    Edge e;
    e.id = id;
    e.from = from;
    e.to = to;
    e.transport = t;
    e.length_meters = len;
    e.name = name;
    e.logic = std::make_unique<StaticLogic>();
    return e;
}

}

TEST(AStarTest, DirectVsDetour) {
    Graph g;
    g.addNode(node(1, 0, 0));
    g.addNode(node(2, 500, 0));
    g.addNode(node(3, 1000, 0));
    g.addNode(node(4, 500, 500));

    g.addEdge(edge(10, 1, 2, TransportType::Walk, 500));
    g.addEdge(edge(11, 2, 3, TransportType::Walk, 500));
    g.addEdge(edge(12, 1, 4, TransportType::Walk, 707));
    g.addEdge(edge(13, 4, 3, TransportType::Walk, 707));

    TransportParams params;
    FastestStrategy strategy;
    auto result = findRoute(g, 1, 3, strategy, params, std::chrono::system_clock::time_point{});

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->nodes.size(), 3u);
    EXPECT_EQ(result->nodes[0], 1);
    EXPECT_EQ(result->nodes[1], 2);
    EXPECT_EQ(result->nodes[2], 3);
    ASSERT_EQ(result->edges.size(), 2u);
    EXPECT_EQ(result->edges[0], 10);
    EXPECT_EQ(result->edges[1], 11);
}

TEST(AStarTest, UnreachableGoal) {
    Graph g;
    g.addNode(node(1, 0, 0));
    g.addNode(node(2, 100, 0));
    g.addNode(node(3, 200, 0));

    g.addEdge(edge(10, 1, 2, TransportType::Walk, 100));

    TransportParams params;
    FastestStrategy strategy;
    auto result = findRoute(g, 1, 3, strategy, params, std::chrono::system_clock::time_point{});
    EXPECT_FALSE(result.has_value());
}

TEST(AStarTest, FastestPrefersFasterTransport) {
    Graph g;
    g.addNode(node(1, 0, 0));
    g.addNode(node(2, 1000, 0));

    g.addEdge(edge(10, 1, 2, TransportType::Walk, 1000));
    g.addEdge(edge(11, 1, 2, TransportType::Car, 1000));

    TransportParams params;
    FastestStrategy strategy;
    auto result = findRoute(g, 1, 2, strategy, params, std::chrono::system_clock::time_point{});

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->edges.size(), 1u);
    EXPECT_EQ(result->edges[0], 11);
}

TEST(AStarTest, StartEqualsGoal) {
    Graph g;
    g.addNode(node(1, 0, 0));

    TransportParams params;
    FastestStrategy strategy;
    auto result = findRoute(g, 1, 1, strategy, params, std::chrono::system_clock::time_point{});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->nodes.size(), 1u);
    EXPECT_TRUE(result->edges.empty());
    EXPECT_DOUBLE_EQ(result->total_cost.time_seconds, 0.0);
}

TEST(AStarTest, TransferCountedOnBusRouteChange) {
    Graph g;
    g.addNode(node(1, 0, 0));
    g.addNode(node(2, 500, 0));
    g.addNode(node(3, 1000, 0));

    g.addEdge(edge(10, 1, 2, TransportType::Bus, 500, "Авт. 1"));
    g.addEdge(edge(11, 2, 3, TransportType::Bus, 500, "Авт. 2"));

    TransportParams params;
    FastestStrategy strategy;
    auto result = findRoute(g, 1, 3, strategy, params, std::chrono::system_clock::time_point{});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->total_cost.transfers, 1);
}

TEST(AStarTest, NoTransferOnSameBusRoute) {
    Graph g;
    g.addNode(node(1, 0, 0));
    g.addNode(node(2, 500, 0));
    g.addNode(node(3, 1000, 0));

    g.addEdge(edge(10, 1, 2, TransportType::Bus, 500, "Авт. 1"));
    g.addEdge(edge(11, 2, 3, TransportType::Bus, 500, "Авт. 1"));

    TransportParams params;
    FastestStrategy strategy;
    auto result = findRoute(g, 1, 3, strategy, params, std::chrono::system_clock::time_point{});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->total_cost.transfers, 0);
}

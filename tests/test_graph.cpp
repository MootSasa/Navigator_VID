#include "domain/graph.h"
#include "domain/time_variants.h"

#include <gtest/gtest.h>

#include <memory>

namespace {

domain::Node makeNode(domain::NodeId id, double x, double y) {
    domain::Node n;
    n.id = id;
    n.type = domain::NodeType::Intersection;
    n.coords = domain::Point{x, y};
    return n;
}

domain::Edge makeEdge(domain::EdgeId id, domain::NodeId from, domain::NodeId to, domain::TransportType t, double length) {
    domain::Edge e;
    e.id = id;
    e.from = from;
    e.to = to;
    e.transport = t;
    e.length_meters = length;
    e.logic = std::make_unique<domain::StaticLogic>();
    return e;
}

}

TEST(GraphTest, AddAndReadNodes) {
    domain::Graph g;
    g.addNode(makeNode(1, 0, 0));
    g.addNode(makeNode(2, 100, 0));

    EXPECT_EQ(g.nodeCount(), 2u);
    EXPECT_TRUE(g.hasNode(1));
    EXPECT_TRUE(g.hasNode(2));
    EXPECT_FALSE(g.hasNode(3));
    EXPECT_DOUBLE_EQ(g.getNode(2).coords.x, 100.0);
}

TEST(GraphTest, AdjacencyList) {
    domain::Graph g;
    g.addNode(makeNode(1, 0, 0));
    g.addNode(makeNode(2, 100, 0));
    g.addNode(makeNode(3, 100, 100));

    g.addEdge(makeEdge(10, 1, 2, domain::TransportType::Walk, 100));
    g.addEdge(makeEdge(11, 1, 3, domain::TransportType::Car, 200));
    g.addEdge(makeEdge(12, 2, 3, domain::TransportType::Walk, 100));

    EXPECT_EQ(g.getEdgesFrom(1).size(), 2u);
    EXPECT_EQ(g.getEdgesFrom(2).size(), 1u);
    EXPECT_EQ(g.getEdgesFrom(3).size(), 0u);

    EXPECT_EQ(g.getEdgesFrom(1)[0].to, 2);
    EXPECT_EQ(g.getEdgesFrom(1)[1].to, 3);
}

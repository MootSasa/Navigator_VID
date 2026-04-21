#include "domain/graph.h"
#include "domain/time_variants.h"

#include <gtest/gtest.h>

#include <memory>

namespace {

Node makeNode(NodeId id, double x, double y) {
    Node n;
    n.id = id;
    n.type = NodeType::Intersection;
    n.coords = Point{x, y};
    return n;
}

Edge makeEdge(EdgeId id, NodeId from, NodeId to, TransportType t, double length) {
    Edge e;
    e.id = id;
    e.from = from;
    e.to = to;
    e.transport = t;
    e.length_meters = length;
    e.logic = std::make_unique<StaticLogic>();
    return e;
}

}

TEST(GraphTest, AddAndReadNodes) {
    Graph g;
    g.addNode(makeNode(1, 0, 0));
    g.addNode(makeNode(2, 100, 0));

    EXPECT_EQ(g.nodeCount(), 2u);
    EXPECT_TRUE(g.hasNode(1));
    EXPECT_TRUE(g.hasNode(2));
    EXPECT_FALSE(g.hasNode(3));
    EXPECT_DOUBLE_EQ(g.getNode(2).coords.x, 100.0);
}

TEST(GraphTest, AdjacencyList) {
    Graph g;
    g.addNode(makeNode(1, 0, 0));
    g.addNode(makeNode(2, 100, 0));
    g.addNode(makeNode(3, 100, 100));

    g.addEdge(makeEdge(10, 1, 2, TransportType::Walk, 100));
    g.addEdge(makeEdge(11, 1, 3, TransportType::Car, 200));
    g.addEdge(makeEdge(12, 2, 3, TransportType::Walk, 100));

    EXPECT_EQ(g.getEdgesFrom(1).size(), 2u);
    EXPECT_EQ(g.getEdgesFrom(2).size(), 1u);
    EXPECT_EQ(g.getEdgesFrom(3).size(), 0u);

    EXPECT_EQ(g.getEdgesFrom(1)[0].to, 2);
    EXPECT_EQ(g.getEdgesFrom(1)[1].to, 3);
}

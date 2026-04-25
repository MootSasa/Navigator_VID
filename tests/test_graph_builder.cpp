#include "infrastructure/graph_builder.h"

#include "domain/edge.h"
#include "domain/graph.h"
#include "domain/node.h"
#include "domain/types.h"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <cstdio>

using namespace infrastructure;

// Вспомогательная функция: записать содержимое во временный файл и вернуть путь
static std::string writeTempFile(const std::string& name, const std::string& content) {
    std::string path = "/tmp/navigator_test_" + name;
    std::ofstream out(path);
    out << content;
    return path;
}

// Вспомогательная функция: удалить временный файл
static void removeTempFile(const std::string& path) {
    std::remove(path.c_str());
}

// ==================== Тесты базовой сборки ====================

TEST(GraphBuilderTest, BuildGraphFromCsvFiles) {
    auto nodes_path = writeTempFile("nodes.csv",
        "node_id,type,name,x,y\n"
        "1,intersection,\"\",0,0\n"
        "2,bus_stop,\"Stop A\",500,0\n"
        "3,metro_station,\"Station B\",1000,0\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "edge_id,from_node,to_node,transport_type,length_meters,name\n"
        "10,1,2,walk,500,\"Road\"\n"
        "11,2,3,bus,500,\"Bus route\"\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path).loadEdges(edges_path);
    auto graph = builder.build();

    ASSERT_TRUE(graph != nullptr);
    EXPECT_EQ(graph->nodeCount(), 3u);
    EXPECT_TRUE(graph->hasNode(1));
    EXPECT_TRUE(graph->hasNode(2));
    EXPECT_TRUE(graph->hasNode(3));

    // Проверяем свойства узлов
    const auto& n2 = graph->getNode(2);
    EXPECT_EQ(n2.type, domain::NodeType::BusStop);
    EXPECT_EQ(n2.name, "Stop A");
    EXPECT_DOUBLE_EQ(n2.coords.x, 500.0);

    const auto& n3 = graph->getNode(3);
    EXPECT_EQ(n3.type, domain::NodeType::MetroStation);
    EXPECT_EQ(n3.name, "Station B");

    // Проверяем рёбра
    const auto& edges_from_1 = graph->getEdgesFrom(1);
    ASSERT_EQ(edges_from_1.size(), 1u);
    EXPECT_EQ(edges_from_1[0].to, 2);
    EXPECT_EQ(edges_from_1[0].transport, domain::TransportType::Walk);
    EXPECT_DOUBLE_EQ(edges_from_1[0].length_meters, 500.0);

    const auto& edges_from_2 = graph->getEdgesFrom(2);
    ASSERT_EQ(edges_from_2.size(), 1u);
    EXPECT_EQ(edges_from_2[0].transport, domain::TransportType::Bus);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
}

TEST(GraphBuilderTest, AllNodeTypesMapped) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,Cross,0,0\n"
        "2,bus_stop,BusStop,1,1\n"
        "3,metro_station,Metro,2,2\n"
        "4,poi,POI,3,3\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "10,1,2,walk,100,w\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path).loadEdges(edges_path);
    auto graph = builder.build();

    EXPECT_EQ(graph->getNode(1).type, domain::NodeType::Intersection);
    EXPECT_EQ(graph->getNode(2).type, domain::NodeType::BusStop);
    EXPECT_EQ(graph->getNode(3).type, domain::NodeType::MetroStation);
    EXPECT_EQ(graph->getNode(4).type, domain::NodeType::Poi);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
}

TEST(GraphBuilderTest, UnknownNodeTypeDefaultsToIntersection) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,unknown_type,Test,0,0\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "10,1,1,walk,10,loop\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path).loadEdges(edges_path);
    auto graph = builder.build();

    // Неизвестный тип узла по умолчанию - Intersection
    EXPECT_EQ(graph->getNode(1).type, domain::NodeType::Intersection);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
}

TEST(GraphBuilderTest, AllTransportTypesMapped) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
        "2,intersection,B,100,0\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "10,1,2,walk,100,w\n"
        "11,1,2,car,100,c\n"
        "12,1,2,bus,100,b\n"
        "13,1,2,metro,100,m\n"
        "14,1,2,taxi,100,t\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path).loadEdges(edges_path);
    auto graph = builder.build();

    const auto& edges = graph->getEdgesFrom(1);
    ASSERT_EQ(edges.size(), 5u);
    EXPECT_EQ(edges[0].transport, domain::TransportType::Walk);
    EXPECT_EQ(edges[1].transport, domain::TransportType::Car);
    EXPECT_EQ(edges[2].transport, domain::TransportType::Bus);
    EXPECT_EQ(edges[3].transport, domain::TransportType::Metro);
    EXPECT_EQ(edges[4].transport, domain::TransportType::Taxi);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
}

TEST(GraphBuilderTest, UnknownTransportTypeDefaultsToWalk) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
        "2,intersection,B,100,0\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "10,1,2,helicopter,100,unknown\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path).loadEdges(edges_path);
    auto graph = builder.build();

    // Неизвестный тип транспорта по умолчанию - Walk
    const auto& edges = graph->getEdgesFrom(1);
    ASSERT_EQ(edges.size(), 1u);
    EXPECT_EQ(edges[0].transport, domain::TransportType::Walk);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
}

TEST(GraphBuilderTest, EdgeFieldMapping) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
        "2,intersection,B,100,0\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "42,1,2,car,150.5,\"Prospect\"\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path).loadEdges(edges_path);
    auto graph = builder.build();

    // Проверяем все поля ребра
    const auto& edges = graph->getEdgesFrom(1);
    ASSERT_EQ(edges.size(), 1u);
    EXPECT_EQ(edges[0].id, 42);
    EXPECT_EQ(edges[0].from, 1);
    EXPECT_EQ(edges[0].to, 2);
    EXPECT_EQ(edges[0].transport, domain::TransportType::Car);
    EXPECT_DOUBLE_EQ(edges[0].length_meters, 150.5);
    EXPECT_EQ(edges[0].name, "Prospect");
    ASSERT_TRUE(edges[0].logic != nullptr);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
}

// ==================== Тесты buildTransportParams ====================

TEST(GraphBuilderTest, BuildTransportParamsFromCsv) {
    auto params_path = writeTempFile("transport_params.csv",
        "transport_type,param_name,param_value\n"
        "walk,speed_kmh,6.0\n"
        "walk,fatigue_per_km,12.0\n"
        "car,speed_kmh,50.0\n"
        "car,fuel_consumption_l_per_100km,10.0\n"
        "car,fuel_price_rub_per_l,60.0\n"
        "bus,speed_kmh,30.0\n"
        "bus,ticket_price,60.0\n"
        "metro,speed_kmh,55.0\n"
        "metro,ticket_price,60.0\n"
        "metro,morning_evening_interval_min,3.0\n"
        "metro,day_interval_min,6.0\n"
        "taxi,price_per_km,25.0\n"
        "taxi,base_price,200.0\n"
    );

    GraphBuilder builder;
    builder.loadTransportParams(params_path);
    auto params = builder.buildTransportParams();

    EXPECT_DOUBLE_EQ(params.walk_speed_kmh, 6.0);
    EXPECT_DOUBLE_EQ(params.walk_fatigue_per_km, 12.0);
    EXPECT_DOUBLE_EQ(params.car_speed_kmh, 50.0);
    EXPECT_DOUBLE_EQ(params.car_fuel_l_per_100km, 10.0);
    EXPECT_DOUBLE_EQ(params.car_fuel_rub_per_l, 60.0);
    EXPECT_DOUBLE_EQ(params.bus_speed_kmh, 30.0);
    EXPECT_DOUBLE_EQ(params.bus_ticket_rub, 60.0);
    EXPECT_DOUBLE_EQ(params.metro_speed_kmh, 55.0);
    EXPECT_DOUBLE_EQ(params.metro_ticket_rub, 60.0);
    EXPECT_DOUBLE_EQ(params.metro_peak_interval_min, 3.0);
    EXPECT_DOUBLE_EQ(params.metro_offpeak_interval_min, 6.0);
    EXPECT_DOUBLE_EQ(params.taxi_price_per_km, 25.0);
    EXPECT_DOUBLE_EQ(params.taxi_base_price, 200.0);

    removeTempFile(params_path);
}

TEST(GraphBuilderTest, BuildTransportParamsUsesDefaultsForMissingParams) {
    auto params_path = writeTempFile("transport_params.csv",
        "walk,speed_kmh,7.0\n"
    );

    GraphBuilder builder;
    builder.loadTransportParams(params_path);
    auto params = builder.buildTransportParams();

    // Переопределённое значение
    EXPECT_DOUBLE_EQ(params.walk_speed_kmh, 7.0);
    // Значения по умолчанию из структуры TransportParams
    EXPECT_DOUBLE_EQ(params.walk_fatigue_per_km, 10.0);
    EXPECT_DOUBLE_EQ(params.car_speed_kmh, 40.0);
    EXPECT_DOUBLE_EQ(params.bus_speed_kmh, 25.0);
    EXPECT_DOUBLE_EQ(params.metro_speed_kmh, 50.0);
    EXPECT_DOUBLE_EQ(params.taxi_base_price, 150.0);

    removeTempFile(params_path);
}

TEST(GraphBuilderTest, BuildTransportParamsEmptyFileUsesAllDefaults) {
    auto params_path = writeTempFile("transport_params.csv", "");

    GraphBuilder builder;
    builder.loadTransportParams(params_path);
    auto params = builder.buildTransportParams();

    // Все значения должны быть по умолчанию
    domain::TransportParams defaults;
    EXPECT_DOUBLE_EQ(params.walk_speed_kmh, defaults.walk_speed_kmh);
    EXPECT_DOUBLE_EQ(params.car_speed_kmh, defaults.car_speed_kmh);
    EXPECT_DOUBLE_EQ(params.bus_speed_kmh, defaults.bus_speed_kmh);
    EXPECT_DOUBLE_EQ(params.metro_speed_kmh, defaults.metro_speed_kmh);
    EXPECT_DOUBLE_EQ(params.taxi_base_price, defaults.taxi_base_price);

    removeTempFile(params_path);
}

// ==================== Тесты валидации ====================

TEST(GraphBuilderTest, MissingNodesFileThrows) {
    GraphBuilder builder;
    EXPECT_THROW(builder.loadNodes("/tmp/nonexistent_file_xyz.csv"), std::runtime_error);
}

TEST(GraphBuilderTest, MissingEdgesFileThrows) {
    GraphBuilder builder;
    EXPECT_THROW(builder.loadEdges("/tmp/nonexistent_file_xyz.csv"), std::runtime_error);
}

TEST(GraphBuilderTest, MissingTransportParamsFileThrows) {
    GraphBuilder builder;
    EXPECT_THROW(builder.loadTransportParams("/tmp/nonexistent_file_xyz.csv"), std::runtime_error);
}

TEST(GraphBuilderTest, MissingSchedulesFileThrows) {
    GraphBuilder builder;
    EXPECT_THROW(builder.loadSchedules("/tmp/nonexistent_file_xyz.csv"), std::runtime_error);
}

TEST(GraphBuilderTest, BuildWithoutNodesThrows) {
    auto edges_path = writeTempFile("edges.csv",
        "10,1,2,walk,100,w\n"
    );

    GraphBuilder builder;
    builder.loadEdges(edges_path);
    EXPECT_THROW(builder.build(), std::runtime_error);

    removeTempFile(edges_path);
}

TEST(GraphBuilderTest, BuildWithoutEdgesThrows) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path);
    EXPECT_THROW(builder.build(), std::runtime_error);

    removeTempFile(nodes_path);
}

TEST(GraphBuilderTest, EdgeReferencesNonExistentNodeThrows) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "10,1,999,walk,100,w\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path).loadEdges(edges_path);
    EXPECT_THROW(builder.build(), std::runtime_error);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
}

TEST(GraphBuilderTest, ScheduleReferencesNonExistentEdgeThrows) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
        "2,intersection,B,100,0\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "10,1,2,walk,100,w\n"
    );
    auto schedules_path = writeTempFile("schedules.csv",
        "999,fixed_times,\"08:00,08:15\"\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path).loadEdges(edges_path).loadSchedules(schedules_path);
    EXPECT_THROW(builder.build(), std::runtime_error);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
    removeTempFile(schedules_path);
}

TEST(GraphBuilderTest, DuplicateNodeIdThrows) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
        "1,bus_stop,Duplicate,100,0\n"
    );

    GraphBuilder builder;
    EXPECT_THROW(builder.loadNodes(nodes_path), std::runtime_error);

    removeTempFile(nodes_path);
}

TEST(GraphBuilderTest, DuplicateEdgeIdThrows) {
    auto edges_path = writeTempFile("edges.csv",
        "10,1,2,walk,100,w1\n"
        "10,2,3,walk,200,w2\n"
    );

    GraphBuilder builder;
    EXPECT_THROW(builder.loadEdges(edges_path), std::runtime_error);

    removeTempFile(edges_path);
}

// ==================== Тесты доступности ====================

TEST(GraphBuilderTest, GetNodesReturnsLoadedData) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
        "2,bus_stop,B,100,200\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path);

    const auto& nodes = builder.getNodes();
    ASSERT_EQ(nodes.size(), 2u);
    EXPECT_EQ(nodes[0].id, 1);
    EXPECT_EQ(nodes[1].id, 2);

    removeTempFile(nodes_path);
}

TEST(GraphBuilderTest, GetEdgesReturnsLoadedData) {
    auto edges_path = writeTempFile("edges.csv",
        "10,1,2,walk,100,Road\n"
    );

    GraphBuilder builder;
    builder.loadEdges(edges_path);

    const auto& edges = builder.getEdges();
    ASSERT_EQ(edges.size(), 1u);
    EXPECT_EQ(edges[0].id, 10);
    EXPECT_EQ(edges[0].transport_type, "walk");

    removeTempFile(edges_path);
}

TEST(GraphBuilderTest, GetSchedulesReturnsLoadedData) {
    auto schedules_path = writeTempFile("schedules.csv",
        "3001,fixed_times,\"08:00,08:15\"\n"
        "5001,time_window,\"01:00-05:00\"\n"
    );

    GraphBuilder builder;
    builder.loadSchedules(schedules_path);

    const auto& schedules = builder.getSchedules();
    ASSERT_EQ(schedules.size(), 2u);
    EXPECT_TRUE(schedules.count(3001) > 0);
    EXPECT_TRUE(schedules.count(5001) > 0);
    EXPECT_EQ(schedules.at(3001).schedule_type, "fixed_times");
    EXPECT_EQ(schedules.at(5001).schedule_type, "time_window");

    removeTempFile(schedules_path);
}

// ==================== Тест цепочечек ====================

TEST(GraphBuilderTest, ChainedLoadCalls) {
    auto nodes_path = writeTempFile("nodes.csv",
        "1,intersection,A,0,0\n"
        "2,intersection,B,100,0\n"
    );
    auto edges_path = writeTempFile("edges.csv",
        "10,1,2,walk,100,Road\n"
    );
    auto params_path = writeTempFile("transport_params.csv",
        "walk,speed_kmh,5.0\n"
    );
    auto schedules_path = writeTempFile("schedules.csv",
        "10,fixed_times,\"08:00\"\n"
    );

    GraphBuilder builder;
    builder.loadNodes(nodes_path)
           .loadEdges(edges_path)
           .loadTransportParams(params_path)
           .loadSchedules(schedules_path);

    auto graph = builder.build();
    ASSERT_TRUE(graph != nullptr);
    EXPECT_EQ(graph->nodeCount(), 2u);

    auto params = builder.buildTransportParams();
    EXPECT_DOUBLE_EQ(params.walk_speed_kmh, 5.0);

    removeTempFile(nodes_path);
    removeTempFile(edges_path);
    removeTempFile(params_path);
    removeTempFile(schedules_path);
}

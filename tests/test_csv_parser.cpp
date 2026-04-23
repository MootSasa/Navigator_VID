#include <gtest/gtest.h>

#include <sstream>

#include "infrastructure/csv_parser.h"

using namespace infrastructure;

// Тесты базового метода parse()

TEST(CsvParserParseTest, SimpleTable) {
  CsvParser parser;
  std::istringstream input(
      "a,b,c\n"
      "1,2,3\n");
  auto table = parser.parse(input);

  ASSERT_EQ(table.size(), 2u);
  ASSERT_EQ(table[0].size(), 3u);
  EXPECT_EQ(table[0][0], "a");
  EXPECT_EQ(table[0][1], "b");
  EXPECT_EQ(table[0][2], "c");
  ASSERT_EQ(table[1].size(), 3u);
  EXPECT_EQ(table[1][0], "1");
  EXPECT_EQ(table[1][1], "2");
  EXPECT_EQ(table[1][2], "3");
}

TEST(CsvParserParseTest, SkipEmptyLines) {
  CsvParser parser;
  std::istringstream input(
      "a,b\n"
      "\n"
      "c,d\n"
      "\n");
  auto table = parser.parse(input);

  ASSERT_EQ(table.size(), 2u);
  EXPECT_EQ(table[0][0], "a");
  EXPECT_EQ(table[1][0], "c");
}

TEST(CsvParserParseTest, SkipComments) {
  CsvParser parser;
  std::istringstream input(
      "# Это комментарий\n"
      "a,b\n"
      "# Ещё комментарий\n"
      "c,d\n");
  auto table = parser.parse(input);

  ASSERT_EQ(table.size(), 2u);
  EXPECT_EQ(table[0][0], "a");
  EXPECT_EQ(table[1][0], "c");
}

TEST(CsvParserParseTest, QuotedFieldWithComma) {
  CsvParser parser;
  std::istringstream input("1,\"hello, world\",3\n");
  auto table = parser.parse(input);

  ASSERT_EQ(table.size(), 1u);
  ASSERT_EQ(table[0].size(), 3u);
  EXPECT_EQ(table[0][0], "1");
  EXPECT_EQ(table[0][1], "hello, world");
  EXPECT_EQ(table[0][2], "3");
}

TEST(CsvParserParseTest, QuotedFieldWithSpaces) {
  CsvParser parser;
  std::istringstream input("  \"hello world\" , value  \n");
  auto table = parser.parse(input);

  ASSERT_EQ(table.size(), 1u);
  ASSERT_EQ(table[0].size(), 2u);
  EXPECT_EQ(table[0][0], "hello world");
  EXPECT_EQ(table[0][1], "value");
}

TEST(CsvParserParseTest, EmptyInput) {
  CsvParser parser;
  std::istringstream input("");
  auto table = parser.parse(input);

  EXPECT_TRUE(table.empty());
}

TEST(CsvParserParseTest, OnlyCommentsAndEmptyLines) {
  CsvParser parser;
  std::istringstream input(
      "# comment\n"
      "\n"
      "# another\n");
  auto table = parser.parse(input);

  EXPECT_TRUE(table.empty());
}

TEST(CsvParserParseTest, SingleColumn) {
  CsvParser parser;
  std::istringstream input("only_one\n");
  auto table = parser.parse(input);

  ASSERT_EQ(table.size(), 1u);
  ASSERT_EQ(table[0].size(), 1u);
  EXPECT_EQ(table[0][0], "only_one");
}

TEST(CsvParserParseTest, WhitespaceTrimmed) {
  CsvParser parser;
  std::istringstream input("  alpha  ,  beta  ,  gamma  \n");
  auto table = parser.parse(input);

  ASSERT_EQ(table.size(), 1u);
  ASSERT_EQ(table[0].size(), 3u);
  EXPECT_EQ(table[0][0], "alpha");
  EXPECT_EQ(table[0][1], "beta");
  EXPECT_EQ(table[0][2], "gamma");
}

// Тесты parseNodes()

TEST(CsvParserNodesTest, BasicNodes) {
  CsvParser parser;
  std::istringstream input(
      "node_id,type,name,x,y\n"
      "101,intersection,\"\",0,0\n"
      "102,bus_stop,\"Ост. 'Университет'\",1200,-450\n"
      "103,metro_station,\"Ст. м. 'Университет'\",1210,-440\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 3u);

  EXPECT_EQ(nodes[0].id, 101);
  EXPECT_EQ(nodes[0].type, "intersection");
  EXPECT_EQ(nodes[0].name, "");
  EXPECT_DOUBLE_EQ(nodes[0].x, 0.0);
  EXPECT_DOUBLE_EQ(nodes[0].y, 0.0);

  EXPECT_EQ(nodes[1].id, 102);
  EXPECT_EQ(nodes[1].type, "bus_stop");
  EXPECT_EQ(nodes[1].name, "Ост. 'Университет'");
  EXPECT_DOUBLE_EQ(nodes[1].x, 1200.0);
  EXPECT_DOUBLE_EQ(nodes[1].y, -450.0);

  EXPECT_EQ(nodes[2].id, 103);
  EXPECT_EQ(nodes[2].type, "metro_station");
  EXPECT_EQ(nodes[2].name, "Ст. м. 'Университет'");
  EXPECT_DOUBLE_EQ(nodes[2].x, 1210.0);
  EXPECT_DOUBLE_EQ(nodes[2].y, -440.0);
}

TEST(CsvParserNodesTest, SkipHeaderOnlyOnce) {
  CsvParser parser;
  std::istringstream input(
      "node_id,type,name,x,y\n"
      "1,intersection,A,10,20\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_EQ(nodes[0].id, 1);
}

TEST(CsvParserNodesTest, NoHeaderParsedAsData) {
  CsvParser parser;
  std::istringstream input(
      "1,intersection,A,10,20\n"
      "2,bus_stop,B,30,40\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 2u);
  EXPECT_EQ(nodes[0].id, 1);
  EXPECT_EQ(nodes[1].id, 2);
}

TEST(CsvParserNodesTest, SkipMalformedRows) {
  CsvParser parser;
  std::istringstream input(
      "node_id,type,name,x,y\n"
      "1,intersection,A,10,20\n"
      "BAD_ROW\n"
      "2,bus_stop,B,30,40\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 2u);
  EXPECT_EQ(nodes[0].id, 1);
  EXPECT_EQ(nodes[1].id, 2);
}

TEST(CsvParserNodesTest, SkipCommentsInNodes) {
  CsvParser parser;
  std::istringstream input(
      "# Описание формата\n"
      "node_id,type,name,x,y\n"
      "# Важный узел\n"
      "1,intersection,Center,0,0\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_EQ(nodes[0].id, 1);
}

TEST(CsvParserNodesTest, SkipEmptyLinesInNodes) {
  CsvParser parser;
  std::istringstream input(
      "node_id,type,name,x,y\n"
      "\n"
      "1,intersection,A,10,20\n"
      "\n"
      "2,bus_stop,B,30,40\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 2u);
}

TEST(CsvParserNodesTest, NegativeCoordinates) {
  CsvParser parser;
  std::istringstream input("1,poi,Test,-1250.5,-430.75\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_DOUBLE_EQ(nodes[0].x, -1250.5);
  EXPECT_DOUBLE_EQ(nodes[0].y, -430.75);
}

TEST(CsvParserNodesTest, ZeroCoordinates) {
  CsvParser parser;
  std::istringstream input("1,intersection,Origin,0,0\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_DOUBLE_EQ(nodes[0].x, 0.0);
  EXPECT_DOUBLE_EQ(nodes[0].y, 0.0);
}

TEST(CsvParserNodesTest, PoiType) {
  CsvParser parser;
  std::istringstream input("104,poi,\"Главное Здание МГУ\",1260,-420\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_EQ(nodes[0].type, "poi");
  EXPECT_EQ(nodes[0].name, "Главное Здание МГУ");
}

TEST(CsvParserNodesTest, EmptyInputNodes) {
  CsvParser parser;
  std::istringstream input("");
  auto nodes = parser.parseNodes(input);

  EXPECT_TRUE(nodes.empty());
}

TEST(CsvParserNodesTest, OnlyHeaderNoData) {
  CsvParser parser;
  std::istringstream input("node_id,type,name,x,y\n");
  auto nodes = parser.parseNodes(input);

  EXPECT_TRUE(nodes.empty());
}

// Тесты parseEdges()

TEST(CsvParserEdgesTest, BasicEdges) {
  CsvParser parser;
  std::istringstream input(
      "edge_id,from_node,to_node,transport_type,length_meters,name\n"
      "2001,102,104,walk,100,\"Пешком до МГУ\"\n"
      "3001,101,102,bus,2500,\"Авт. 113\"\n"
      "4001,101,104,car,3000,\"Проспект Вернадского\"\n");
  auto edges = parser.parseEdges(input);

  ASSERT_EQ(edges.size(), 3u);

  EXPECT_EQ(edges[0].id, 2001);
  EXPECT_EQ(edges[0].from_node, 102);
  EXPECT_EQ(edges[0].to_node, 104);
  EXPECT_EQ(edges[0].transport_type, "walk");
  EXPECT_DOUBLE_EQ(edges[0].length_meters, 100.0);
  EXPECT_EQ(edges[0].name, "Пешком до МГУ");

  EXPECT_EQ(edges[1].id, 3001);
  EXPECT_EQ(edges[1].transport_type, "bus");
  EXPECT_DOUBLE_EQ(edges[1].length_meters, 2500.0);
  EXPECT_EQ(edges[1].name, "Авт. 113");

  EXPECT_EQ(edges[2].id, 4001);
  EXPECT_EQ(edges[2].transport_type, "car");
  EXPECT_DOUBLE_EQ(edges[2].length_meters, 3000.0);
  EXPECT_EQ(edges[2].name, "Проспект Вернадского");
}

TEST(CsvParserEdgesTest, AllTransportTypes) {
  CsvParser parser;
  std::istringstream input(
      "1,1,2,walk,100,Пешком\n"
      "2,1,2,car,100,Авто\n"
      "3,1,2,bus,100,Автобус\n"
      "4,1,2,metro,100,Метро\n"
      "5,1,2,taxi,100,Такси\n");
  auto edges = parser.parseEdges(input);

  ASSERT_EQ(edges.size(), 5u);
  EXPECT_EQ(edges[0].transport_type, "walk");
  EXPECT_EQ(edges[1].transport_type, "car");
  EXPECT_EQ(edges[2].transport_type, "bus");
  EXPECT_EQ(edges[3].transport_type, "metro");
  EXPECT_EQ(edges[4].transport_type, "taxi");
}

TEST(CsvParserEdgesTest, SkipHeader) {
  CsvParser parser;
  std::istringstream input(
      "edge_id,from_node,to_node,transport_type,length_meters,name\n"
      "1,1,2,walk,100,test\n");
  auto edges = parser.parseEdges(input);

  ASSERT_EQ(edges.size(), 1u);
  EXPECT_EQ(edges[0].id, 1);
}

TEST(CsvParserEdgesTest, SkipMalformedRows) {
  CsvParser parser;
  std::istringstream input(
      "1,1,2,walk,100,test\n"
      "BAD\n"
      "2,3,4,bus,200,bus_route\n");
  auto edges = parser.parseEdges(input);

  ASSERT_EQ(edges.size(), 2u);
  EXPECT_EQ(edges[0].id, 1);
  EXPECT_EQ(edges[1].id, 2);
}

TEST(CsvParserEdgesTest, SkipCommentsInEdges) {
  CsvParser parser;
  std::istringstream input(
      "# Рёбра графа\n"
      "1,1,2,walk,100,Пешком\n"
      "# Ещё ребро\n"
      "2,3,4,car,200,Авто\n");
  auto edges = parser.parseEdges(input);

  ASSERT_EQ(edges.size(), 2u);
}

TEST(CsvParserEdgesTest, FractionalLength) {
  CsvParser parser;
  std::istringstream input("1,1,2,walk,150.5,Пешком\n");
  auto edges = parser.parseEdges(input);

  ASSERT_EQ(edges.size(), 1u);
  EXPECT_DOUBLE_EQ(edges[0].length_meters, 150.5);
}

TEST(CsvParserEdgesTest, EmptyInputEdges) {
  CsvParser parser;
  std::istringstream input("");
  auto edges = parser.parseEdges(input);

  EXPECT_TRUE(edges.empty());
}

// Тесты parseTransportParams()

TEST(CsvParserTransportParamsTest, BasicParams) {
  CsvParser parser;
  std::istringstream input(
      "transport_type,param_name,param_value\n"
      "walk,speed_kmh,5.0\n"
      "walk,fatigue_per_km,10.0\n"
      "bus,speed_kmh,25.0\n"
      "bus,ticket_price,55.0\n");
  auto params = parser.parseTransportParams(input);

  ASSERT_EQ(params.size(), 4u);

  EXPECT_EQ(params[0].transport_type, "walk");
  EXPECT_EQ(params[0].param_name, "speed_kmh");
  EXPECT_EQ(params[0].param_value, "5.0");

  EXPECT_EQ(params[1].transport_type, "walk");
  EXPECT_EQ(params[1].param_name, "fatigue_per_km");
  EXPECT_EQ(params[1].param_value, "10.0");

  EXPECT_EQ(params[2].transport_type, "bus");
  EXPECT_EQ(params[2].param_name, "speed_kmh");
  EXPECT_EQ(params[2].param_value, "25.0");

  EXPECT_EQ(params[3].transport_type, "bus");
  EXPECT_EQ(params[3].param_name, "ticket_price");
  EXPECT_EQ(params[3].param_value, "55.0");
}

TEST(CsvParserTransportParamsTest, AllTransportTypesParams) {
  CsvParser parser;
  std::istringstream input(
      "walk,speed_kmh,5.0\n"
      "car,speed_kmh,40.0\n"
      "car,fuel_consumption_l_per_100km,8.0\n"
      "car,fuel_price_rub_per_l,50.0\n"
      "bus,speed_kmh,25.0\n"
      "bus,ticket_price,55.0\n"
      "metro,speed_kmh,50.0\n"
      "metro,ticket_price,55.0\n"
      "metro,morning_evening_interval_min,2.0\n"
      "metro,day_interval_min,5.0\n"
      "taxi,price_per_km,20.0\n"
      "taxi,base_price,150.0\n");
  auto params = parser.parseTransportParams(input);

  EXPECT_EQ(params.size(), 12u);
}

TEST(CsvParserTransportParamsTest, SkipHeader) {
  CsvParser parser;
  std::istringstream input(
      "transport_type,param_name,param_value\n"
      "walk,speed_kmh,5.0\n");
  auto params = parser.parseTransportParams(input);

  ASSERT_EQ(params.size(), 1u);
  EXPECT_EQ(params[0].transport_type, "walk");
}

TEST(CsvParserTransportParamsTest, SkipMalformedRows) {
  CsvParser parser;
  std::istringstream input(
      "walk,speed_kmh,5.0\n"
      "BAD\n"
      "bus,ticket_price,55.0\n");
  auto params = parser.parseTransportParams(input);

  ASSERT_EQ(params.size(), 2u);
}

TEST(CsvParserTransportParamsTest, SkipCommentsInParams) {
  CsvParser parser;
  std::istringstream input(
      "# Параметры транспорта\n"
      "walk,speed_kmh,5.0\n"
      "# Параметры автобуса\n"
      "bus,speed_kmh,25.0\n");
  auto params = parser.parseTransportParams(input);

  ASSERT_EQ(params.size(), 2u);
}

TEST(CsvParserTransportParamsTest, ParamValuePreservedAsString) {
  CsvParser parser;
  std::istringstream input("metro,morning_evening_interval_min,2.0\n");
  auto params = parser.parseTransportParams(input);

  ASSERT_EQ(params.size(), 1u);
  EXPECT_EQ(params[0].param_value, "2.0");
}

TEST(CsvParserTransportParamsTest, EmptyInputParams) {
  CsvParser parser;
  std::istringstream input("");
  auto params = parser.parseTransportParams(input);

  EXPECT_TRUE(params.empty());
}

// Тесты parseSchedules()

TEST(CsvParserSchedulesTest, FixedTimesSchedule) {
  CsvParser parser;
  std::istringstream input(
      "edge_id,schedule_type,params\n"
      "3001,fixed_times,\"08:00,08:15,08:30,08:45,09:00\"\n");
  auto schedules = parser.parseSchedules(input);

  ASSERT_EQ(schedules.size(), 1u);
  EXPECT_EQ(schedules[0].edge_id, 3001);
  EXPECT_EQ(schedules[0].schedule_type, "fixed_times");
  EXPECT_EQ(schedules[0].params, "08:00,08:15,08:30,08:45,09:00");
}

TEST(CsvParserSchedulesTest, TimeWindowSchedule) {
  CsvParser parser;
  std::istringstream input(
      "edge_id,schedule_type,params\n"
      "5001,time_window,\"01:00-05:00\"\n");
  auto schedules = parser.parseSchedules(input);

  ASSERT_EQ(schedules.size(), 1u);
  EXPECT_EQ(schedules[0].edge_id, 5001);
  EXPECT_EQ(schedules[0].schedule_type, "time_window");
  EXPECT_EQ(schedules[0].params, "01:00-05:00");
}

TEST(CsvParserSchedulesTest, MixedScheduleTypes) {
  CsvParser parser;
  std::istringstream input(
      "edge_id,schedule_type,params\n"
      "3001,fixed_times,\"08:00,08:15,08:30\"\n"
      "5001,time_window,\"01:00-05:00\"\n"
      "3002,fixed_times,\"09:00,09:30,10:00\"\n");
  auto schedules = parser.parseSchedules(input);

  ASSERT_EQ(schedules.size(), 3u);
  EXPECT_EQ(schedules[0].schedule_type, "fixed_times");
  EXPECT_EQ(schedules[1].schedule_type, "time_window");
  EXPECT_EQ(schedules[2].schedule_type, "fixed_times");
}

TEST(CsvParserSchedulesTest, SkipHeader) {
  CsvParser parser;
  std::istringstream input(
      "edge_id,schedule_type,params\n"
      "3001,fixed_times,\"08:00,08:15\"\n");
  auto schedules = parser.parseSchedules(input);

  ASSERT_EQ(schedules.size(), 1u);
  EXPECT_EQ(schedules[0].edge_id, 3001);
}

TEST(CsvParserSchedulesTest, SkipCommentsInSchedules) {
  CsvParser parser;
  std::istringstream input(
      "# Расписание автобусов\n"
      "3001,fixed_times,\"08:00,08:15\"\n"
      "# Разводной мост\n"
      "5001,time_window,\"01:00-05:00\"\n");
  auto schedules = parser.parseSchedules(input);

  ASSERT_EQ(schedules.size(), 2u);
}

TEST(CsvParserSchedulesTest, SkipMalformedRows) {
  CsvParser parser;
  std::istringstream input(
      "3001,fixed_times,\"08:00,08:15\"\n"
      "BAD\n"
      "5001,time_window,\"01:00-05:00\"\n");
  auto schedules = parser.parseSchedules(input);

  ASSERT_EQ(schedules.size(), 2u);
}

TEST(CsvParserSchedulesTest, EmptyInputSchedules) {
  CsvParser parser;
  std::istringstream input("");
  auto schedules = parser.parseSchedules(input);

  EXPECT_TRUE(schedules.empty());
}

TEST(CsvParserSchedulesTest, QuotedParamsWithCommas) {
  CsvParser parser;
  std::istringstream input(
      "3001,fixed_times,\"08:00,08:15,08:30,08:45,09:00\"\n");
  auto schedules = parser.parseSchedules(input);

  ASSERT_EQ(schedules.size(), 1u);
  // params должен содержать все времена через запятую, а не только "08:00"
  EXPECT_EQ(schedules[0].params, "08:00,08:15,08:30,08:45,09:00");
}

// Интеграционные тесты

TEST(CsvParserIntegrationTest, FullNodesFromSpec) {
  CsvParser parser;
  std::istringstream input(
      "node_id,type,name,x,y\n"
      "101,intersection,\"\",0,0\n"
      "102,bus_stop,\"Ост. 'Университет'\",1200,-450\n"
      "103,metro_station,\"Ст. м. 'Университет'\",1210,-440\n"
      "104,poi,\"Главное Здание МГУ\",1260,-420\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 4u);

  EXPECT_EQ(nodes[0].id, 101);
  EXPECT_EQ(nodes[0].type, "intersection");
  EXPECT_EQ(nodes[0].name, "");
  EXPECT_DOUBLE_EQ(nodes[0].x, 0.0);
  EXPECT_DOUBLE_EQ(nodes[0].y, 0.0);

  EXPECT_EQ(nodes[1].id, 102);
  EXPECT_EQ(nodes[1].type, "bus_stop");

  EXPECT_EQ(nodes[2].id, 103);
  EXPECT_EQ(nodes[2].type, "metro_station");

  EXPECT_EQ(nodes[3].id, 104);
  EXPECT_EQ(nodes[3].type, "poi");
  EXPECT_EQ(nodes[3].name, "Главное Здание МГУ");
}

TEST(CsvParserIntegrationTest, FullEdgesFromSpec) {
  CsvParser parser;
  std::istringstream input(
      "edge_id,from_node,to_node,transport_type,length_meters,name\n"
      "2001,102,104,walk,100,\"Пешком до МГУ\"\n"
      "2002,104,102,walk,100,\"Пешком от МГУ\"\n"
      "2003,102,103,walk,50,\"Пересадка на метро\"\n"
      "3001,101,102,bus,2500,\"Авт. 113\"\n"
      "4001,101,104,car,3000,\"Проспект Вернадского\"\n");
  auto edges = parser.parseEdges(input);

  ASSERT_EQ(edges.size(), 5u);

  EXPECT_EQ(edges[0].id, 2001);
  EXPECT_EQ(edges[0].from_node, 102);
  EXPECT_EQ(edges[0].to_node, 104);
  EXPECT_EQ(edges[0].transport_type, "walk");
  EXPECT_DOUBLE_EQ(edges[0].length_meters, 100.0);

  EXPECT_EQ(edges[2].id, 2003);
  EXPECT_EQ(edges[2].name, "Пересадка на метро");
  EXPECT_DOUBLE_EQ(edges[2].length_meters, 50.0);

  EXPECT_EQ(edges[3].id, 3001);
  EXPECT_EQ(edges[3].transport_type, "bus");
  EXPECT_DOUBLE_EQ(edges[3].length_meters, 2500.0);

  EXPECT_EQ(edges[4].id, 4001);
  EXPECT_EQ(edges[4].transport_type, "car");
}

TEST(CsvParserIntegrationTest, FullTransportParamsFromSpec) {
  CsvParser parser;
  std::istringstream input(
      "transport_type,param_name,param_value\n"
      "walk,speed_kmh,5.0\n"
      "walk,fatigue_per_km,10.0\n"
      "car,speed_kmh,40.0\n"
      "car,fuel_consumption_l_per_100km,8.0\n"
      "car,fuel_price_rub_per_l,50.0\n"
      "bus,speed_kmh,25.0\n"
      "bus,ticket_price,55.0\n"
      "metro,speed_kmh,50.0\n"
      "metro,ticket_price,55.0\n"
      "metro,morning_evening_interval_min,2.0\n"
      "metro,day_interval_min,5.0\n"
      "taxi,price_per_km,20.0\n"
      "taxi,base_price,150.0\n");
  auto params = parser.parseTransportParams(input);

  ASSERT_EQ(params.size(), 13u);

  EXPECT_EQ(params[0].transport_type, "walk");
  EXPECT_EQ(params[0].param_name, "speed_kmh");
  EXPECT_EQ(params[0].param_value, "5.0");

  EXPECT_EQ(params[4].transport_type, "car");
  EXPECT_EQ(params[4].param_name, "fuel_price_rub_per_l");
  EXPECT_EQ(params[4].param_value, "50.0");

  EXPECT_EQ(params[12].transport_type, "taxi");
  EXPECT_EQ(params[12].param_name, "base_price");
  EXPECT_EQ(params[12].param_value, "150.0");
}

TEST(CsvParserIntegrationTest, FullSchedulesFromSpec) {
  CsvParser parser;
  std::istringstream input(
      "edge_id,schedule_type,params\n"
      "# Расписание для автобусного маршрута с edge_id = 3001\n"
      "3001,fixed_times,\"08:00,08:15,08:30,08:45,09:00\"\n"
      "# Разводной мост (ребро с id = 5001): недоступен с 01:00 до 05:00\n"
      "5001,time_window,\"01:00-05:00\"\n");
  auto schedules = parser.parseSchedules(input);

  ASSERT_EQ(schedules.size(), 2u);

  EXPECT_EQ(schedules[0].edge_id, 3001);
  EXPECT_EQ(schedules[0].schedule_type, "fixed_times");
  EXPECT_EQ(schedules[0].params, "08:00,08:15,08:30,08:45,09:00");

  EXPECT_EQ(schedules[1].edge_id, 5001);
  EXPECT_EQ(schedules[1].schedule_type, "time_window");
  EXPECT_EQ(schedules[1].params, "01:00-05:00");
}

// Граничные случаи

TEST(CsvParserEdgeCaseTest, MultipleEmptyAndCommentLines) {
  CsvParser parser;
  std::istringstream input(
      "\n"
      "# comment\n"
      "\n"
      "# another\n"
      "\n"
      "1,intersection,A,10,20\n"
      "\n"
      "# mid comment\n"
      "2,bus_stop,B,30,40\n"
      "\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 2u);
  EXPECT_EQ(nodes[0].id, 1);
  EXPECT_EQ(nodes[1].id, 2);
}

TEST(CsvParserEdgeCaseTest, LargeIdValues) {
  CsvParser parser;
  std::istringstream input("999999,intersection,LargeId,100,200\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_EQ(nodes[0].id, 999999);
}

TEST(CsvParserEdgeCaseTest, VerySmallDoubleValue) {
  CsvParser parser;
  std::istringstream input("1,intersection,Tiny,0.001,-0.001\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_NEAR(nodes[0].x, 0.001, 1e-9);
  EXPECT_NEAR(nodes[0].y, -0.001, 1e-9);
}

TEST(CsvParserEdgeCaseTest, ExtraColumnsIgnored) {
  CsvParser parser;
  std::istringstream input("1,intersection,A,10,20,extra1,extra2\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_EQ(nodes[0].id, 1);
  EXPECT_DOUBLE_EQ(nodes[0].x, 10.0);
  EXPECT_DOUBLE_EQ(nodes[0].y, 20.0);
}

TEST(CsvParserEdgeCaseTest, ExactlyMinimumColumns) {
  CsvParser parser;
  std::istringstream input("1,intersection,A,10,20\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_EQ(nodes[0].id, 1);
}

TEST(CsvParserEdgeCaseTest, OneColumnShortForNodes) {
  CsvParser parser;
  std::istringstream input("1,intersection,A,10\n");
  auto nodes = parser.parseNodes(input);

  EXPECT_TRUE(nodes.empty());
}

TEST(CsvParserEdgeCaseTest, OneColumnShortForEdges) {
  CsvParser parser;
  std::istringstream input("1,1,2,walk,100\n");
  auto edges = parser.parseEdges(input);

  EXPECT_TRUE(edges.empty());
}

TEST(CsvParserEdgeCaseTest, OneColumnShortForParams) {
  CsvParser parser;
  std::istringstream input("walk,speed_kmh\n");
  auto params = parser.parseTransportParams(input);

  EXPECT_TRUE(params.empty());
}

TEST(CsvParserEdgeCaseTest, OneColumnShortForSchedules) {
  CsvParser parser;
  std::istringstream input("3001,fixed_times\n");
  auto schedules = parser.parseSchedules(input);

  EXPECT_TRUE(schedules.empty());
}

TEST(CsvParserEdgeCaseTest, NonNumericIdReturnsZero) {
  CsvParser parser;
  std::istringstream input("abc,intersection,A,10,20\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_EQ(nodes[0].id, 0);
}

TEST(CsvParserEdgeCaseTest, NonNumericDoubleReturnsZero) {
  CsvParser parser;
  std::istringstream input("1,intersection,A,xyz,abc\n");
  auto nodes = parser.parseNodes(input);

  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_DOUBLE_EQ(nodes[0].x, 0.0);
  EXPECT_DOUBLE_EQ(nodes[0].y, 0.0);
}

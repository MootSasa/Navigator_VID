#include "infrastructure/graph_builder.h"

#include "domain/edge.h"
#include "domain/graph.h"
#include "domain/node.h"
#include "domain/time_variants.h"
#include "domain/types.h"

#include <fstream>
#include <stdexcept>
#include <sstream>

namespace infrastructure {

GraphBuilder& GraphBuilder::loadNodes(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open nodes file: " + filepath);
    }

    nodes_ = parser_.parseNodes(file);

    // Заполняем множество ID узлов для валидации
    node_ids_.clear();
    for (const auto& node : nodes_) {
        if (node_ids_.count(node.id) > 0) {
            throw std::runtime_error("Duplicate node ID: " + std::to_string(node.id));
        }
        node_ids_.insert(node.id);
    }

    return *this;
}

GraphBuilder& GraphBuilder::loadEdges(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open edges file: " + filepath);
    }

    edges_ = parser_.parseEdges(file);

    // Заполняем множество ID рёбер для валидации
    edge_ids_.clear();
    for (const auto& edge : edges_) {
        if (edge_ids_.count(edge.id) > 0) {
            throw std::runtime_error("Duplicate edge ID: " + std::to_string(edge.id));
        }
        edge_ids_.insert(edge.id);
    }

    return *this;
}

GraphBuilder& GraphBuilder::loadTransportParams(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open transport params file: " + filepath);
    }

    auto params = parser_.parseTransportParams(file);

    // Организуем параметры в иерархическую структуру
    transport_params_.clear();
    for (const auto& param : params) {
        transport_params_[param.transport_type][param.param_name] = param.param_value;
    }

    return *this;
}

GraphBuilder& GraphBuilder::loadSchedules(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open schedules file: " + filepath);
    }

    auto schedules = parser_.parseSchedules(file);

    // Организуем расписания по edge_id
    schedules_.clear();
    for (const auto& schedule : schedules) {
        schedules_[schedule.edge_id] = schedule;
    }

    return *this;
}

// ==================== Валидация ====================

void GraphBuilder::validateData() const {
    // Проверяем, что загружены узлы
    if (nodes_.empty()) {
        throw std::runtime_error("No nodes loaded. Call loadNodes() first.");
    }

    // Проверяем, что загружены рёбра
    if (edges_.empty()) {
        throw std::runtime_error("No edges loaded. Call loadEdges() first.");
    }

    // Проверяем ссылки рёбер на узлы
    validateEdgeReferences();

    // Проверяем ссылки расписаний на рёбра
    validateScheduleReferences();
}

void GraphBuilder::validateEdgeReferences() const {
    for (const auto& edge : edges_) {
        if (node_ids_.find(edge.from_node) == node_ids_.end()) {
            throw std::runtime_error(
                "Edge " + std::to_string(edge.id) +
                " references non-existent from_node: " + std::to_string(edge.from_node)
            );
        }
        if (node_ids_.find(edge.to_node) == node_ids_.end()) {
            throw std::runtime_error(
                "Edge " + std::to_string(edge.id) +
                " references non-existent to_node: " + std::to_string(edge.to_node)
            );
        }
    }
}

void GraphBuilder::validateScheduleReferences() const {
    for (const auto& [edge_id, schedule] : schedules_) {
        if (edge_ids_.find(edge_id) == edge_ids_.end()) {
            throw std::runtime_error(
                "Schedule references non-existent edge ID: " + std::to_string(edge_id)
            );
        }
    }
}

// ==================== Вспомогательные методы ====================

double GraphBuilder::getParam(const std::string& type, const std::string& name, double default_value) const {
    auto type_it = transport_params_.find(type);
    if (type_it == transport_params_.end()) {
        return default_value;
    }
    auto param_it = type_it->second.find(name);
    if (param_it == type_it->second.end()) {
        return default_value;
    }
    try {
        return std::stod(param_it->second);
    } catch (const std::exception&) {
        return default_value;
    }
}

domain::TransportParams GraphBuilder::buildTransportParams() const {
    domain::TransportParams params;

    params.walk_speed_kmh        = getParam("walk", "speed_kmh", params.walk_speed_kmh);
    params.walk_fatigue_per_km   = getParam("walk", "fatigue_per_km", params.walk_fatigue_per_km);

    params.car_speed_kmh         = getParam("car", "speed_kmh", params.car_speed_kmh);
    params.car_fuel_l_per_100km  = getParam("car", "fuel_consumption_l_per_100km", params.car_fuel_l_per_100km);
    params.car_fuel_rub_per_l    = getParam("car", "fuel_price_rub_per_l", params.car_fuel_rub_per_l);

    params.taxi_speed_kmh        = getParam("taxi", "speed_kmh", params.taxi_speed_kmh);
    params.taxi_price_per_km     = getParam("taxi", "price_per_km", params.taxi_price_per_km);
    params.taxi_base_price       = getParam("taxi", "base_price", params.taxi_base_price);

    params.bus_speed_kmh         = getParam("bus", "speed_kmh", params.bus_speed_kmh);
    params.bus_ticket_rub        = getParam("bus", "ticket_price", params.bus_ticket_rub);

    params.metro_speed_kmh              = getParam("metro", "speed_kmh", params.metro_speed_kmh);
    params.metro_ticket_rub             = getParam("metro", "ticket_price", params.metro_ticket_rub);
    params.metro_peak_interval_min      = getParam("metro", "morning_evening_interval_min", params.metro_peak_interval_min);
    params.metro_offpeak_interval_min   = getParam("metro", "day_interval_min", params.metro_offpeak_interval_min);

    return params;
}

// ==================== Сборка графа ====================

std::unique_ptr<domain::Graph> GraphBuilder::build() {
    // Валидация данных перед сборкой
    validateData();

    // Создаём пустой граф
    auto graph = std::make_unique<domain::Graph>();

    // Добавляем все узлы в граф
    for (const auto& parsed_node : nodes_) {
        domain::Node node;
        node.id = parsed_node.id;
        node.coords.x = parsed_node.x;
        node.coords.y = parsed_node.y;

        // Определяем тип узла
        if (parsed_node.type == "intersection") {
            node.type = domain::NodeType::Intersection;
        } else if (parsed_node.type == "bus_stop") {
            node.type = domain::NodeType::BusStop;
        } else if (parsed_node.type == "metro_station") {
            node.type = domain::NodeType::MetroStation;
        } else if (parsed_node.type == "poi") {
            node.type = domain::NodeType::Poi;
        } else {
            // По умолчанию - перекрёсток
            node.type = domain::NodeType::Intersection;
        }

        node.name = parsed_node.name;
        graph->addNode(node);
    }

    // Добавляем все рёбра в граф
    for (const auto& parsed_edge : edges_) {
        domain::Edge edge;
        edge.id = parsed_edge.id;
        edge.from = parsed_edge.from_node;
        edge.to = parsed_edge.to_node;
        edge.name = parsed_edge.name;
        edge.length_meters = parsed_edge.length_meters;

        // Определяем тип транспорта
        if (parsed_edge.transport_type == "walk") {
            edge.transport = domain::TransportType::Walk;
        } else if (parsed_edge.transport_type == "car") {
            edge.transport = domain::TransportType::Car;
        } else if (parsed_edge.transport_type == "bus") {
            edge.transport = domain::TransportType::Bus;
        } else if (parsed_edge.transport_type == "metro") {
            edge.transport = domain::TransportType::Metro;
        } else if (parsed_edge.transport_type == "taxi") {
            edge.transport = domain::TransportType::Taxi;
        } else {
            // По умолчанию - пешком
            edge.transport = domain::TransportType::Walk;
        }

        // Создаём логику для ребра на основе расписания.
        // Пока используем StaticLogic для всех рёбер.
        // ScheduledLogic, FrequencyBasedLogic и TimeWindowLogic
        // будут добавлены на этапе 2 разработки.
        edge.logic = std::make_unique<domain::StaticLogic>();

        graph->addEdge(std::move(edge));
    }

    return graph;
}

} // namespace infrastructure

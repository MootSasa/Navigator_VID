#include "application/navigation_facade.h"
#include "infrastructure/graph_builder.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

// ==================== Вспомогательные функции ====================

// Форматирование длительности в виде ЧЧ:ММ:СС
static std::string formatDuration(double seconds) {
    int total_sec = static_cast<int>(seconds);
    int hours = total_sec / 3600;
    int minutes = (total_sec % 3600) / 60;
    int secs = total_sec % 60;

    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hours, minutes, secs);
    return std::string(buf);
}

// Тип транспорта
static const char* transportName(domain::TransportType t) {
    switch (t) {
        case domain::TransportType::Walk:  return "Пешком";
        case domain::TransportType::Car:   return "Авто";
        case domain::TransportType::Taxi:  return "Такси";
        case domain::TransportType::Bus:   return "Автобус";
        case domain::TransportType::Metro: return "Метро";
    }
    return "???";
}

// ==================== Печать маршрута ====================

static void printRoute(const application::RouteResult& result, const domain::Graph& graph) {
    std::cout << "  Узлы: ";
    for (size_t i = 0; i < result.nodes.size(); ++i) {
        const auto& node = graph.getNode(result.nodes[i]);
        std::cout << node.name << "(" << result.nodes[i] << ")";
        if (i + 1 < result.nodes.size()) std::cout << " -> ";
    }
    std::cout << "\n";

    std::cout << "  Рёбра: ";
    for (size_t i = 0; i < result.edges.size(); ++i) {
        const auto& edges = graph.getEdgesFrom(result.nodes[i]);
        // Ищем ребро с нужным ID
        for (const auto& e : edges) {
            if (e.id == result.edges[i]) {
                std::cout << transportName(e.transport) << " [" << e.name << "]";
                break;
            }
        }
        if (i + 1 < result.edges.size()) std::cout << " -> ";
    }
    std::cout << "\n";

    const auto& cost = result.total_cost;
    std::cout << "  Время: " << formatDuration(cost.time_seconds) << "\n";
    std::cout << "  Стоимость: " << cost.money_rub << " руб.\n";
    std::cout << "  Усталость: " << cost.fatigue << "\n";
    std::cout << "  Пересадки: " << cost.transfers << "\n";
}

// ==================== main ====================

int main(int argc, char* argv[]) {
    // Пути к CSV по умолчанию
    std::string data_dir = "data";
    if (argc > 1) {
        data_dir = argv[1];
    }

    std::string nodes_path    = data_dir + "/nodes.csv";
    std::string edges_path    = data_dir + "/edges.csv";
    std::string params_path   = data_dir + "/transport_params.csv";
    std::string schedules_path = data_dir + "/schedules.csv";

    try {
        // Загружаем данные из CSV
        infrastructure::GraphBuilder builder;
        builder.loadNodes(nodes_path)
               .loadEdges(edges_path)
               .loadTransportParams(params_path)
               .loadSchedules(schedules_path);

        auto graph = builder.build();
        auto params = builder.buildTransportParams();

        // Создаём фасад
        application::NavigationFacade facade(std::move(graph), std::move(params));

        std::cout << "=== Transport Navigator ===\n";
        std::cout << "Загружено узлов: " << facade.getGraph().nodeCount() << "\n\n";

        // Интерактивный режим
        std::cout << "Введите: start_id goal_id [strategy]\n";
        std::cout << "Стратегии: fastest (по умолчанию), cheapest, convenient\n";
        std::cout << "Для выхода введите: q\n\n";

        std::string line;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, line) || line == "q") break;

            std::istringstream iss(line);
            int start_id = 0, goal_id = 0;
            std::string strategy = "fastest";

            if (!(iss >> start_id >> goal_id)) {
                std::cout << "Ошибка: введите start_id goal_id [strategy]\n";
                continue;
            }
            iss >> strategy;

            if (!facade.hasNode(start_id)) {
                std::cout << "Ошибка: узел " << start_id << " не найден\n";
                continue;
            }
            if (!facade.hasNode(goal_id)) {
                std::cout << "Ошибка: узел " << goal_id << " не найден\n";
                continue;
            }

            auto now = std::chrono::system_clock::now();
            auto result = facade.findRoute(start_id, goal_id, strategy, now);

            if (!result) {
                std::cout << "Маршрут не найден.\n";
            } else {
                std::cout << "Маршрут (" << strategy << "):\n";
                printRoute(*result, facade.getGraph());
            }
            std::cout << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

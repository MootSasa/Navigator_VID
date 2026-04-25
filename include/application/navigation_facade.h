#pragma once

#include "a_star_solver.h"
#include "strategies.h"
#include "../domain/graph.h"
#include "../domain/transport_params.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace application {

/**
 * @brief Фасад для навигационной системы.
 *
 * Предоставляет простой интерфейс для поиска маршрутов
 * и получения данных графа для отображения.
 */
class NavigationFacade {
public:
    /**
     * @brief Конструктор с готовым графом и параметрами.
     * @param graph Уникальный указатель на граф
     * @param params Параметры транспорта
     */
    NavigationFacade(std::unique_ptr<domain::Graph> graph, domain::TransportParams params);

    /**
     * @brief Найти маршрут между двумя узлами.
     * @param start ID начального узла
     * @param goal ID конечного узла
     * @param strategy_type Тип стратегии: "fastest", "cheapest", "convenient"
     * @param departure Время отправления
     * @return Результат маршрута или nullopt
     */
    std::optional<RouteResult> findRoute(
        domain::NodeId start,
        domain::NodeId goal,
        const std::string& strategy_type = "fastest",
        std::chrono::system_clock::time_point departure = std::chrono::system_clock::now()
    );

    /**
     * @brief Найти маршруты всеми стратегиями.
     * @param start ID начального узла
     * @param goal ID конечного узла
     * @param departure Время отправления
     * @return Вектор результатов (по одному на стратегию)
     */
    std::vector<RouteResult> findAllRoutes(
        domain::NodeId start,
        domain::NodeId goal,
        std::chrono::system_clock::time_point departure = std::chrono::system_clock::now()
    );

    /**
     * @brief Получить граф для отображения.
     * @return Константная ссылка на граф
     */
    const domain::Graph& getGraph() const;

    /**
     * @brief Получить параметры транспорта.
     * @return Константная ссылка на параметры
     */
    const domain::TransportParams& getParams() const;

    /**
     * @brief Проверить, существует ли узел.
     * @param id ID узла
     * @return true если узел существует
     */
    bool hasNode(domain::NodeId id) const;

private:
    std::unique_ptr<domain::Graph> graph_;
    domain::TransportParams params_;

    FastestStrategy fastest_;
    CheapestStrategy cheapest_;
    MostConvenientStrategy convenient_;

    /**
     * @brief Получить стратегию по имени.
     * @param type Имя стратегии
     * @return Ссылка на стратегию
     */
    IRouteStrategy& getStrategy(const std::string& type);
};

} // namespace application

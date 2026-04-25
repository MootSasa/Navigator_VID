#pragma once

#include "edge.h"
#include "node.h"
#include "types.h"

#include <unordered_map>
#include <vector>
#include <string>

namespace domain {

/**
 * @brief Ориентированный мультимодальный граф.
 *
 * Хранит узлы и рёбра в виде списка смежности.
 * Поддерживает добавление узлов, рёбер и пересадок.
 */
class Graph {
public:
    /**
     * @brief Добавляет узел в граф.
     * @param node Узел для добавления
     */
    void addNode(Node node);

    /**
     * @brief Добавляет ребро в граф.
     * @param edge Ребро для добавления
     */
    void addEdge(Edge edge);

    /**
     * @brief Добавляет пересадку между узлами (пешеходное ребро).
     * @param from ID начального узла
     * @param to ID конечного узла
     * @param time_minutes Время пересадки в минутах
     * @param name Название пересадки (по умолчанию "Пересадка")
     */
    void addTransfer(NodeId from, NodeId to, double time_minutes, const std::string& name = "Пересадка");

    /**
     * @brief Проверяет существование узла.
     * @param id ID узла
     * @return true если узел существует
     */
    bool hasNode(NodeId id) const;

    /**
     * @brief Возвращает узел по ID.
     * @param id ID узла
     * @return Константная ссылка на узел
     */
    const Node& getNode(NodeId id) const;

    /**
     * @brief Возвращает все рёбра исходящие из узла.
     * @param id ID узла
     * @return Константная ссылка на вектор рёбер
     */
    const std::vector<Edge>& getEdgesFrom(NodeId id) const;

    /**
     * @brief Возвращает все узлы графа (для UI/Facade).
     * @return Константная ссылка на карту узлов
     */
    const std::unordered_map<NodeId, Node>& getAllNodes() const;

    /**
     * @brief Возвращает все рёбра графа (для UI/Facade).
     * @return Константная ссылка на карту списков смежности
     */
    const std::unordered_map<NodeId, std::vector<Edge>>& getAllEdges() const;

    /**
     * @brief Возвращает количество узлов в графе.
     * @return Количество узлов
     */
    std::size_t nodeCount() const;

private:
    std::unordered_map<NodeId, Node> nodes_;
    std::unordered_map<NodeId, std::vector<Edge>> adj_;
};

} // namespace domain

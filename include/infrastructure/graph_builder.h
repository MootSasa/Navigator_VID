#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "domain/transport_params.h"
#include "infrastructure/csv_parser.h"

// Forward declarations для доменных типов
namespace domain {
    class Graph;
    struct Node;
    struct Edge;
} // namespace domain

namespace infrastructure {

/**
 * @brief Класс для построения графа из CSV файлов.
 *
 * Использует паттерн Builder для пошаговой загрузки данных
 * и последующей сборки графа.
 */
class GraphBuilder {
public:
    GraphBuilder() = default;

    /**
     * @brief Загружает узлы из CSV файла.
     * @param filepath Путь к файлу nodes.csv
     * @return Ссылка на *this для цепочечного вызова
     * @throws std::runtime_error если файл не найден
     */
    GraphBuilder& loadNodes(const std::string& filepath);

    /**
     * @brief Загружает рёбра из CSV файла.
     * @param filepath Путь к файлу edges.csv
     * @return Ссылка на *this для цепочечного вызова
     * @throws std::runtime_error если файл не найден
     */
    GraphBuilder& loadEdges(const std::string& filepath);

    /**
     * @brief Загружает параметры транспорта из CSV файла.
     * @param filepath Путь к файлу transport_params.csv
     * @return Ссылка на *this для цепочечного вызова
     * @throws std::runtime_error если файл не найден
     */
    GraphBuilder& loadTransportParams(const std::string& filepath);

    /**
     * @brief Загружает расписания из CSV файла.
     * @param filepath Путь к файлу schedules.csv
     * @return Ссылка на *this для цепочечного вызова
     * @throws std::runtime_error если файл не найден
     */
    GraphBuilder& loadSchedules(const std::string& filepath);

    /**
     * @brief Собирает и возвращает готовый граф.
     * @return Уникальный указатель на построенный граф
     * @throws std::runtime_error если данные некорректны
     */
    std::unique_ptr<domain::Graph> build();

    /**
     * @brief Собирает объект TransportParams из загруженных параметров.
     * @return Объект TransportParams с параметрами из CSV
     */
    domain::TransportParams buildTransportParams() const;

    /**
     * @brief Возвращает список загруженных узлов.
     * @return Константная ссылка на вектор узлов
     */
    const std::vector<ParsedNode>& getNodes() const { return nodes_; }

    /**
     * @brief Возвращает список загруженных рёбер.
     * @return Константная ссылка на вектор рёбер
     */
    const std::vector<ParsedEdge>& getEdges() const { return edges_; }

    /**
     * @brief Возвращает параметры транспорта.
     * @return Константная ссылка на карту параметров
     */
    const std::map<std::string, std::map<std::string, std::string>>& getTransportParams() const {
        return transport_params_;
    }

    /**
     * @brief Возвращает расписания.
     * @return Константная ссылка на карту расписаний по edge_id
     */
    const std::map<int, ParsedSchedule>& getSchedules() const { return schedules_; }

private:
    // Распарсенные данные
    std::vector<ParsedNode> nodes_;
    std::vector<ParsedEdge> edges_;
    std::map<std::string, std::map<std::string, std::string>> transport_params_;
    std::map<int, ParsedSchedule> schedules_;  // По edge_id

    // Множество ID для валидации
    std::set<int> node_ids_;
    std::set<int> edge_ids_;

    CsvParser parser_;

    /**
     * @brief Проверяет корректность загруженных данных.
     * @throws std::runtime_error если данные некорректны
     */
    void validateData() const;

    /**
     * @brief Проверяет, что все узлы, на которые ссылаются рёбра, существуют.
     * @throws std::runtime_error если найдена ссылка на несуществующий узел
     */
    void validateEdgeReferences() const;

    /**
     * @brief Проверяет, что расписания ссылаются на существующие рёбра.
     * @throws std::runtime_error если найдена ссылка на несуществующее ребро
     */
    void validateScheduleReferences() const;

    /**
     * @brief Вспомогательный метод для извлечения числового параметра из карты.
     * @param type Тип транспорта
     * @param name Имя параметра
     * @param default_value Значение по умолчанию
     * @return Значение параметра или default_value
     */
    double getParam(const std::string& type, const std::string& name, double default_value) const;
};

} // namespace infrastructure

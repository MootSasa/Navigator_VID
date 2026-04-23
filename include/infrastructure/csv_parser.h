#pragma once

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace infrastructure {

/**
 * @brief Структура, представляющая один узел графа после парсинга.
 */
struct ParsedNode {
  int id;
  std::string type;  ///< "intersection", "bus_stop", "metro_station", "poi"
  std::string name;
  double x;
  double y;
};

/**
 * @brief Структура, представляющая одно ребро графа после парсинга.
 */
struct ParsedEdge {
  int id;
  int from_node;
  int to_node;
  std::string transport_type;  ///< "walk", "car", "bus", "metro", "taxi"
  double length_meters;
  std::string name;
};

/**
 * @brief Структура, представляющая параметр транспорта.
 */
struct ParsedTransportParam {
  std::string transport_type;
  std::string param_name;
  std::string param_value;
};

/**
 * @brief Структура, представляющая расписание для ребра.
 */
struct ParsedSchedule {
  int edge_id;
  std::string schedule_type;  ///< "fixed_times", "time_window"
  std::string params;
};

/**
 * @brief Класс для парсинга CSV файлов транспортного навигатора.
 *
 * Предоставляет методы для парсинга каждого типа CSV файла:
 * - nodes.csv
 * - edges.csv
 * - transport_params.csv
 * - schedules.csv
 */
class CsvParser {
 public:
  /**
   * @brief Базовый парсер CSV в таблицу строк.
   * @param input Входной поток для парсинга
   * @return Таблица (вектор строк), где каждая строка - вектор ячеек
   */
  std::vector<std::vector<std::string>> parse(std::istream& input) const;

  /**
   * @brief Парсит файл nodes.csv.
   * Формат: node_id,type,name,x,y
   * @param input Входной поток
   * @return Вектор распарсенных узлов
   */
  std::vector<ParsedNode> parseNodes(std::istream& input) const;

  /**
   * @brief Парсит файл edges.csv.
   * Формат: edge_id,from_node,to_node,transport_type,length_meters,name
   * @param input Входной поток
   * @return Вектор распарсенных рёбер
   */
  std::vector<ParsedEdge> parseEdges(std::istream& input) const;

  /**
   * @brief Парсит файл transport_params.csv.
   * Формат: transport_type,param_name,param_value
   * @param input Входной поток
   * @return Вектор распарсенных параметров
   */
  std::vector<ParsedTransportParam> parseTransportParams(
      std::istream& input) const;

  /**
   * @brief Парсит файл schedules.csv.
   * Формат: edge_id,schedule_type,params
   * @param input Входной поток
   * @return Вектор распарсенных расписаний
   */
  std::vector<ParsedSchedule> parseSchedules(std::istream& input) const;

 private:
  /**
   * @brief Удаляет пробелы и кавычки из строки.
   * @param str Исходная строка
   * @return Очищенная строка
   */
  std::string trim(const std::string& str) const;

  /**
   * @brief Разбирает строку на ячейки по разделителю.
   * @param line Строка для разбора
   * @param delimiter Разделитель (по умолчанию ',')
   * @return Вектор ячеек
   */
  std::vector<std::string> splitLine(const std::string& line,
                                      char delimiter = ',') const;

  /**
   * @brief Парсит число типа double из строки.
   * @param str Строка с числом
   * @return Распарсенное число или 0.0 при ошибке
   */
  double parseDouble(const std::string& str) const;

  /**
   * @brief Парсит целое число из строки.
   * @param str Строка с числом
   * @return Распарсенное число или 0 при ошибке
   */
  int parseInt(const std::string& str) const;
};

} // namespace infrastructure

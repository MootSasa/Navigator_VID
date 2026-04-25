#pragma once

#include "combined_cost.h"
#include "transport_params.h"

#include <chrono>
#include <optional>

namespace application {
class IRouteStrategy;
}  // namespace application

namespace domain {

struct Edge;

/**
 * @brief Контекст запроса для расчёта ребра.
 */
struct QueryContext {
  std::chrono::system_clock::time_point departure_time;  ///< Время отправления от начального узла ребра
  const TransportParams& params;                          ///< Параметры транспорта
};

/**
 * @brief Результат расчёта ребра.
 */
struct EdgeResult {
  std::chrono::system_clock::time_point arrival_time;  ///< Время прибытия в конечный узел
  CombinedCost cost;                                   ///< Стоимость прохождения ребра
};

/**
 * @brief Интерфейс для рёбер с поведением, зависящим от времени.
 *
 * Реализации: StaticLogic, TimeWindowLogic, FrequencyBasedLogic, ScheduledLogic.
 */
class ITimeVariant {
 public:
  virtual ~ITimeVariant() = default;

  /**
   * @brief Рассчитывает время прибытия и стоимость прохождения ребра.
   * Если ребро недоступно в данный момент, возвращает std::nullopt.
   * @param edge Ребро графа
   * @param ctx Контекст запроса (время, параметры)
   * @param strategy Стратегия поиска маршрута
   * @return Результат или nullopt если ребро недоступно
   */
  virtual std::optional<EdgeResult> calculate(
      const Edge& edge, const QueryContext& ctx,
      const application::IRouteStrategy& strategy) const = 0;
};

}  // namespace domain

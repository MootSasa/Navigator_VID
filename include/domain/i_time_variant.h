#pragma once

#include <chrono>
#include <optional>

#include "domain/combined_cost.h"
#include "domain/transport_params.h"

namespace application {
class IRouteStrategy;
}  // namespace application

namespace domain {

struct Edge;

struct QueryContext {
  std::chrono::system_clock::time_point departure_time;
  const TransportParams& params;
};

struct EdgeResult {
  std::chrono::system_clock::time_point arrival_time;
  CombinedCost cost;
};

class ITimeVariant {
 public:
  virtual ~ITimeVariant() = default;

  virtual std::optional<EdgeResult> calculate(
      const Edge& edge, const QueryContext& ctx,
      const application::IRouteStrategy& strategy) const = 0;
};

}  // namespace domain

#pragma once

#include "domain/i_time_variant.h"

namespace domain {

class StaticLogic : public ITimeVariant {
public:
    std::optional<EdgeResult> calculate(
        const Edge& edge,
        const QueryContext& ctx,
        const application::IRouteStrategy& strategy
    ) const override;
};

} // namespace domain

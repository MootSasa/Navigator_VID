#pragma once

#include "domain/i_time_variant.h"

class StaticLogic : public ITimeVariant {
public:
    std::optional<EdgeResult> calculate(
        const Edge& edge,
        const QueryContext& ctx,
        const IRouteStrategy& strategy
    ) const override;
};

#pragma once

#include "domain/i_time_variant.h"
#include "domain/types.h"

#include <memory>
#include <string>

struct Edge {
    EdgeId id = 0;
    NodeId from = 0;
    NodeId to = 0;
    TransportType transport = TransportType::Walk;
    double length_meters = 0.0;
    std::string name;
    std::unique_ptr<ITimeVariant> logic;
};

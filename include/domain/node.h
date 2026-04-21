#pragma once

#include "domain/types.h"

#include <string>

struct Point {
    double x = 0.0;
    double y = 0.0;
};

struct Node {
    NodeId id = 0;
    NodeType type = NodeType::Intersection;
    std::string name;
    Point coords;
};

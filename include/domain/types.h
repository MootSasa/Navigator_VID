#pragma once

using NodeId = int;
using EdgeId = int;

enum class TransportType {
    Walk,
    Car,
    Taxi,
    Bus,
    Metro
};

enum class NodeType {
    Intersection,
    BusStop,
    MetroStation,
    Poi
};

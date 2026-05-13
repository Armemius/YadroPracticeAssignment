#pragma once

#include "simulation/types.hpp"

namespace sim {

enum class ProductionEventType { START, FINISH, READY };

struct ProductionEvent {
    ProductionEventType type;
    simtime_t time;
    product_index_t index;
    operation_t operation;
    machine_t machine;
};

struct EnqueueEvent {
    ProductionEventType type;
    simtime_t time;
    product_index_t index;
    operation_t operation;
    machine_t machine;
    product_index_t queue_size;
};

struct StopEvent {
    simtime_t time;
};

}  // namespace sim

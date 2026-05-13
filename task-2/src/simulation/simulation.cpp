#include "simulation/simulation.hpp"
#include <algorithm>
#include <limits>
#include <utility>
#include <vector>
#include "simulation/machine.hpp"
#include "simulation/types.hpp"

namespace sim {

namespace {

struct ReadyProduct {
    product_index_t product_index;
    machine_t machine_index;
};

struct WaitEvent {
    simtime_t tick;
    product_index_t product_index;
    product_type_t product_type;
    machine_t machine_index;
    size_t queue_size;
};

struct RoutedProduct {
    product_t product;
    Machine *machine;
};

}  // namespace

Simulation::Simulation(std::vector<Machine> machines, product_type_t product_type_count, std::ostream &log)
    : machines_(std::move(machines)), product_type_count_(product_type_count), log_(&log) {}

void Simulation::run() {
    while (!finished()) {
        next();
    }
    log_stop(tick_);
}

void Simulation::next() {
    for (auto &machine : machines_) {
        if (machine.last_tick() < tick_) {
            machine.tick(tick_);
        }
    }

    std::vector<ReadyProduct> ready_products;
    do {
        std::vector<RoutedProduct> routed_products;
        std::vector<WaitEvent> wait_events;

        for (auto &machine : machines_) {
            if (machine.ready()) {
                product_t product = machine.yield();
                log_finish(tick_, product.index, product.type - 1, machine.index());
                if (product.type + 1 == product_type_count_) {
                    ready_products.push_back({.product_index = product.index, .machine_index = machine.index()});
                } else {
                    routed_products.push_back({.product = product, .machine = nullptr});
                }
            }
        }

        for (auto &product : routed_products) {
            product.machine = &*std::ranges::min_element(machines_, [](const Machine &lhs, const Machine &rhs) {
                return std::pair<simtime_t, machine_t>{lhs.wait_time(), lhs.index()} <
                       std::pair<simtime_t, machine_t>{rhs.wait_time(), rhs.index()};
            });
        }

        for (auto &machine : machines_) {
            if (machine.can_process()) {
                product_t product = machine.next_item();
                log_start(tick_, product.index, product.type, machine.index());
                machine.start();
            }
        }

        for (auto &routed_product : routed_products) {
            Machine &machine = *routed_product.machine;
            if (machine.idle() && !machine.has_next() && !machine.ready()) {
                log_start(tick_, routed_product.product.index, routed_product.product.type, machine.index());
                machine.start(routed_product.product);
            } else {
                wait_events.push_back({.tick = tick_,
                                       .product_index = routed_product.product.index,
                                       .product_type = routed_product.product.type,
                                       .machine_index = machine.index(),
                                       .queue_size = machine.queue_size()});
                machine.enqueue(routed_product.product);
            }
        }

        for (const auto &event : wait_events) {
            log_wait(event.tick, event.product_index, event.product_type, event.machine_index, event.queue_size);
        }

    } while (std::ranges::any_of(machines_, [](const Machine &machine) { return machine.ready(); }));

    for (const auto &product : ready_products) {
        log_ready(tick_, product.product_index, product.machine_index);
    }

    if (!finished()) {
        advance_to_next_event();
    }
}

bool Simulation::finished() const {
    return std::ranges::all_of(
        machines_, [](const Machine &machine) { return machine.idle() && !machine.has_next() && !machine.ready(); });
}

void Simulation::advance_to_next_event() {
    simtime_t next_tick = std::numeric_limits<simtime_t>::max();
    for (const auto &machine : machines_) {
        if (machine.processing()) {
            next_tick = std::min(next_tick, machine.current_processing_time());
        }
    }
    if (next_tick != std::numeric_limits<simtime_t>::max()) {
        tick_ = next_tick;
    }
}

void Simulation::log_start(simtime_t tick, product_index_t product_index, product_type_t product_type,
                           machine_t machine_index) const {
    *log_ << "start " << tick << " " << product_index << " " << static_cast<int>(product_type) << " "
          << static_cast<int>(machine_index) << "\n";
}

void Simulation::log_finish(simtime_t tick, product_index_t product_index, product_type_t product_type,
                            machine_t machine_index) const {
    *log_ << "finish " << tick << " " << product_index << " " << static_cast<int>(product_type) << " "
          << static_cast<int>(machine_index) << "\n";
}

void Simulation::log_ready(simtime_t tick, product_index_t product_index, machine_t machine_index) const {
    *log_ << "ready " << tick << " " << product_index << " " << static_cast<int>(machine_index) << "\n";
}

void Simulation::log_wait(simtime_t tick, product_index_t product_index, product_type_t product_type,
                          machine_t machine_index, size_t queue_size) const {
    *log_ << "wait " << tick << " " << product_index << " " << static_cast<int>(product_type) << " "
          << static_cast<int>(machine_index) << " " << queue_size << "\n";
}

void Simulation::log_stop(simtime_t tick) const {
    *log_ << "stop " << tick << "\n";
}

}  // namespace sim

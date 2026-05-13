#include "simulation/simulation.hpp"

#include "simulation/machine.hpp"
#include "simulation/types.hpp"

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

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
    : machines_(std::move(machines)),
      product_type_count_(product_type_count),
      log_(&log),
      machine_versions_(machines_.size()) {
    for (const auto &machine : machines_) {
        wait_index_.emplace(machine.wait_time(), machine.index());
    }
}

void Simulation::run() {
    while (!finished()) {
        next();
    }
    log_stop(tick_);
}

void Simulation::next() {
    tick_finished_machines();

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
            product.machine = &select_machine();
        }

        for (auto &machine : machines_) {
            if (machine.can_process()) {
                start_from_queue(machine);
            }
        }

        for (auto &routed_product : routed_products) {
            Machine &machine = *routed_product.machine;
            if (machine.idle() && !machine.has_next() && !machine.ready()) {
                start_direct(machine, routed_product.product);
            } else {
                wait_events.push_back({.tick = tick_,
                                       .product_index = routed_product.product.index,
                                       .product_type = routed_product.product.type,
                                       .machine_index = machine.index(),
                                       .queue_size = machine.queue_size()});
                enqueue_to_machine(machine, routed_product.product);
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

void Simulation::refresh_wait_index(machine_t machine_index, simtime_t previous_wait_time) {
    wait_index_.erase({previous_wait_time, machine_index});
    wait_index_.emplace(machines_[machine_index].wait_time(), machine_index);
}

void Simulation::schedule_finish(const Machine &machine) {
    if (!machine.processing()) {
        return;
    }
    const machine_t machine_index = machine.index();
    ++machine_versions_[machine_index];
    finish_events_.push({.tick = machine.current_processing_time(),
                         .machine_index = machine_index,
                         .version = machine_versions_[machine_index]});
}

void Simulation::sync_machine_time(Machine &machine) const {
    if (machine.last_tick() < tick_) {
        machine.tick(tick_);
    }
}

void Simulation::tick_finished_machines() {
    while (!finish_events_.empty()) {
        const FinishEvent event = finish_events_.top();
        if (event.version != machine_versions_[event.machine_index]) {
            finish_events_.pop();
            continue;
        }
        if (event.tick != tick_) {
            break;
        }

        finish_events_.pop();
        Machine &machine = machines_[event.machine_index];
        if (machine.last_tick() < tick_) {
            machine.tick(tick_);
        }
    }
}

void Simulation::start_from_queue(Machine &machine) {
    sync_machine_time(machine);
    product_t product = machine.next_item();
    const simtime_t previous_wait_time = machine.wait_time();
    log_start(tick_, product.index, product.type, machine.index());
    machine.start();
    refresh_wait_index(machine.index(), previous_wait_time);
    schedule_finish(machine);
}

void Simulation::start_direct(Machine &machine, product_t product) {
    sync_machine_time(machine);
    log_start(tick_, product.index, product.type, machine.index());
    machine.start(product);
    schedule_finish(machine);
}

void Simulation::enqueue_to_machine(Machine &machine, product_t product) {
    sync_machine_time(machine);
    const simtime_t previous_wait_time = machine.wait_time();
    machine.enqueue(product);
    refresh_wait_index(machine.index(), previous_wait_time);
}

Machine &Simulation::select_machine() {
    return machines_[wait_index_.begin()->second];
}

void Simulation::advance_to_next_event() {
    while (!finish_events_.empty()) {
        const FinishEvent event = finish_events_.top();
        if (event.version != machine_versions_[event.machine_index]) {
            finish_events_.pop();
            continue;
        }
        tick_ = event.tick;
        return;
    }

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

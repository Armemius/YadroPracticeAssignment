#pragma once

#include "simulation/machine.hpp"
#include "simulation/types.hpp"

#include <functional>
#include <ostream>
#include <queue>
#include <set>
#include <utility>
#include <vector>

namespace sim {

/// Class encapsulating simulation logic
class Simulation final {
   public:
    /**
     * @brief Construct a new Simulation object
     *
     * @param machines List of machines for simulation
     */
    Simulation(std::vector<Machine> machines, product_type_t product_type_count, std::ostream &log);

    /**
     * @brief Starts the simulation
     *
     */
    void run();

    /**
     * @brief Process next step or several steps for the simulation
     *
     */
    void next();

    /**
     * @brief Checks if simulation is finished or still running
     *
     * @return true Simulation is finished
     * @return false Simulation is not finished
     */
    bool finished() const;

   private:
    struct FinishEvent {
        simtime_t tick;
        machine_t machine_index;
        uint64_t version;

        friend bool operator>(const FinishEvent &lhs, const FinishEvent &rhs) noexcept {
            return std::pair<simtime_t, machine_t>{lhs.tick, lhs.machine_index} >
                   std::pair<simtime_t, machine_t>{rhs.tick, rhs.machine_index};
        }
    };

    /**
     * @brief Refreshes machine position in the wait-time index.
     *
     * @param machine_index Machine index.
     * @param previous_wait_time Machine wait time before mutation.
     */
    void refresh_wait_index(machine_t machine_index, simtime_t previous_wait_time);

    /**
     * @brief Registers current machine processing completion in the finish queue.
     *
     * @param machine Machine to schedule.
     */
    void schedule_finish(const Machine &machine);

    /**
     * @brief Advances one machine to the current simulation tick if needed.
     *
     * @param machine Machine to synchronize.
     */
    void sync_machine_time(Machine &machine) const;

    /**
     * @brief Marks all machines that finish at current tick as ready.
     */
    void tick_finished_machines();

    /**
     * @brief Starts processing from a machine queue and updates indexes.
     *
     * @param machine Machine to start.
     */
    void start_from_queue(Machine &machine);

    /**
     * @brief Starts processing of a directly routed product and updates indexes.
     *
     * @param machine Machine to start.
     * @param product Product to process.
     */
    void start_direct(Machine &machine, product_t product);

    /**
     * @brief Queues a routed product and updates indexes.
     *
     * @param machine Machine receiving the product.
     * @param product Product to enqueue.
     */
    void enqueue_to_machine(Machine &machine, product_t product);

    /**
     * @brief Returns machine with minimal queued wait time.
     *
     * @return Machine& Selected machine.
     */
    Machine &select_machine();

    /**
     * @brief Advance current simulation time to the next processing completion
     */
    void advance_to_next_event();

    /**
     * @brief Logs start of product processing
     *
     * @param tick Simulation time
     * @param product_index Product index
     * @param product_type Product type
     * @param machine_index Machine index
     */
    void log_start(simtime_t tick, product_index_t product_index, product_type_t product_type,
                   machine_t machine_index) const;

    /**
     * @brief Logs end of product processing
     *
     * @param tick Simulation time
     * @param product_index Product index
     * @param product_type Product type
     * @param machine_index Machine index
     */
    void log_finish(simtime_t tick, product_index_t product_index, product_type_t product_type,
                    machine_t machine_index) const;

    /**
     * @brief Logs ready product
     *
     * @param tick Simulation time
     * @param product_index Product index
     * @param machine_index Machine index
     */
    void log_ready(simtime_t tick, product_index_t product_index, machine_t machine_index) const;

    /**
     * @brief Logs product enqueue
     *
     * @param tick Simulation time
     * @param product_index Product index
     * @param product_type Product type
     * @param machine_index Machine index
     * @param queue_size Size of the queue of the machine
     */
    void log_wait(simtime_t tick, product_index_t product_index, product_type_t product_type, machine_t machine_index,
                  size_t queue_size) const;

    /**
     * @brief Logs end of simulation
     *
     * @param tick Simulation time
     */
    void log_stop(simtime_t tick) const;

    std::ostream *log_{};                                   ///< Log output stream for the simulation
    simtime_t tick_{};                                      ///< Current simulation's tick
    product_type_t product_type_count_{};                   ///< Total number of product types
    std::vector<Machine> machines_;                         ///< Machines for the simulation
    std::set<std::pair<simtime_t, machine_t>> wait_index_;  ///< Machines ordered by queued wait time
    std::priority_queue<FinishEvent, std::vector<FinishEvent>, std::greater<>> finish_events_;  ///< Finish events
    std::vector<uint64_t> machine_versions_;  ///< Per-machine versions for stale finish events
};

}  // namespace sim

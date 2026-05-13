#pragma once

#include <ostream>
#include <vector>
#include "simulation/machine.hpp"
#include "simulation/types.hpp"

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

    std::ostream *log_{};                  ///< Log output stream for the simulation
    simtime_t tick_{};                     ///< Current simulation's tick
    product_type_t product_type_count_{};  ///< Total number of product types
    std::vector<Machine> machines_;        ///< Machines for the simulation
};

}  // namespace sim

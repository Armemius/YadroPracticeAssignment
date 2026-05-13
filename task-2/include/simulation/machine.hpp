#pragma once

#include <optional>
#include <queue>
#include <unordered_map>
#include "simulation/types.hpp"

namespace sim {

/// Represents individual machine
class Machine final {
   public:
    /**
     * @brief Construct a new Machine object
     * 
     * @param clock Simulation clock
     * @param products Order of products to process
     * @param optimes Times of operations for the products
     */
    Machine(machine_t index, const std::vector<product_t> &products,
            std::unordered_map<product_type_t, optime_t> optimes);

    /**
     * @brief Returns index of the machine
     * 
     * @return machine_t Index of the machine
     */
    [[nodiscard]] machine_t index() const noexcept;

    /**
     * @brief Starts processing of item in queue
     * 
     * @param now Current simulation time
     * @exception std::logic_error Some item is already is processed
     * @exception std::out_of_range No items are present in the queue
     * @return true Item was instantly processed
     * @return false Item processing is in progress
     */
    bool start(simtime_t now);

    /**
     * @brief Processes tick of the simulation
     * 
     * @param now Current simulation time
     * @return true Resource is ready
     * @return false Resource is not ready or no operations are present
     */
    bool tick(simtime_t now);

    /**
     * @brief Checks if item is ready to yield
     * 
     * @return true Item is ready
     * @return false Item is not ready or no operations are present
     */
    [[nodiscard]] bool ready() const noexcept;

    /**
     * @brief Checks if some item is processed right now
     * 
     * @return true Item is processing
     * @return false No current operations
     */
    [[nodiscard]] bool processing() const noexcept;

    /**
     * @brief Yields ready product
     * 
     * @exception std::out_of_range No items are available for yielding
     * @return product_t ready product if exists 
     */
    product_t yield();

    /**
     * @brief Adds product to the processing queue
     * 
     * @param product Product to process
     * @return simtime_t Estimated time when the machine would process all the items
     */
    simtime_t enqueue(product_t product);

    /**
     * @brief Gets time when the machine would process all the items
     * 
     * @return simtime_t Simulation time when machine would process all the items
     */
    [[nodiscard]] simtime_t processing_till() const noexcept;

   private:
    machine_t index_;                                       ///< Index of the machine
    simtime_t current_processing_till_{};                   ///< Time point till current processing is performed
    simtime_t processing_till_{};                           ///< Time point till machine has processes to perform
    std::optional<product_t> current_{std::nullopt};        ///< Current product in process
    std::optional<product_t> result_{std::nullopt};         ///< Result of the processing
    std::queue<product_t> queue_;                           ///< Queue of items to process
    std::unordered_map<product_type_t, optime_t> optimes_;  ///< Execution times for the certain product type
};

}  // namespace sim

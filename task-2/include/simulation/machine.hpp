#pragma once

#include "simulation/types.hpp"

#include <optional>
#include <queue>
#include <unordered_map>

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
     * @exception std::logic_error Some item is already is processed
     * @exception std::logic_error Output slot is full
     * @exception std::out_of_range No items are present in the queue
     * @return true Item was instantly processed
     * @return false Item processing is in progress
     */
    bool start();

    /**
     * @brief Starts processing of given item
     * 
     * @exception std::logic_error Some item is already is processed
     * @exception std::logic_error Output slot is full
     * @exception std::out_of_range Some items is present in the queue
     * @return true Item was instantly processed
     * @return false Item processing is in progress
     */
    bool start(product_t product);

    /**
     * @brief Returns item that is currently processed
     * 
     * @exception std::out_of_range No item is currently processed
     * @return product_t Item that is currently processed
     */
    [[nodiscard]] product_t current_processing() const;

    /**
     * @brief Returns next item to process
     *
     * @exception std::out_of_range Queue is empty
     * @return product_t Item that will be processed next
     */
    [[nodiscard]] product_t next_item() const;

    /**
     * @brief Advances time of the machine to certain point and updates state
     * 
     * @exception std::logic_error If tick is less or equal than last tick
     * @param now Current simulation time
     * @return true Resource is ready
     * @return false Resource is not ready or no operations are present
     */
    bool tick(simtime_t now);

    /**
     * @brief Returns last tick of the machine
     * 
     * @return simtime_t Last tick of the machine
     */
    [[nodiscard]] simtime_t last_tick() const noexcept;

    /**
     * @brief Checks if item is ready to yield
     * 
     * @return true Item is ready
     * @return false Item is not ready or no operations are present
     */
    [[nodiscard]] bool ready() const noexcept;

    /**
     * @brief Checks if machine is idle
     * 
     * @return true Machine does no work
     * @return false Machine is processing some item
     */
    [[nodiscard]] bool idle() const noexcept;

    /**
     * @brief Checks if some item is processed right now
     * 
     * @return true Item is processing
     * @return false No current operations
     */
    [[nodiscard]] bool processing() const noexcept;

    /**
     * @brief Checks if machine has next item to process
     * 
     * @return true There is item in queue
     * @return false Queue is empty
     */
    [[nodiscard]] bool has_next() const noexcept;

    /**
     * @brief Checks if machine can start new processing
     * 
     * @return true Machine can start new operation
     * @return false Machine is busy or no items are present in the queue
     */
    [[nodiscard]] bool can_process() const noexcept;

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
     * @brief Returns time when current item would be processed
     * 
     * @return simtime_t Time of item to process
     */
    [[nodiscard]] simtime_t current_processing_time() const noexcept;

    /**
     * @brief Gets time when the machine would process all the items
     * 
     * @return simtime_t Simulation time when machine would process all the items
     */
    [[nodiscard]] simtime_t queue_time() const noexcept;

    /**
     * @brief Gets predicted processing time for items currently waiting in the queue
     *
     * @return simtime_t Total processing time of queued items
     */
    [[nodiscard]] simtime_t wait_time() const noexcept;

    /**
     * @brief Returns total queue size for current machine
     * 
     * @return size_t Amount of items in queue
     */
    [[nodiscard]] size_t queue_size() const noexcept;

   private:
    machine_t index_;                                       ///< Index of the machine
    simtime_t current_processing_till_{};                   ///< Time point till current processing is performed
    simtime_t queue_time_{};                                ///< Time point till machine has processes to perform
    simtime_t wait_time_{};                                 ///< Total processing time of queued products
    simtime_t last_tick_{};                                 ///< Last tick time
    std::optional<product_t> current_{std::nullopt};        ///< Current product in process
    std::optional<product_t> result_{std::nullopt};         ///< Result of the processing
    std::queue<product_t> queue_;                           ///< Queue of items to process
    std::unordered_map<product_type_t, optime_t> optimes_;  ///< Execution times for the certain product type
};

}  // namespace sim
